/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "secondaryconfigview.h"

// local
#include <config-latte.h>
#include "primaryconfigview.h"
#include "../panelshadows_p.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

// KDE
#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowEffects>
#include <KWindowSystem>

// Plasma
#include <Plasma/Package>

namespace Latte {
namespace ViewPart {

SecondaryConfigView::SecondaryConfigView(Latte::View *view, PrimaryConfigView *parent)
    : SubConfigView(view, QString("#secondaryconfigview#")),
      m_parent(parent)
{
    connect(this, &QQuickView::widthChanged, this, &SecondaryConfigView::updateEffects);
    connect(this, &QQuickView::heightChanged, this, &SecondaryConfigView::updateEffects);

    connect(this, &QQuickView::statusChanged, [&](QQuickView::Status status) {
        if (status == QQuickView::Ready) {
            updateEffects();
        }
    });

    connections << connect(m_parent, &PrimaryConfigView::availableScreenGeometryChanged, this, &SecondaryConfigView::syncGeometry);

    setParentView(view);
    init();
}

void SecondaryConfigView::init()
{
    SubConfigView::init();

    QByteArray tempFilePath = "lattedocksecondaryconfigurationui";

    updateEnabledBorders();

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();

    if (m_parent && KWindowSystem::isPlatformX11()) {
        m_parent->requestActivate();
    }
}

QRect SecondaryConfigView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
}

void SecondaryConfigView::initParentView(Latte::View *view)
{   
    SubConfigView::initParentView(view);

    rootContext()->setContextProperty(QStringLiteral("primaryConfigView"), m_parent);

    updateEnabledBorders();
    syncGeometry();
}

void SecondaryConfigView::syncGeometry()
{
    if (!m_latteView || !m_latteView->layout() || !m_latteView->containment() || !m_parent || !rootObject()) {
        return;
    }

    const QSize size(rootObject()->width(), rootObject()->height());
    const auto location = m_latteView->containment()->location();
    const auto scrGeometry = m_latteView->screenGeometry();
    const auto availGeometry = m_parent->availableScreenGeometry();
    const auto canvasGeometry = m_latteView->positioner()->canvasGeometry();

    int canvasThickness = m_latteView->formFactor() == Plasma::Types::Vertical ? canvasGeometry.width() : canvasGeometry.height();

    int secondaryConfigSpacing = 2 * m_latteView->fontPixelSize();

    QPoint position{0, 0};

    int xPos{0};
    int yPos{0};

    switch (m_latteView->containment()->formFactor()) {
    case Plasma::Types::Horizontal: {
        if (qApp->isLeftToRight()) {
            xPos = availGeometry.x() + secondaryConfigSpacing;
        } else {
            xPos = availGeometry.x() + availGeometry.width() - size.width() - secondaryConfigSpacing;
        }

        if (location == Plasma::Types::TopEdge) {
            yPos = scrGeometry.y() + canvasThickness;
        } else if (location == Plasma::Types::BottomEdge) {
            yPos = scrGeometry.y() + scrGeometry.height() - canvasThickness - size.height();
        }
    }
        break;

    case Plasma::Types::Vertical: {
        yPos = availGeometry.y() + secondaryConfigSpacing;

        if (location == Plasma::Types::LeftEdge) {
            xPos = scrGeometry.x() + canvasThickness;
        } else if (location == Plasma::Types::RightEdge) {
            xPos = scrGeometry.x() + scrGeometry.width() - canvasThickness - size.width();
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

void SecondaryConfigView::showEvent(QShowEvent *ev)
{
    if (m_shellSurface) {
        //! under wayland it needs to be set again after its hiding
        m_shellSurface->setPosition(m_geometryWhenVisible.topLeft());
    }

    SubConfigView::showEvent(ev);

    if (!m_latteView) {
        return;
    }

    setFlags(wFlags());
    m_corona->wm()->setViewExtraFlags(this, false, Latte::Types::NormalWindow);

    syncGeometry();

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &SecondaryConfigView::syncGeometry);

    emit showSignal();
}

void SecondaryConfigView::focusOutEvent(QFocusEvent *ev)
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

void SecondaryConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }
}

void SecondaryConfigView::updateEffects()
{
    //! Don't apply any effect before the wayland surface is created under wayland
    //! https://bugs.kde.org/show_bug.cgi?id=392890
    if (KWindowSystem::isPlatformWayland() && !m_shellSurface) {
        return;
    }

    //! Don't apply any effect before the wayland surface is created under wayland
    //! https://bugs.kde.org/show_bug.cgi?id=392890
    if (KWindowSystem::isPlatformWayland() && !m_shellSurface) {
        return;
    }

    if (!m_background) {
        m_background = new Plasma::FrameSvg(this);
    }

    if (m_background->imagePath() != "dialogs/background") {
        m_background->setImagePath(QStringLiteral("dialogs/background"));
    }

    m_background->setEnabledBorders(m_enabledBorders);
    m_background->resizeFrame(size());

    QRegion mask = m_background->mask();

    QRegion fixedMask = mask.isNull() ? QRegion(QRect(0,0,width(),height())) : mask;

    if (!fixedMask.isEmpty()) {
        setMask(fixedMask);
    } else {
        setMask(QRegion());
    }

    if (KWindowSystem::compositingActive()) {
        KWindowEffects::enableBlurBehind(winId(), true, fixedMask);
    } else {
        KWindowEffects::enableBlurBehind(winId(), false);
    }
}

//!BEGIN borders
void SecondaryConfigView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (m_latteView->location()) {
    case Plasma::Types::TopEdge:
        borders &= ~Plasma::FrameSvg::TopBorder;
        break;

    case Plasma::Types::LeftEdge:
        borders &= ~Plasma::FrameSvg::LeftBorder;
        break;

    case Plasma::Types::RightEdge:
        borders &= ~Plasma::FrameSvg::RightBorder;
        break;

    case Plasma::Types::BottomEdge:
        borders &=  ~Plasma::FrameSvg::BottomBorder;
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

