/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "dockconfigview.h"

// local
#include "view.h"
#include "panelshadows_p.h"
#include "../lattecorona.h"
#include "../layoutmanager.h"
#include "../settings/universalsettings.h"
#include "../wm/abstractwindowinterface.h"

// Qt
#include <QFontMetrics>
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

// KDE
#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

// Plasma
#include <Plasma/Package>

namespace Latte {

DockConfigView::DockConfigView(Plasma::Containment *containment, Latte::View *view, QWindow *parent)
    : PlasmaQuick::ConfigView(containment, parent),
      m_latteView(view)
{
    m_corona = qobject_cast<Latte::Corona *>(m_latteView->containment()->corona());

    setupWaylandIntegration();

    setScreen(m_latteView->screen());

    if (containment) {
        setIcon(qGuiApp->windowIcon());
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);

    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        setScreen(m_latteView->screen());
        setFlags(wFlags());
        syncGeometry();
        syncSlideEffect();
    });
    connections << connect(m_latteView->visibility(), &VisibilityManager::modeChanged, this, &DockConfigView::syncGeometry);
    connections << connect(containment, &Plasma::Containment::immutabilityChanged, this, &DockConfigView::immutabilityChanged);

    m_thicknessSyncTimer.setSingleShot(true);
    m_thicknessSyncTimer.setInterval(200);
    connections << connect(&m_thicknessSyncTimer, &QTimer::timeout, this, [this]() {
        syncGeometry();
    });

    connections << connect(m_latteView, &Latte::View::normalThicknessChanged, [&]() {
        m_thicknessSyncTimer.start();
    });

    if (m_corona) {
        connections << connect(m_corona, &Latte::Corona::raiseViewsTemporaryChanged, this, &DockConfigView::raiseDocksTemporaryChanged);
    }
}

DockConfigView::~DockConfigView()
{
    qDebug() << "DockConfigView deleting ...";

    deleteSecondaryWindow();

    foreach (auto var, connections) {
        QObject::disconnect(var);
    }

    if (m_shellSurface) {
        delete m_shellSurface;
        m_shellSurface = nullptr;
    }

}

void DockConfigView::init()
{
    qDebug() << "dock config view : initialization started...";

    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    PanelShadows::self()->addWindow(this);
    rootContext()->setContextProperty(QStringLiteral("dock"), m_latteView);
    rootContext()->setContextProperty(QStringLiteral("dockConfig"), this);

    if (m_corona) {
        rootContext()->setContextProperty(QStringLiteral("universalSettings"), m_corona->universalSettings());
        rootContext()->setContextProperty(QStringLiteral("layoutManager"), m_corona->layoutManager());
    }

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
    kdeclarative.setupBindings();

    QByteArray tempFilePath = "lattedockconfigurationui";

    updateEnabledBorders();

    auto source = QUrl::fromLocalFile(m_latteView->containment()->corona()->kPackage().filePath(tempFilePath));
    setSource(source);
    syncGeometry();
    syncSlideEffect();

    qDebug() << "dock config view : initialization ended...";
}

inline Qt::WindowFlags DockConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

QWindow *DockConfigView::secondaryWindow()
{
    return m_secConfigView;
}

void DockConfigView::setAdvanced(bool advanced)
{
    if (m_advanced == advanced) {
        return;
    }

    m_advanced = advanced;

    if (m_advanced) {
        createSecondaryWindow();
    } else {
        deleteSecondaryWindow();
    }
}

void DockConfigView::createSecondaryWindow()
{
    //! do not proceed when secondary window is already created
    //! or when main dock settings window has not updated yet
    //! its geometry
    if (m_secConfigView || geometryWhenVisible().isNull()) {
        return;
    }

    QRect geometry = m_latteView->screenGeometry();

    m_secConfigView = new DockSecConfigView(m_latteView, this);
    m_secConfigView->init();

    if (m_secConfigView->geometryWhenVisible().intersects(geometryWhenVisible())) {
        setShowInlineProperties(true);
        m_secConfigView->hideConfigWindow();
    } else {
        if (!KWindowSystem::isPlatformWayland()) {
            QTimer::singleShot(150, m_secConfigView, SLOT(show()));
        } else {
            QTimer::singleShot(150, [this]() {
                m_secConfigView->setVisible(true);
            });
        }

        setShowInlineProperties(false);
    }
}

void DockConfigView::deleteSecondaryWindow()
{
    if (m_secConfigView) {
        m_secConfigView->deleteLater();
    }
}

QRect DockConfigView::geometryWhenVisible() const
{
    return m_geometryWhenVisible;
}

