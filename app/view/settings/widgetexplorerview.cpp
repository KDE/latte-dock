/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "widgetexplorerview.h"

// local
#include "../panelshadows_p.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QQuickItem>
#include <QScreen>

// KDE
#include <KWindowEffects>
#include <KWindowSystem>
#include <KWayland/Client/plasmashell.h>

// Plasma
#include <Plasma/Package>

namespace Latte {
namespace ViewPart {

WidgetExplorerView::WidgetExplorerView(Latte::View *view)
    : SubConfigView(view, QString("#widgetexplorerview#"), true)
{
    setResizeMode(QQuickView::SizeRootObjectToView);
    //!set flags early in order for wayland to initialize properly
    setFlags(wFlags());

    connect(this, &QQuickView::widthChanged, this, &WidgetExplorerView::updateEffects);
    connect(this, &QQuickView::heightChanged, this, &WidgetExplorerView::updateEffects);

    connect(this, &QQuickView::statusChanged, [&](QQuickView::Status status) {
        if (status == QQuickView::Ready) {
            updateEffects();
        }
    });

    setParentView(view);
    init();
}

void WidgetExplorerView::init()
{
    SubConfigView::init();

    QByteArray tempFilePath = "widgetexplorerui";

    updateEnabledBorders();

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
}

bool WidgetExplorerView::hideOnWindowDeactivate() const
{
    return m_hideOnWindowDeactivate;
}

void WidgetExplorerView::setHideOnWindowDeactivate(bool hide)
{
    if (m_hideOnWindowDeactivate == hide) {
        return;
    }

    m_hideOnWindowDeactivate = hide;
    emit hideOnWindowDeactivateChanged();
}

Qt::WindowFlags WidgetExplorerView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

QRect WidgetExplorerView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
}

void WidgetExplorerView::initParentView(Latte::View *view)
{
    SubConfigView::initParentView(view);

    rootContext()->setContextProperty(QStringLiteral("containmentFromView"), m_latteView->containment());
    rootContext()->setContextProperty(QStringLiteral("latteView"), m_latteView);

    updateEnabledBorders();
    syncGeometry();
}

QRect WidgetExplorerView::availableScreenGeometry() const
{
    int currentScrId = m_latteView->positioner()->currentScreenId();

    QList<Latte::Types::Visibility> ignoreModes{Latte::Types::SidebarOnDemand,Latte::Types::SidebarAutoHide};

    if (m_latteView->visibility() && m_latteView->visibility()->isSidebar()) {
        ignoreModes.removeAll(Latte::Types::SidebarOnDemand);
        ignoreModes.removeAll(Latte::Types::SidebarAutoHide);
    }

    QString activityid = m_latteView->layout()->lastUsedActivity();

    return m_corona->availableScreenRectWithCriteria(currentScrId, activityid, ignoreModes, {}, false, true);
}

void WidgetExplorerView::syncGeometry()
{
    if (!m_latteView || !m_latteView->layout() || !m_latteView->containment() || !rootObject()) {
        return;
    }
    const QSize size(rootObject()->width(), rootObject()->height());
    auto availGeometry = availableScreenGeometry();

    int margin = availGeometry.height() == m_latteView->screenGeometry().height() ? 100 : 0;
    auto geometry = QRect(availGeometry.x(), availGeometry.y(), size.width(), availGeometry.height()-margin);

    updateEnabledBorders();

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
}

void WidgetExplorerView::showEvent(QShowEvent *ev)
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

    requestActivate();

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &WidgetExplorerView::syncGeometry);

    emit showSignal();
}

void WidgetExplorerView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    if (!m_latteView) {
        return;
    }

    hideConfigWindow();
}

void WidgetExplorerView::updateEffects()
{
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

void WidgetExplorerView::hideConfigWindow()
{
    if (!m_hideOnWindowDeactivate) {
        return;
    }

    deleteLater();

    /*QTimer::singleShot(100, [this]() {
        //! avoid crashes under wayland because some mouse events are sended after the surface is destroyed

        if (m_shellSurface) {
            //!NOTE: Avoid crash in wayland environment with qt5.9
            close();
        } else {
            hide();
        }
    });*/
}

void WidgetExplorerView::syncSlideEffect()
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    auto slideLocation = WindowSystem::AbstractWindowInterface::Slide::Left;

    m_corona->wm()->slideWindow(*this, slideLocation);
}

//!BEGIN borders
void WidgetExplorerView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    if (!m_geometryWhenVisible.isEmpty()) {
        if (m_geometryWhenVisible.x() == m_latteView->screenGeometry().x()) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
        }

        if (m_geometryWhenVisible.y() == m_latteView->screenGeometry().y()) {
            borders &= ~Plasma::FrameSvg::TopBorder;
        }

        if (m_geometryWhenVisible.height() == m_latteView->screenGeometry().height()) {
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }
    }

    if (m_enabledBorders != borders) {
        if (isVisible()) {
            m_enabledBorders = borders;
        }
        m_corona->dialogShadows()->addWindow(this, m_enabledBorders);

        emit enabledBordersChanged();
    }
}

//!END borders

}
}

