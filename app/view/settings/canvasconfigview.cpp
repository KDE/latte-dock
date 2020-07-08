/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "canvasconfigview.h"

// local
#include "primaryconfigview.h"
#include "../panelshadows_p.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QQuickItem>
#include <QScreen>

// KDE
#include <KWindowSystem>
#include <KWayland/Client/plasmashell.h>

// Plasma
#include <Plasma/Package>

namespace Latte {
namespace ViewPart {

CanvasConfigView::CanvasConfigView(Latte::View *view, PrimaryConfigView *parent)
    : SubConfigView(view, QString("#canvasconfigview#")),
      m_parent(parent)
{
    setResizeMode(QQuickView::SizeRootObjectToView);

    connections << connect(m_parent, &PrimaryConfigView::availableScreenGeometryChanged, this, &CanvasConfigView::syncGeometry);

    setParentView(view);
    init();
}

void CanvasConfigView::init()
{
    SubConfigView::init();

    QByteArray tempFilePath = "canvasconfigurationui";

    updateEnabledBorders();

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
    syncSlideEffect();

    if (m_parent && KWindowSystem::isPlatformX11()) {
        m_parent->requestActivate();
    }
}

QRect CanvasConfigView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
}

void CanvasConfigView::initParentView(Latte::View *view)
{
    SubConfigView::initParentView(view);

    updateEnabledBorders();
    syncGeometry();

    show();
}

void CanvasConfigView::syncGeometry()
{
    if (!m_latteView || !m_latteView->layout() || !m_latteView->containment() || !m_parent || !rootObject()) {
        return;
    }

    const auto location = m_latteView->containment()->location();
    const auto scrGeometry = m_latteView->screenGeometry();
    const auto availGeometry = m_parent->availableScreenGeometry();
    QSize size;

    int editThickness = m_latteView->editThickness();

    QPoint position{0, 0};

    int xPos{0};
    int yPos{0};

    switch (m_latteView->containment()->formFactor()) {
    case Plasma::Types::Horizontal: {
        xPos = availGeometry.x();
        size.setWidth(availGeometry.width());
        size.setHeight(editThickness);

        if (location == Plasma::Types::TopEdge) {
            yPos = scrGeometry.y();
        } else if (location == Plasma::Types::BottomEdge) {
            yPos = scrGeometry.y() + scrGeometry.height() - editThickness;
        }
    }
        break;

    case Plasma::Types::Vertical: {
        yPos = availGeometry.y();
        size.setWidth(editThickness);
        size.setHeight(availGeometry.height());

        if (location == Plasma::Types::LeftEdge) {
            xPos = scrGeometry.x();
        } else if (location == Plasma::Types::RightEdge) {
            xPos = scrGeometry.x() + scrGeometry.width() - editThickness;
        }
    }
        break;

    default:
        qWarning() << "no sync geometry, wrong formFactor";
        break;
    }

    position = {xPos, yPos};

    updateEnabledBorders();

    auto geometry = QRect(position.x(), position.y(), size.width(), size.height());

    if (m_geometryWhenVisible == geometry) {
        return;
    }

    m_geometryWhenVisible = geometry;

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }

    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    //! after placement request to activate the main config window in order to avoid
    //! rare cases of closing settings window from secondaryConfigView->focusOutEvent
    if (m_parent && KWindowSystem::isPlatformX11()) {
        m_parent->requestActivate();
    }
}

void CanvasConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    if (!m_latteView) {
        return;
    }

    setFlags(wFlags());
    m_corona->wm()->setViewExtraFlags(this, false, Latte::Types::NormalWindow);

    syncGeometry();
    syncSlideEffect();

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &CanvasConfigView::syncGeometry);

    emit showSignal();
}

void CanvasConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    const auto *focusWindow = qGuiApp->focusWindow();

    if ((focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                         || focusWindow->flags().testFlag(Qt::ToolTip)))
            || m_latteView->alternativesIsShown()) {
        return;
    }

    const auto parent = qobject_cast<PrimaryConfigView *>(m_parent);

    if (!m_latteView->containsMouse() && parent && !parent->sticker() && !parent->isActive()) {
        parent->hideConfigWindow();
    }
}

void CanvasConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }
}

//!BEGIN borders
void CanvasConfigView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::TopBorder;

    switch (m_latteView->location()) {
    case Plasma::Types::TopEdge:
        borders = Plasma::FrameSvg::BottomBorder;
        break;

    case Plasma::Types::LeftEdge:
        borders = Plasma::FrameSvg::RightBorder;
        break;

    case Plasma::Types::RightEdge:
        borders = Plasma::FrameSvg::LeftBorder;
        break;

    case Plasma::Types::BottomEdge:
        borders = Plasma::FrameSvg::TopBorder;
        break;

    default:
        break;
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;
        m_corona->dialogShadows()->addWindow(this, m_enabledBorders);

        emit enabledBordersChanged();
    }
}

//!END borders

}
}