void DockConfigView::syncGeometry()
{
    if (!m_latteView->managedLayout() || !m_latteView->containment() || !rootObject())
        return;

    const QSize size(rootObject()->width(), rootObject()->height());
    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    const auto location = m_latteView->containment()->location();
    const auto sGeometry = m_latteView->screenGeometry();

    int clearThickness = m_latteView->normalThickness() + m_latteView->fontPixelSize();

    QPoint position{0, 0};

    switch (m_latteView->containment()->formFactor()) {
        case Plasma::Types::Horizontal: {
            if (location == Plasma::Types::TopEdge) {
                position = {sGeometry.center().x() - size.width() / 2
                            , sGeometry.y() + clearThickness
                           };

            } else if (location == Plasma::Types::BottomEdge) {
                position = {sGeometry.center().x() - size.width() / 2
                            , sGeometry.y() + sGeometry.height() - clearThickness - size.height()
                           };

            }
        }
        break;

        case Plasma::Types::Vertical: {
            if (location == Plasma::Types::LeftEdge) {
                position = {sGeometry.x() + clearThickness
                            , sGeometry.center().y() - size.height() / 2
                           };
            } else if (location == Plasma::Types::RightEdge) {
                position = {sGeometry.x() + sGeometry.width() - clearThickness - size.width()
                            , sGeometry.center().y() - size.height() / 2
                           };
            }
        }
        break;

        default:
            qWarning() << "no sync geometry, wrong formFactor";
            break;
    }

    updateEnabledBorders();

    m_geometryWhenVisible = QRect(position.x(), position.y(), size.width(), size.height());

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }

    if (m_advanced) {
        //! consider even the secondary window can be create
        createSecondaryWindow();
    }
}

void DockConfigView::syncSlideEffect()
{
    if (!m_latteView->containment())
        return;

    auto slideLocation = WindowSystem::Slide::None;

    switch (m_latteView->containment()->location()) {
        case Plasma::Types::TopEdge:
            slideLocation = WindowSystem::Slide::Top;
            break;

        case Plasma::Types::RightEdge:
            slideLocation = WindowSystem::Slide::Right;
            break;

        case Plasma::Types::BottomEdge:
            slideLocation = WindowSystem::Slide::Bottom;
            break;

        case Plasma::Types::LeftEdge:
            slideLocation = WindowSystem::Slide::Left;
            break;

        default:
            qDebug() << staticMetaObject.className() << "wrong location";
            break;
    }

    m_corona->wm()->slideWindow(*this, slideLocation);
}

void DockConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    m_corona->wm()->setViewExtraFlags(*this);
    setFlags(wFlags());

    m_corona->wm()->enableBlurBehind(*this);

    syncGeometry();
    syncSlideEffect();

    if (m_latteView && m_latteView->containment())
        m_latteView->containment()->setUserConfiguring(true);

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &DockConfigView::syncGeometry);

    emit showSignal();
}

void DockConfigView::hideEvent(QHideEvent *ev)
{
    if (!m_latteView) {
        QQuickWindow::hideEvent(ev);
        return;
    }

    if (m_latteView->containment())
        m_latteView->containment()->setUserConfiguring(false);

    QQuickWindow::hideEvent(ev);

    const auto mode = m_latteView->visibility()->mode();
    const auto previousByPassWMBehavior = (m_latteView->flags() & Qt::BypassWindowManagerHint) ? true : false;

    if (mode == Types::AlwaysVisible || mode == Types::WindowsGoBelow) {
        if (!previousByPassWMBehavior) {
            m_latteView->managedLayout()->recreateView(m_latteView->containment());
        }
    } else if (m_latteView->byPassWM() != previousByPassWMBehavior) {
        m_latteView->managedLayout()->recreateView(m_latteView->containment());
    }

    deleteLater();
}

void DockConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);

    const auto *focusWindow = qGuiApp->focusWindow();

    if (focusWindow && (focusWindow->flags().testFlag(Qt::Popup)
                        || focusWindow->flags().testFlag(Qt::ToolTip)))
        return;

    if (!m_blockFocusLost && (!m_secConfigView || (m_secConfigView && !m_secConfigView->isActive()))) {
        hideConfigWindow();
    }
}

void DockConfigView::setupWaylandIntegration()
{
    if (m_shellSurface || !KWindowSystem::isPlatformWayland() || !m_latteView || !m_latteView->containment()) {
        // already setup
        return;
    }

    if (m_corona) {
        using namespace KWayland::Client;
        PlasmaShell *interface = m_corona->waylandCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland dock window surface was created...";

        m_shellSurface = interface->createSurface(s, this);
        m_shellSurface->setSkipTaskbar(true);

        syncGeometry();
    }
}

