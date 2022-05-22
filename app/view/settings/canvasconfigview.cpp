/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    : SubConfigView(view, QString("#canvasconfigview#"), false),
      m_parent(parent)
{
    setResizeMode(QQuickView::SizeRootObjectToView);

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

    rootContext()->setContextProperty(QStringLiteral("primaryConfigView"), m_parent);

    updateEnabledBorders();
    syncGeometry();
}

void CanvasConfigView::syncGeometry()
{
    if (!m_latteView || !m_latteView->layout() || !m_latteView->containment() || !m_parent || !rootObject()) {
        return;
    }

    updateEnabledBorders();

    auto geometry = m_latteView->positioner()->canvasGeometry();

    if (m_geometryWhenVisible == geometry) {
        return;
    }

    m_geometryWhenVisible = geometry;

    setPosition(geometry.topLeft());

    if (m_shellSurface) {
        m_shellSurface->setPosition(geometry.topLeft());
    }

    setMaximumSize(geometry.size());
    setMinimumSize(geometry.size());
    resize(geometry.size());

    //! after placement request to activate the main config window in order to avoid
    //! rare cases of closing settings window from secondaryConfigView->focusOutEvent
    if (m_parent && KWindowSystem::isPlatformX11()) {
        m_parent->requestActivate();
    }
}

bool CanvasConfigView::event(QEvent *e)
{
    bool result = SubConfigView::event(e);

    switch (e->type()) {
    case QEvent::Enter:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
        if (m_parent) {
            m_parent->requestActivate();
        }
        break;
    default:
        break;
    }

    return result;
}

void CanvasConfigView::showEvent(QShowEvent *ev)
{
    if (m_shellSurface) {
        //! under wayland it needs to be set again after its hiding
        m_shellSurface->setPosition(m_geometryWhenVisible.topLeft());
    }

    SubConfigView::showEvent(ev);

    if (!m_latteView) {
        return;
    }

    syncGeometry();

    //! show Canvas on top of all other panels/docks and show
    //! its parent view on top afterwards
    m_corona->wm()->setViewExtraFlags(this, true);

    QTimer::singleShot(100, [this]() {
        //! delay execution in order to take influence after last Canvas on top call
        if (m_parent) {
            m_parent->requestActivate();
        }
    });

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &CanvasConfigView::syncGeometry);

    emit showSignal();
}

void CanvasConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    if (!m_latteView) {
        return;
    }

    const auto *focusWindow = qGuiApp->focusWindow();

    if (focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                         || focusWindow->flags().testFlag(Qt::ToolTip))) {
        return;
    }

    const auto parent = qobject_cast<PrimaryConfigView *>(m_parent);

    if (!parent->hasFocus()) {
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

