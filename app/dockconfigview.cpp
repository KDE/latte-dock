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
#include "dockview.h"
#include "dockcorona.h"
#include "panelshadows_p.h"
#include "abstractwindowinterface.h"
#include "../liblattedock/dock.h"

#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>

#include <Plasma/Package>

#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

namespace Latte {

DockConfigView::DockConfigView(Plasma::Containment *containment, DockView *dockView, QWindow *parent)
    : PlasmaQuick::ConfigView(containment, parent),
      m_blockFocusLost(false),
      m_dockView(dockView)
{
    setupWaylandIntegration();

    setScreen(m_dockView->screen());

    if (containment) {
        setIcon(qGuiApp->windowIcon());
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);
    connections << connect(dockView, SIGNAL(screenChanged(QScreen *)), &m_screenSyncTimer, SLOT(start()));
    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        setScreen(m_dockView->screen());
        setFlags(wFlags());
        syncGeometry();
        syncSlideEffect();
    });
    connections << connect(dockView->visibility(), &VisibilityManager::modeChanged, this, &DockConfigView::syncGeometry);
    connections << connect(containment, &Plasma::Containment::immutabilityChanged, this, &DockConfigView::immutabilityChanged);
    connections << connect(containment, &Plasma::Containment::locationChanged, [&]() {
        syncSlideEffect();
        QTimer::singleShot(200, this, &DockConfigView::syncGeometry);
    });

    auto *dockCorona = qobject_cast<DockCorona *>(m_dockView->corona());

    if (dockCorona) {
        connections << connect(this, &DockConfigView::aboutApplication, dockCorona, &DockCorona::aboutApplication);
        connections << connect(dockCorona, SIGNAL(raiseDocksTemporaryChanged()), this, SIGNAL(raiseDocksTemporaryChanged()));
    }
}

DockConfigView::~DockConfigView()
{
    qDebug() << "DockConfigView deleting ...";

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
    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    PanelShadows::self()->addWindow(this);
    rootContext()->setContextProperty(QStringLiteral("dock"), m_dockView);
    rootContext()->setContextProperty(QStringLiteral("dockConfig"), this);
    auto *dockCorona = qobject_cast<DockCorona *>(m_dockView->corona());

    if (dockCorona) {
        rootContext()->setContextProperty(QStringLiteral("universalSettings"), dockCorona->universalSettings());
        rootContext()->setContextProperty(QStringLiteral("layoutManager"), dockCorona->layoutManager());
    }

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
    kdeclarative.setupBindings();
    auto source = QUrl::fromLocalFile(m_dockView->containment()->corona()->kPackage().filePath("lattedockconfigurationui"));
    setSource(source);
    syncGeometry();
    syncSlideEffect();
}

inline Qt::WindowFlags DockConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

void DockConfigView::syncGeometry()
{
    if (!m_dockView->containment() || !rootObject())
        return;

    const auto location = m_dockView->containment()->location();
    const auto sGeometry = screen()->geometry();

    int clearThickness = m_dockView->normalThickness();

    QPoint position{0, 0};

    switch (m_dockView->containment()->formFactor()) {
        case Plasma::Types::Horizontal: {
            const QSize size(rootObject()->width(), rootObject()->height());
            setMaximumSize(size);
            setMinimumSize(size);
            resize(size);

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
            const QSize size(rootObject()->width(), rootObject()->height());
            setMaximumSize(size);
            setMinimumSize(size);
            resize(size);

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

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }
}

void DockConfigView::syncSlideEffect()
{
    if (!m_dockView->containment())
        return;

    auto slideLocation = WindowSystem::Slide::None;

    switch (m_dockView->containment()->location()) {
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

    WindowSystem::self().slideWindow(*this, slideLocation);
}

void DockConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    WindowSystem::self().setDockExtraFlags(*this);
    setFlags(wFlags());

    WindowSystem::self().enableBlurBehind(*this);

    syncGeometry();
    syncSlideEffect();

    if (m_dockView && m_dockView->containment())
        m_dockView->containment()->setUserConfiguring(true);

    m_screenSyncTimer.start();
    QTimer::singleShot(400, this, &DockConfigView::syncGeometry);

    emit showSignal();
}

void DockConfigView::hideEvent(QHideEvent *ev)
{
    if (!m_dockView) {
        QQuickWindow::hideEvent(ev);
        return;
    }

    if (m_dockView->containment())
        m_dockView->containment()->setUserConfiguring(false);

    QQuickWindow::hideEvent(ev);

    auto recreateDock = [&]() noexcept {
        auto *dockCorona = qobject_cast<DockCorona *>(m_dockView->corona());

        if (dockCorona) {
            dockCorona->recreateDock(m_dockView->containment());
        }
    };

    const auto mode = m_dockView->visibility()->mode();
    const auto previousDockWinBehavior = (m_dockView->flags() & Qt::BypassWindowManagerHint) ? false : true;

    if (mode == Dock::AlwaysVisible || mode == Dock::WindowsGoBelow) {
        if (!previousDockWinBehavior) {
            recreateDock();
        }
    } else if (m_dockView->dockWinBehavior() != previousDockWinBehavior) {
        recreateDock();
    }

    deleteLater();
}

void DockConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);
    const auto *focusWindow = qGuiApp->focusWindow();

    if (focusWindow && focusWindow->flags().testFlag(Qt::Popup))
        return;

    if (!m_blockFocusLost)
        hideConfigWindow();
}

void DockConfigView::setupWaylandIntegration()
{
    if (m_shellSurface) {
        // already setup
        return;
    }

    if (DockCorona *c = qobject_cast<DockCorona *>(m_dockView->containment()->corona())) {
        using namespace KWayland::Client;
        PlasmaShell *interface = c->waylandDockCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland dock window surface was created...";

        m_shellSurface = interface->createSurface(s, this);

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
                    }

                    PanelShadows::self()->removeWindow(this);
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

void DockConfigView::setSticker(bool blockFocusLost)
{
    if (m_blockFocusLost == blockFocusLost)
        return;

    m_blockFocusLost = blockFocusLost;
}

void DockConfigView::addPanelSpacer()
{
    if (m_dockView && m_dockView->containment()) {
        m_dockView->containment()->createApplet(QStringLiteral("org.kde.latte.spacer"));
    }
}

void DockConfigView::hideConfigWindow()
{
    if (m_shellSurface) {
        //!NOTE: Avoid crash in wayland enviroment with qt5.9
        close();
    } else {
        hide();
    }
}

void DockConfigView::updateLaunchersForGroup(int groupInt)
{
    Dock::LaunchersGroup group = (Dock::LaunchersGroup)groupInt;

    auto *dockCorona = qobject_cast<DockCorona *>(m_dockView->corona());

    //! when the layout/global launchers list is empty then the current dock launchers are used for them
    //! as a start point
    if (dockCorona && dockCorona->layoutManager() && dockCorona->layoutManager()->currentLayout()) {
        if ((group == Dock::LayoutLaunchers && dockCorona->layoutManager()->currentLayout()->launchers().isEmpty())
            || group == Dock::GlobalLaunchers && dockCorona->universalSettings()->launchers().isEmpty()) {

            Plasma::Containment *c = m_dockView->containment();

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
                                    if (group == Dock::LayoutLaunchers) {
                                        dockCorona->layoutManager()->currentLayout()->setLaunchers(launchers.toStringList());
                                    } else if (group == Dock::GlobalLaunchers) {
                                        dockCorona->universalSettings()->setLaunchers(launchers.toStringList());
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


}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