bool DockConfigView::event(QEvent *e)
{
    if (e->type() == QEvent::PlatformSurface) {
        if (auto pe = dynamic_cast<QPlatformSurfaceEvent *>(e)) {
            switch (pe->surfaceEventType()) {
                case QPlatformSurfaceEvent::SurfaceCreated:

                    if (m_shellSurface) {
                        break;
                    }

                    setupWaylandIntegration();
                    break;

                case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                    if (m_shellSurface) {
                        delete m_shellSurface;
                        m_shellSurface = nullptr;
                        qDebug() << "WAYLAND config window surface was deleted...";
                        PanelShadows::self()->removeWindow(this);
                    }

                    break;
            }
        }
    }

    return PlasmaQuick::ConfigView::event(e);
}


void DockConfigView::immutabilityChanged(Plasma::Types::ImmutabilityType type)
{
    if (type != Plasma::Types::Mutable && isVisible())
        hideConfigWindow();
}

bool DockConfigView::sticker() const
{
    return m_blockFocusLost;
}

void DockConfigView::setSticker(bool blockFocusLost)
{
    if (m_blockFocusLost == blockFocusLost)
        return;

    m_blockFocusLost = blockFocusLost;
}

bool DockConfigView::showInlineProperties() const
{
    return m_showInlineProperties;
}
void DockConfigView::setShowInlineProperties(bool show)
{
    if (m_showInlineProperties == show) {
        return;
    }

    m_showInlineProperties = show;
    emit showInlinePropertiesChanged();
}

void DockConfigView::addPanelSpacer()
{
    if (m_latteView && m_latteView->containment()) {
        m_latteView->containment()->createApplet(QStringLiteral("org.kde.latte.spacer"));
    }
}

void DockConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland environment with qt5.9
        close();
    } else {
        hide();
    }
}

void DockConfigView::updateLaunchersForGroup(int groupInt)
{
    Types::LaunchersGroup group = (Types::LaunchersGroup)groupInt;

    //! when the layout/global launchers list is empty then the current dock launchers are used for them
    //! as a start point
    if (m_corona &&  m_latteView->managedLayout()) {
        if ((group == Types::LayoutLaunchers && m_latteView->managedLayout()->launchers().isEmpty())
            || (group == Types::GlobalLaunchers && m_corona->universalSettings()->launchers().isEmpty())) {

            Plasma::Containment *c = m_latteView->containment();

            const auto &applets = c->applets();

            for (auto *applet : applets) {
                KPluginMetaData meta = applet->kPackage().metadata();

                if (meta.pluginId() == "org.kde.latte.plasmoid") {
                    if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
                        const auto &childItems = appletInterface->childItems();

                        if (childItems.isEmpty()) {
                            continue;
                        }

                        for (QQuickItem *item : childItems) {
                            if (auto *metaObject = item->metaObject()) {
                                // not using QMetaObject::invokeMethod to avoid warnings when calling
                                // this on applets that don't have it or other child items since this
                                // is pretty much trial and error.
                                // Also, "var" arguments are treated as QVariant in QMetaObject

                                int methodIndex = metaObject->indexOfMethod("getLauncherList()");

                                if (methodIndex == -1) {
                                    continue;
                                }

                                QMetaMethod method = metaObject->method(methodIndex);

                                QVariant launchers;

                                if (method.invoke(item, Q_RETURN_ARG(QVariant, launchers))) {
                                    if (group == Types::LayoutLaunchers) {
                                        m_latteView->managedLayout()->setLaunchers(launchers.toStringList());
                                    } else if (group == Types::GlobalLaunchers) {
                                        m_corona->universalSettings()->setLaunchers(launchers.toStringList());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

//!BEGIN borders
Plasma::FrameSvg::EnabledBorders DockConfigView::enabledBorders() const
{
    return m_enabledBorders;
}

void DockConfigView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (m_latteView->location()) {
        case Plasma::Types::TopEdge:
            borders &= m_inReverse ? ~Plasma::FrameSvg::BottomBorder : ~Plasma::FrameSvg::TopBorder;
            break;

        case Plasma::Types::LeftEdge:
            borders &= ~Plasma::FrameSvg::LeftBorder;
            break;

        case Plasma::Types::RightEdge:
            borders &= ~Plasma::FrameSvg::RightBorder;
            break;

        case Plasma::Types::BottomEdge:
            borders &= m_inReverse ? ~Plasma::FrameSvg::TopBorder : ~Plasma::FrameSvg::BottomBorder;
            break;

        default:
            break;
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;

        PanelShadows::self()->addWindow(this, m_enabledBorders);

        emit enabledBordersChanged();
    }
}

//!END borders

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
