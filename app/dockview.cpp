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

#include "dockview.h"
#include "dockconfigview.h"
#include "dockcorona.h"
#include "panelshadows_p.h"
#include "visibilitymanager.h"
#include "../liblattedock/extras.h"

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QMenu>
#include <QMetaEnum>
#include <QVersionNumber>

#include <KActionCollection>
#include <KAuthorized>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowEffects>
#include <KWindowSystem>

#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <PlasmaQuick/AppletQuickItem>

#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

namespace Latte {

//! both alwaysVisible and dockWinBehavior are passed through corona because
//! during the dock window creation containment hasnt been set, but these variables
//! are needed in order for window flags to be set correctly
DockView::DockView(Plasma::Corona *corona, QScreen *targetScreen, bool dockWindowBehavior)
    : PlasmaQuick::ContainmentView(corona),
      m_contextMenu(nullptr)
{
    setTitle(corona->kPackage().metadata().name());
    setIcon(qGuiApp->windowIcon());
    setResizeMode(QuickViewSharedEngine::SizeRootObjectToView);
    setColor(QColor(Qt::transparent));
    setClearBeforeRendering(true);

    const auto flags = Qt::FramelessWindowHint
                       | Qt::WindowStaysOnTopHint
                       | Qt::NoDropShadowWindowHint
                       | Qt::WindowDoesNotAcceptFocus;

    if (dockWindowBehavior) {
        setFlags(flags);
    } else {
        setFlags(flags | Qt::BypassWindowManagerHint);
    }

    KWindowSystem::setOnAllDesktops(winId(), true);

    if (targetScreen)
        setScreenToFollow(targetScreen);
    else
        setScreenToFollow(qGuiApp->primaryScreen());

    connect(this, &DockView::containmentChanged
    , this, [ &, dockWindowBehavior]() {
        qDebug() << "dock view c++ containment changed 1...";

        if (!this->containment())
            return;

        qDebug() << "dock view c++ containment changed 2...";

        setDockWinBehavior(dockWindowBehavior);

        restoreConfig();
        reconsiderScreen();

        if (!m_visibility) {
            m_visibility = new VisibilityManager(this);
        }

        QAction *lockWidgetsAction = this->containment()->actions()->action("lock widgets");
        this->containment()->actions()->removeAction(lockWidgetsAction);
        QAction *removeAction = containment()->actions()->action("remove");
        removeAction->setVisible(false);
        QAction *configureAction = containment()->actions()->action("configure");
        configureAction->setShortcut(QKeySequence());
        //containment()->actions()->removeAction(removeAction);
        //FIX: hide and not delete in order to disable a nasty behavior from
        //ContainmentInterface. If only one action exists for containment the
        //this action is triggered directly
        QAction *addWidgetsAction = this->containment()->actions()->action("add widgets");
        addWidgetsAction->setVisible(false);
        //containment()->actions()->removeAction(addWidgetsAction);
        connect(this->containment(), SIGNAL(statusChanged(Plasma::Types::ItemStatus)), SLOT(statusChanged(Plasma::Types::ItemStatus)));
    }, Qt::DirectConnection);
    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona) {
        connect(dockCorona, &DockCorona::docksCountChanged, this, &DockView::docksCountChanged);
        connect(this, &DockView::docksCountChanged, this, &DockView::totalDocksCountChanged);
        connect(dockCorona, &DockCorona::dockLocationChanged, this, &DockView::dockLocationChanged);
        connect(dockCorona, &DockCorona::dockLocationChanged, this, [&]() {
            //! check if an edge has been freed for a primary dock
            //! from another screen
            if (m_onPrimary) {
                m_screenSyncTimer.start();
            }
        });
    }

    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(2000);
    connect(&m_screenSyncTimer, &QTimer::timeout, this, &DockView::reconsiderScreen);
}

DockView::~DockView()
{
    m_inDelete = true;
    disconnect(corona(), &Plasma::Corona::availableScreenRectChanged, this, &DockView::availableScreenRectChanged);
    disconnect(containment(), SIGNAL(statusChanged(Plasma::Types::ItemStatus)), this, SLOT(statusChanged(Plasma::Types::ItemStatus)));

    m_screenSyncTimer.stop();

    qDebug() << "dock view deleting...";
    rootContext()->setContextProperty(QStringLiteral("dock"), nullptr);

    //! this disconnect does not free up connections correctly when
    //! dockView is deleted. A crash for this example is the following:
    //! switch to Alternative Session and disable compositing,
    //! the signal creating the crash was probably from deleted
    //! windows.
    //! this->disconnect();

    if (m_configView)
        m_configView->setVisible(false);//hide();

    if (m_visibility)
        delete m_visibility;

    if (m_shellSurface) {
        m_shellSurface->release();
        delete m_shellSurface;
        m_shellSurface = nullptr;
    }
}

void DockView::init()
{
    connect(this, &QQuickWindow::screenChanged, this, &DockView::screenChanged);
    connect(this, &QQuickWindow::screenChanged, this, &DockView::docksCountChanged);
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &DockView::screenChanged);
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DockView::screenChanged);
    connect(this, &DockView::screenGeometryChanged, this, &DockView::syncGeometry);
    connect(this, &QQuickWindow::xChanged, this, &DockView::xChanged);
    connect(this, &QQuickWindow::xChanged, this, &DockView::updateAbsDockGeometry);
    connect(this, &QQuickWindow::yChanged, this, &DockView::yChanged);
    connect(this, &QQuickWindow::yChanged, this, &DockView::updateAbsDockGeometry);
    connect(this, &QQuickWindow::widthChanged, this, &DockView::widthChanged);
    connect(this, &QQuickWindow::widthChanged, this, &DockView::updateAbsDockGeometry);
    connect(this, &QQuickWindow::heightChanged, this, &DockView::heightChanged);
    connect(this, &QQuickWindow::heightChanged, this, &DockView::updateAbsDockGeometry);

    connect(corona(), &Plasma::Corona::availableScreenRectChanged, this, &DockView::availableScreenRectChanged);

    connect(this, &DockView::behaveAsPlasmaPanelChanged, this, &DockView::syncGeometry);
    connect(this, &DockView::drawShadowsChanged, this, &DockView::syncGeometry);
    connect(this, &DockView::maxLengthChanged, this, &DockView::syncGeometry);
    connect(this, &DockView::offsetChanged, this, &DockView::syncGeometry);
    connect(this, &DockView::alignmentChanged, this, &DockView::updateEnabledBorders);
    connect(this, &DockView::dockWinBehaviorChanged, this, &DockView::saveConfig);
    connect(this, &DockView::onPrimaryChanged, this, &DockView::saveConfig);
    connect(this, &DockView::onPrimaryChanged, this, &DockView::reconsiderScreen);
    connect(this, &DockView::locationChanged, this, [&]() {
        updateFormFactor();
        syncGeometry();
    });
    connect(this, &DockView::dockTransparencyChanged, this, &DockView::updateEffects);
    connect(this, &DockView::drawEffectsChanged, this, &DockView::updateEffects);
    connect(this, &DockView::effectsAreaChanged, this, &DockView::updateEffects);

    connect(&m_theme, &Plasma::Theme::themeChanged, this, &DockView::updateEffects);

    connect(this, &DockView::normalThicknessChanged, this, [&]() {
        if (m_behaveAsPlasmaPanel) {
            syncGeometry();
        }
    });

    connect(this, SIGNAL(normalThicknessChanged()), corona(), SIGNAL(availableScreenRectChanged()));
    connect(this, SIGNAL(shadowChanged()), corona(), SIGNAL(availableScreenRectChanged()));

    initSignalingForLocationChangeSliding();

    ///!!!!!
    rootContext()->setContextProperty(QStringLiteral("dock"), this);

    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona) {
        rootContext()->setContextProperty(QStringLiteral("universalSettings"), dockCorona->universalSettings());
        rootContext()->setContextProperty(QStringLiteral("layoutManager"), dockCorona->layoutManager());
    }

    setSource(corona()->kPackage().filePath("lattedockui"));
    // setVisible(true);
    syncGeometry();

    if (!KWindowSystem::isPlatformWayland()) {
        setVisible(true);
    }

    qDebug() << "SOURCE:" << source();
}

void DockView::initSignalingForLocationChangeSliding()
{
    connect(this, &DockView::hideDockDuringLocationChangeSignal, this, [&]() {
        setBlockHiding(false);

        if (m_configView) {
            auto configDialog = qobject_cast<DockConfigView *>(m_configView);

            if (configDialog) {
                configDialog->hideConfigWindow();
            }
        }
    });

    connect(this, &DockView::hideDockDuringLocationChangeFinished, this, [&]() {
        setBlockAnimations(true);
        setLocation(m_goToLocation);
    });

    connect(this, &DockView::dockLocationChanged, this, [&]() {
        if (blockAnimations()) {
            QTimer::singleShot(100, [this]() {
                setBlockAnimations(false);
                emit showDockAfterLocationChangeSignal();
                showSettingsWindow();
            });
        }
    });
}


void DockView::availableScreenRectChanged()
{
    if (m_inDelete)
        return;

    if (formFactor() == Plasma::Types::Vertical)
        syncGeometry();
}

void DockView::setupWaylandIntegration()
{
    using namespace KWayland::Client;

    if (m_shellSurface)
        return;

    if (DockCorona *c = qobject_cast<DockCorona *>(corona())) {
        PlasmaShell *interface{c->waylandDockCoronaInterface()};

        if (!interface)
            return;

        Surface *s{Surface::fromWindow(this)};

        if (!s)
            return;

        m_shellSurface = interface->createSurface(s, this);
        qDebug() << "wayland dock window surface was created...";

        m_shellSurface->setSkipTaskbar(true);
        m_shellSurface->setRole(PlasmaShellSurface::Role::Panel);
        m_shellSurface->setPanelBehavior(PlasmaShellSurface::PanelBehavior::WindowsGoBelow);
    }
}

bool DockView::setCurrentScreen(const QString id)
{
    QScreen *nextScreen{qGuiApp->primaryScreen()};

    if (id != "primary") {
        foreach (auto scr, qGuiApp->screens()) {
            if (scr && scr->name() == id) {
                nextScreen = scr;
                break;
            }
        }
    }

    if (m_screenToFollow == nextScreen) {
        return true;
    }

    if (nextScreen) {
        auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

        if (dockCorona) {
            auto freeEdges = dockCorona->freeEdges(nextScreen);

            if (!freeEdges.contains(location())) {
                return false;
            } else {
                setScreenToFollow(nextScreen);
            }
        }
    }

    return true;
}

//! this function updates the dock's associated screen.
//! updateScreenId = true, update also the m_screenToFollowId
//! updateScreenId = false, do not update the m_screenToFollowId
//! that way an explicit dock can be shown in another screen when
//! there isnt a tasks dock running in the system and for that
//! dock its first origin screen is stored and that way when
//! that screen is reconnected the dock will return to its original
//! place
void DockView::setScreenToFollow(QScreen *screen, bool updateScreenId)
{
    if (!screen || m_screenToFollow == screen) {
        return;
    }

    m_screenToFollow = screen;

    if (updateScreenId) {
        m_screenToFollowId = screen->name();
    }

    qDebug() << "adapting to screen...";
    setScreen(screen);

    if (this->containment())
        this->containment()->reactToScreenChange();

    connect(screen, &QScreen::geometryChanged, this, &DockView::screenGeometryChanged);
    syncGeometry();
    updateAbsDockGeometry(true);
    emit screenGeometryChanged();
}

//! the main function which decides if this dock is at the
//! correct screen
void DockView::reconsiderScreen()
{
    qDebug() << "  Delayer  ";

    foreach (auto scr, qGuiApp->screens()) {
        qDebug() << "      D, found screen: " << scr->name();
    }

    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());
    bool screenExists{false};

    //!check if the associated screen is running
    foreach (auto scr, qGuiApp->screens()) {
        if (m_screenToFollowId == scr->name()
            || (onPrimary() && scr == qGuiApp->primaryScreen())) {
            screenExists = true;
        }
    }

    qDebug() << "dock screen exists  ::: " << screenExists;

    //! 1.a primary dock must be always on the primary screen
    //! 2.the last tasks dock must also always on the primary screen
    //! even though it has been configured as an explicit
    if ((m_onPrimary || (tasksPresent() && dockCorona->noDocksWithTasks() == 1 && !screenExists))
        && (m_screenToFollowId != qGuiApp->primaryScreen()->name()
            || m_screenToFollow != qGuiApp->primaryScreen())) {
        //change to primary screen only if the specific edge is free
        qDebug() << "updating the primary screen for dock...";
        qDebug() << "available primary screen edges:" << dockCorona->freeEdges(qGuiApp->primaryScreen());
        qDebug() << "dock location:" << location();

        if (dockCorona->freeEdges(qGuiApp->primaryScreen()).contains(location())) {
            //! case 2
            if (!m_onPrimary && !screenExists && tasksPresent() && (dockCorona->noDocksWithTasks() == 1)) {
                qDebug() << "reached case 2 of updating dock primary screen...";
                setScreenToFollow(qGuiApp->primaryScreen(), false);
            } else {
                //! case 1
                qDebug() << "reached case 1 of updating dock primary screen...";
                setScreenToFollow(qGuiApp->primaryScreen());
            }

            syncGeometry();
        }
    } else if (!m_onPrimary) {
        //! 3.an explicit dock must be always on the correct associated screen
        //! there are cases that window manager misplaces the dock, this function
        //! ensures that this dock will return at its correct screen
        foreach (auto scr, qGuiApp->screens()) {
            if (scr && scr->name() == m_screenToFollowId) {
                qDebug() << "updating the explicit screen for dock...";
                setScreenToFollow(scr);
                syncGeometry();
                break;
            }
        }
    }

    emit docksCountChanged();
}

void DockView::screenChanged(QScreen *scr)
{
    m_screenSyncTimer.start();

    //! this is needed in order to update the struts on screen change
    //! and even though the geometry has been set correctly the offsets
    //! of the screen must be updated to the new ones
    if (m_visibility && m_visibility->mode() == Latte::Dock::AlwaysVisible) {
        updateAbsDockGeometry(true);
    }
}

void DockView::addNewDock()
{
    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona) {
        dockCorona->loadDefaultLayout();
    }
}

void DockView::copyDock()
{
    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona) {
        dockCorona->copyDock(containment());
    }
}

void DockView::removeDock()
{
    if (totalDocksCount() > 1) {
        QAction *removeAct = this->containment()->actions()->action(QStringLiteral("remove"));

        if (removeAct) {
            removeAct->trigger();
        }
    }
}

QQmlListProperty<QScreen> DockView::screens()
{
    return QQmlListProperty<QScreen>(this, nullptr, &countScreens, &atScreens);
}

int DockView::countScreens(QQmlListProperty<QScreen> *property)
{
    Q_UNUSED(property)
    return qGuiApp->screens().count();
}

QScreen *DockView::atScreens(QQmlListProperty<QScreen> *property, int index)
{
    Q_UNUSED(property)
    return qGuiApp->screens().at(index);
}

QString DockView::currentScreen() const
{
    return m_screenToFollowId;
}

bool DockView::settingsWindowIsShown() const
{
    return (m_configView != nullptr);
}

void DockView::showSettingsWindow()
{
    showConfigurationInterface(containment());
}

void DockView::showConfigurationInterface(Plasma::Applet *applet)
{
    if (!applet || !applet->containment())
        return;

    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);

    if (m_configView && c && c->isContainment() && c == this->containment()) {
        if (m_configView->isVisible()) {
            m_configView->setVisible(false);
            //m_configView->hide();
        } else {
            m_configView->setVisible(true);
            //m_configView->show();
        }

        return;
    } else if (m_configView) {
        if (m_configView->applet() == applet) {
            m_configView->setVisible(true);
            //m_configView->show();
            m_configView->requestActivate();
            return;
        } else {
            m_configView->setVisible(false);
            //m_configView->hide();
            m_configView->deleteLater();
        }
    }

    bool delayConfigView = false;

    if (c && containment() && c->isContainment() && c->id() == this->containment()->id()) {
        m_configView = new DockConfigView(c, this);
        delayConfigView = true;
    } else {
        m_configView = new PlasmaQuick::ConfigView(applet);
    }

    m_configView.data()->init();

    if (!delayConfigView) {
        m_configView->setVisible(true);
        //m_configView.data()->show();
    } else {
        //add a timer for showing the configuration window the first time it is
        //created in order to give the containmnent's layouts the time to
        //calculate the window's height
        if (!KWindowSystem::isPlatformWayland()) {
            QTimer::singleShot(150, m_configView, SLOT(show()));
        } else {
            QTimer::singleShot(150, [this]() {
                m_configView->setVisible(true);
            });
        }
    }
}

//! this is used mainly from vertical panels in order to
//! to get the maximum geometry that can be used from the dock
//! based on their alignment type and the location dock
QRect DockView::maximumNormalGeometry()
{
    int xPos = 0;
    int yPos = 0;
    int maxHeight = maxLength() * screen()->geometry().height();
    int maxWidth = normalThickness();
    QRect maxGeometry;
    maxGeometry.setRect(0, 0, maxWidth, maxHeight);

    switch (location()) {
        case Plasma::Types::LeftEdge:
            xPos = screen()->geometry().x();

            switch (alignment()) {
                case Latte::Dock::Top:
                    yPos = screen()->geometry().y();
                    break;

                case Latte::Dock::Center:
                case Latte::Dock::Justify:
                    yPos = qMax(screen()->geometry().center().y() - maxHeight / 2, screen()->geometry().y());
                    break;

                case Latte::Dock::Bottom:
                    yPos = screen()->geometry().bottom() - maxHeight + 1;
                    break;
            }

            maxGeometry.setRect(xPos, yPos, maxWidth, maxHeight);
            break;

        case Plasma::Types::RightEdge:
            xPos = screen()->geometry().right() - maxWidth + 1;

            switch (alignment()) {
                case Latte::Dock::Top:
                    yPos = screen()->geometry().y();
                    break;

                case Latte::Dock::Center:
                case Latte::Dock::Justify:
                    yPos = qMax(screen()->geometry().center().y() - maxHeight / 2, screen()->geometry().y());
                    break;

                case Latte::Dock::Bottom:
                    yPos = screen()->geometry().bottom() - maxHeight + 1;
                    break;
            }

            maxGeometry.setRect(xPos, yPos, maxWidth, maxHeight);
            break;

        default:
            //! bypass clang warnings
            break;
    }

    //! this is needed in order to preserve that the top dock will be above
    //! the others in case flag bypasswindowmanagerhint hasnt be set,
    //! such a case is the AlwaysVisible mode
    if (location() == Plasma::Types::TopEdge) {
        KWindowSystem::setState(winId(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(winId(), NET::KeepAbove);
    }

    return maxGeometry;
}

void DockView::resizeWindow(QRect availableScreenRect)
{
    if (formFactor() == Plasma::Types::Vertical) {
        //qDebug() << "MAXIMUM RECT :: " << maximumRect << " - AVAILABLE RECT :: " << availableRect;
        QSize size{maxThickness(), availableScreenRect.height()};

        if (m_behaveAsPlasmaPanel) {
            size.setWidth(normalThickness());
            size.setHeight(static_cast<int>(maxLength() * availableScreenRect.height()));
        }

        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);
    } else {
        QSize screenSize = this->screen()->size();
        QSize size{screenSize.width(), maxThickness()};

        if (m_behaveAsPlasmaPanel) {
            size.setWidth(static_cast<int>(maxLength() * screenSize.width()));
            size.setHeight(normalThickness());
        }

        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);

        if (corona())
            emit corona()->availableScreenRectChanged();
    }
}

QRect DockView::localGeometry() const
{
    return m_localGeometry;
}

void DockView::setLocalGeometry(const QRect &geometry)
{
    if (m_localGeometry == geometry) {
        return;
    }

    m_localGeometry = geometry;
    emit localGeometryChanged();
    updateAbsDockGeometry();
}

void DockView::updateAbsDockGeometry(bool bypassChecks)
{
    //! there was a -1 in height and width here. The reason of this
    //! if I remember correctly was related to multi-screen but I cant
    //! remember exactly the reason, something related to rigth edge in
    //! multi screen environment. BUT this was breaking the entire AlwaysVisible
    //! experience with struts. Removing them in order to restore correct
    //! behavior and keeping this comment in order to check for
    //! multi-screen breakage
    QRect absGeometry {x() + m_localGeometry.x(), y() + m_localGeometry.y()
                       , m_localGeometry.width(), m_localGeometry.height()};

    if (m_absGeometry == absGeometry && !bypassChecks)
        return;

    m_absGeometry = absGeometry;
    syncGeometry();
    emit absGeometryChanged(m_absGeometry);
}

void DockView::updatePosition(QRect availableScreenRect)
{
    QRect screenGeometry{availableScreenRect};
    QPoint position;
    position = {0, 0};

    const auto length = [&](int length) -> int {
        float offs = static_cast<float>(offset());
        return static_cast<int>(length * ((1 - maxLength()) / 2) + length * (offs / 100));
    };
    int cleanThickness = normalThickness() - shadow();

    switch (location()) {
        case Plasma::Types::TopEdge:
            if (m_behaveAsPlasmaPanel) {
                position = {screenGeometry.x() + length(screenGeometry.width()), screenGeometry.y()};
            } else {
                position = {screenGeometry.x(), screenGeometry.y()};
            }

            break;

        case Plasma::Types::BottomEdge:
            if (m_behaveAsPlasmaPanel) {
                position = {screenGeometry.x() + length(screenGeometry.width()),
                            screenGeometry.y() + screenGeometry.height() - cleanThickness
                           };
            } else {
                position = {screenGeometry.x(), screenGeometry.y() + screenGeometry.height() - height()};
            }

            break;

        case Plasma::Types::RightEdge:
            if (m_behaveAsPlasmaPanel && !mask().isNull()) {
                position = {availableScreenRect.right() - cleanThickness + 1,
                            availableScreenRect.y() + length(availableScreenRect.height())
                           };
            } else {
                position = {availableScreenRect.right() - width() + 1, availableScreenRect.y()};
            }

            break;

        case Plasma::Types::LeftEdge:
            if (m_behaveAsPlasmaPanel && !mask().isNull()) {
                position = {availableScreenRect.x(), availableScreenRect.y() + length(availableScreenRect.height())};
            } else {
                position = {availableScreenRect.x(), availableScreenRect.y()};
            }

            break;

        default:
            qWarning() << "wrong location, couldn't update the panel position"
                       << location();
    }

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }
}

inline void DockView::syncGeometry()
{
    if (!(this->screen() && this->containment()))
        return;

    bool found{false};

    //! before updating the positioning and geometry of the dock
    //! we make sure that the dock is at the correct screen
    if (this->screen() != m_screenToFollow) {
        qDebug() << "Sync Geometry screens incosistent!!!!";
        m_screenSyncTimer.start();
    } else {
        found = true;
    }

    //! if the dock isnt at the correct screen the calculations
    //! are not executed
    if (found) {
        //! compute the free screen rectangle for vertical panels only once
        //! this way the costly QRegion computations are calculated only once
        //! instead of two times (both inside the resizeWindow and the updatePosition)
        QRegion freeRegion;;
        QRect maximumRect;
        QRect availableScreenRect{this->screen()->geometry()};

        if (formFactor() == Plasma::Types::Vertical) {
            freeRegion = corona()->availableScreenRegion(this->containment()->screen());
            maximumRect = maximumNormalGeometry();
            QRegion availableRegion = freeRegion.intersected(maximumRect);
            availableScreenRect = freeRegion.intersected(maximumRect).boundingRect();
            float area = 0;

            //! it is used to choose which or the availableRegion rectangles will
            //! be the one representing dock geometry
            for (int i = 0; i < availableRegion.rectCount(); ++i) {
                QRect rect = availableRegion.rects().at(i);
                //! the area of each rectangle in calculated in squares of 50x50
                //! this is a way to avoid enourmous numbers for area value
                float tempArea = (float)(rect.width() * rect.height()) / 2500;

                if (tempArea > area) {
                    availableScreenRect = rect;
                    area = tempArea;
                }
            }

            if (availableRegion.rectCount() > 1 && m_behaveAsPlasmaPanel)
                m_forceDrawCenteredBorders = true;
            else
                m_forceDrawCenteredBorders = false;
        } else {
            m_forceDrawCenteredBorders = false;
        }

        updateEnabledBorders();
        resizeWindow(availableScreenRect);
        updatePosition(availableScreenRect);
    }

    // qDebug() << "dock geometry:" << qRectToStr(geometry());
}

void DockView::statusChanged(Plasma::Types::ItemStatus status)
{
    if (containment()) {
        if (containment()->status() >= Plasma::Types::NeedsAttentionStatus &&
            containment()->status() != Plasma::Types::HiddenStatus) {
            setBlockHiding(true);
        } else {
            setBlockHiding(false);
        }
    }
}

bool DockView::alternativesIsShown() const
{
    return m_alternativesIsShown;
}

void DockView::setAlternativesIsShown(bool show)
{
    if (m_alternativesIsShown == show) {
        return;
    }

    m_alternativesIsShown = show;

    setBlockHiding(show);
    emit alternativesIsShownChanged();
}

bool DockView::contextMenuIsShown() const
{
    return m_contextMenu;
}

int DockView::currentThickness() const
{
    if (formFactor() == Plasma::Types::Vertical) {
        return m_maskArea.isNull() ? width() : m_maskArea.width() - m_shadow;
    } else {
        return m_maskArea.isNull() ? height() : m_maskArea.height() - m_shadow;
    }
}

int DockView::normalThickness() const
{
    return m_normalThickness;
}

void DockView::setNormalThickness(int thickness)
{
    if (m_normalThickness == thickness) {
        return;
    }

    m_normalThickness = thickness;
    emit normalThicknessChanged();
}

int DockView::docksCount() const
{
    auto dockCorona = qobject_cast<DockCorona *>(corona());

    if (!dockCorona)
        return 0;

    return dockCorona->docksCount(screen());
}

int DockView::totalDocksCount() const
{
    auto dockCorona = qobject_cast<DockCorona *>(corona());

    if (!dockCorona)
        return 0;

    return dockCorona->docksCount();
}

int DockView::docksWithTasks()
{
    auto dockCorona = qobject_cast<DockCorona *>(corona());

    if (!dockCorona)
        return 0;

    return dockCorona->noDocksWithTasks();
}

void DockView::updateFormFactor()
{
    if (!this->containment())
        return;

    switch (location()) {
        case Plasma::Types::TopEdge:
        case Plasma::Types::BottomEdge:
            this->containment()->setFormFactor(Plasma::Types::Horizontal);
            break;

        case Plasma::Types::LeftEdge:
        case Plasma::Types::RightEdge:
            this->containment()->setFormFactor(Plasma::Types::Vertical);
            break;

        default:
            qWarning() << "wrong location, couldn't update the panel position" << location();
    }
}

bool DockView::dockWinBehavior() const
{
    return m_dockWinBehavior;
}

void DockView::setDockWinBehavior(bool dock)
{
    if (m_dockWinBehavior == dock) {
        return;
    }

    m_dockWinBehavior = dock;
    emit dockWinBehaviorChanged();
}

bool DockView::behaveAsPlasmaPanel() const
{
    return m_behaveAsPlasmaPanel;
}

void DockView::setBehaveAsPlasmaPanel(bool behavior)
{
    if (m_behaveAsPlasmaPanel == behavior) {
        return;
    }

    m_behaveAsPlasmaPanel = behavior;

    if (m_behaveAsPlasmaPanel && m_drawShadows) {
        PanelShadows::self()->addWindow(this, enabledBorders());
    } else {
        PanelShadows::self()->removeWindow(this);
        // m_enabledBorders = Plasma::FrameSvg::AllBorders;
        // emit enabledBordersChanged();
    }

    updateEffects();
    emit behaveAsPlasmaPanelChanged();
}

bool DockView::drawShadows() const
{
    return m_drawShadows;
}

void DockView::setDrawShadows(bool draw)
{
    if (m_drawShadows == draw) {
        return;
    }

    m_drawShadows = draw;

    if (m_behaveAsPlasmaPanel && m_drawShadows) {
        PanelShadows::self()->addWindow(this, enabledBorders());
    } else {
        PanelShadows::self()->removeWindow(this);
        //m_enabledBorders = Plasma::FrameSvg::AllBorders;
        //emit enabledBordersChanged();
    }

    emit drawShadowsChanged();
}

bool DockView::drawEffects() const
{
    return m_drawEffects;
}

void DockView::setDrawEffects(bool draw)
{
    if (m_drawEffects == draw) {
        return;
    }

    m_drawEffects = draw;

    emit drawEffectsChanged();
}

bool DockView::onPrimary() const
{
    return m_onPrimary;
}

void DockView::setOnPrimary(bool flag)
{
    if (m_onPrimary == flag) {
        return;
    }

    m_onPrimary = flag;
    emit onPrimaryChanged();
}

float DockView::maxLength() const
{
    return m_maxLength;
}

void DockView::setMaxLength(float length)
{
    if (m_maxLength == length) {
        return;
    }

    m_maxLength = length;
    emit maxLengthChanged();
}

int DockView::maxThickness() const
{
    return m_maxThickness;
}

void DockView::setMaxThickness(int thickness)
{
    if (m_maxThickness == thickness)
        return;

    m_maxThickness = thickness;
    syncGeometry();
    emit maxThicknessChanged();
}

int DockView::alignment() const
{
    return m_alignment;
}

void DockView::setAlignment(int alignment)
{
    Dock::Alignment align = static_cast<Dock::Alignment>(alignment);

    if (m_alignment == alignment) {
        return;
    }

    m_alignment = align;
    emit alignmentChanged();
}

bool DockView::blockAnimations() const
{
    return m_blockAnimations;
}

void DockView::setBlockAnimations(bool block)
{
    if (m_blockAnimations == block) {
        return;
    }

    m_blockAnimations = block;
    emit blockAnimationsChanged();
}

QRect DockView::maskArea() const
{
    return m_maskArea;
}

void DockView::setMaskArea(QRect area)
{
    if (m_maskArea == area)
        return;

    m_maskArea = area;

    if (KWindowSystem::compositingActive()) {
        if (m_behaveAsPlasmaPanel) {
            setMask(QRect());
        } else {
            setMask(m_maskArea);
        }
    } else {
        //! this is used when compositing is disabled and provides
        //! the correct way for the mask to be painted in order for
        //! rounded corners to be shown correctly
        //! the enabledBorders check was added because there was cases
        //! that the mask region wasnt calculated correctly after location changes
        if (!m_background || m_background->enabledBorders() != enabledBorders()) {
            m_background = new Plasma::FrameSvg(this);
        }

        if (m_background->imagePath() != "opaque/dialogs/background") {
            m_background->setImagePath(QStringLiteral("opaque/dialogs/background"));
        }

        m_background->setEnabledBorders(enabledBorders());
        m_background->resizeFrame(area.size());
        QRegion fixedMask = m_background->mask();
        fixedMask.translate(m_maskArea.x(), m_maskArea.y());

        //! fix for KF5.32 that return empty QRegion's for the mask
        if (fixedMask.isEmpty()) {
            fixedMask = QRegion(m_maskArea);
        }

        setMask(fixedMask);
    }

    qDebug() << "dock mask set:" << m_maskArea;
    emit maskAreaChanged();
}


QRect DockView::effectsArea() const
{
    return m_effectsArea;
}

void DockView::setEffectsArea(QRect area)
{
    if (m_effectsArea == area) {
        return;
    }

    m_effectsArea = area;
    emit effectsAreaChanged();
}

QRect DockView::absGeometry() const
{
    return m_absGeometry;
}

QRect DockView::screenGeometry() const
{
    if (this->screen()) {
        QRect geom = this->screen()->geometry();
        return geom;
    }

    return QRect();
}

int DockView::offset() const
{
    return m_offset;
}

void DockView::setOffset(int offset)
{
    if (m_offset == offset) {
        return;
    }

    m_offset = offset;
    emit offsetChanged();
}

int DockView::dockTransparency() const
{
    return m_dockTransparency;
}

void DockView::setDockTransparency(int transparency)
{
    if (m_dockTransparency == transparency) {
        return;
    }

    m_dockTransparency = transparency;
    emit dockTransparencyChanged();
}

int DockView::shadow() const
{
    return m_shadow;
}

void DockView::setShadow(int shadow)
{
    if (m_shadow == shadow)
        return;

    m_shadow = shadow;

    if (m_behaveAsPlasmaPanel) {
        syncGeometry();
    }

    emit shadowChanged();
}

void DockView::hideDockDuringLocationChange(int goToLocation)
{
    m_goToLocation = static_cast<Plasma::Types::Location>(goToLocation);
    emit hideDockDuringLocationChangeSignal();
}


void DockView::setBlockHiding(bool block)
{
    if (!block) {
        auto *configView = qobject_cast<DockConfigView *>(m_configView);

        if (m_alternativesIsShown || (configView && configView->sticker() && configView->isVisible())) {
            return;
        }

        if (m_visibility) {
            m_visibility->setBlockHiding(false);
        }
    } else {
        if (m_visibility) {
            m_visibility->setBlockHiding(true);
        }
    }
}

void DockView::updateEffects()
{
    if (!m_behaveAsPlasmaPanel) {
        if (m_drawEffects && !m_effectsArea.isNull() && !m_effectsArea.isEmpty()) {
            //! this is used when compositing is disabled and provides
            //! the correct way for the mask to be painted in order for
            //! rounded corners to be shown correctly
            if (!m_background) {
                m_background = new Plasma::FrameSvg(this);
            }

            if (m_background->imagePath() != "widgets/panel-background") {
                m_background->setImagePath(QStringLiteral("widgets/panel-background"));
            }

            m_background->setEnabledBorders(enabledBorders());
            m_background->resizeFrame(m_effectsArea.size());
            QRegion fixedMask = m_background->mask();
            fixedMask.translate(m_effectsArea.x(), m_effectsArea.y());

            //! fix for KF5.32 that return empty QRegion's for the mask
            if (fixedMask.isEmpty()) {
                fixedMask = QRegion(m_effectsArea);
            }

            KWindowEffects::enableBlurBehind(winId(), true, fixedMask);

            bool drawBackgroundEffect = m_theme.backgroundContrastEnabled() && (m_dockTransparency == 100);
            //based on Breeze Dark theme behavior the enableBackgroundContrast even though it does accept
            //a QRegion it uses only the first rect. The bug was that for Breeze Dark there was a line
            //at the dock bottom that was distinguishing it from other themes
            KWindowEffects::enableBackgroundContrast(winId(), drawBackgroundEffect,
                    m_theme.backgroundContrast(),
                    m_theme.backgroundIntensity(),
                    m_theme.backgroundSaturation(),
                    fixedMask.boundingRect());
        } else {
            KWindowEffects::enableBlurBehind(winId(), false);
            KWindowEffects::enableBackgroundContrast(winId(), false);
        }
    } else if (m_behaveAsPlasmaPanel && m_drawEffects) {
        KWindowEffects::enableBlurBehind(winId(), true);
        KWindowEffects::enableBackgroundContrast(winId(), m_theme.backgroundContrastEnabled(),
                m_theme.backgroundContrast(),
                m_theme.backgroundIntensity(),
                m_theme.backgroundSaturation());
    } else {
        KWindowEffects::enableBlurBehind(winId(), false);
        KWindowEffects::enableBackgroundContrast(winId(), false);
    }
}

//! remove latte tasks plasmoid
void DockView::removeTasksPlasmoid()
{
    if (!tasksPresent() || !containment()) {
        return;
    }

    foreach (Plasma::Applet *applet, containment()->applets()) {
        KPluginMetaData meta = applet->kPackage().metadata();

        if (meta.pluginId() == "org.kde.latte.plasmoid") {
            QAction *closeApplet = applet->actions()->action(QStringLiteral("remove"));

            if (closeApplet) {
                closeApplet->trigger();
                //! remove only the first found
                return;
            }
        }
    }
}

//! check if the tasks plasmoid exist in the dock
bool DockView::tasksPresent()
{
    if (!this->containment()) {
        return false;
    }

    foreach (Plasma::Applet *applet, this->containment()->applets()) {
        const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

        if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
            return true;
        }
    }

    return false;
}


//! check if the tasks plasmoid exist in the dock
bool DockView::latteTasksPresent()
{
    if (!this->containment()) {
        return false;
    }

    foreach (Plasma::Applet *applet, this->containment()->applets()) {
        KPluginMetaData metadata = applet->pluginMetaData();

        if (metadata.pluginId() == "org.kde.latte.plasmoid") {
            return true;
        }
    }

    return false;
}


//!check if the plasmoid with _name_ exists in the midedata
bool DockView::mimeContainsPlasmoid(QMimeData *mimeData, QString name)
{
    if (!mimeData) {
        return false;
    }

    if (mimeData->hasFormat(QStringLiteral("text/x-plasmoidservicename"))) {
        QString data = mimeData->data(QStringLiteral("text/x-plasmoidservicename"));
        const QStringList appletNames = data.split('\n', QString::SkipEmptyParts);

        foreach (const QString &appletName, appletNames) {
            if (appletName == name)
                return true;
        }
    }

    return false;
}


VisibilityManager *DockView::visibility() const
{
    return m_visibility;
}

bool DockView::event(QEvent *e)
{
    if (!m_inDelete) {
        emit eventTriggered(e);

        switch (e->type()) {
            case QEvent::PlatformSurface:
                if (auto pe = dynamic_cast<QPlatformSurfaceEvent *>(e)) {
                    switch (pe->surfaceEventType()) {
                        case QPlatformSurfaceEvent::SurfaceCreated:
                            setupWaylandIntegration();

                            if (m_shellSurface) {
                                syncGeometry();

                                if (m_drawShadows) {
                                    PanelShadows::self()->addWindow(this, enabledBorders());
                                }
                            }

                            break;

                        case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                            if (m_shellSurface) {
                                m_shellSurface->release();
                                delete m_shellSurface;
                                m_shellSurface = nullptr;
                                qDebug() << "wayland dock window surface was deleted...";
                                PanelShadows::self()->removeWindow(this);
                            }

                            break;
                    }
                }

                break;

            default:
                break;
        }

        return ContainmentView::event(e);
    }

    return false;
}

QList<int> DockView::freeEdges() const
{
    DockCorona *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (!dockCorona) {
        const QList<int> emptyEdges;
        return emptyEdges;
    }

    const auto edges = dockCorona->freeEdges(screen());
    QList<int> edgesInt;

    foreach (Plasma::Types::Location edge, edges) {
        edgesInt.append(static_cast<int>(edge));
    }

    return edgesInt;
}

void DockView::closeApplication()
{
    DockCorona *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (m_configView)
        m_configView->close();

    if (dockCorona)
        dockCorona->closeApplication();
}

void DockView::deactivateApplets()
{
    if (!containment()) {
        return;
    }

    foreach (auto applet, containment()->applets()) {
        PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

        if (ai) {
            ai->setExpanded(false);
        }
    }
}

void DockView::toggleAppletExpanded(const int id)
{
    if (!containment()) {
        return;
    }

    foreach (auto applet, containment()->applets()) {
        if (applet->id() == id) {
            PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai) {
                if (!ai->isActivationTogglesExpanded()) {
                    ai->setActivationTogglesExpanded(true);
                }

                emit applet->activated();
            }
        }
    }
}

QVariantList DockView::containmentActions()
{
    QVariantList actions;
    /*if (containment()->corona()->immutability() != Plasma::Types::Mutable) {
        return actions;
    }*/
    //FIXME: the trigger string it should be better to be supported this way
    //const QString trigger = Plasma::ContainmentActions::eventToString(event);
    const QString trigger = "RightButton;NoModifier";
    Plasma::ContainmentActions *plugin = this->containment()->containmentActions().value(trigger);

    if (!plugin) {
        return actions;
    }

    if (plugin->containment() != this->containment()) {
        plugin->setContainment(this->containment());
        // now configure it
        KConfigGroup cfg(this->containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(this->containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    foreach (QAction *ac, plugin->contextualActions()) {
        actions << QVariant::fromValue<QAction *>(ac);
    }

    return actions;
}


//!BEGIN overriding context menus behavior
void DockView::menuAboutToHide()
{
    m_contextMenu = 0;
    m_visibility->setBlockHiding(false);
    emit contextMenuIsShownChanged();
}


void DockView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event || !this->containment()) {
        return;
    }

    PlasmaQuick::ContainmentView::mouseReleaseEvent(event);
    event->setAccepted(this->containment()->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
}

void DockView::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "Step -1 ...";
    if (!event || !this->containment()) {
        return;
    }

    //qDebug() << "Step 0...";

    //even if the menu is executed synchronously, other events may be processed
    //by the qml incubator when plasma is loading, so we need to guard there
    if (m_contextMenu) {
        //qDebug() << "Step 0.5 ...";
        m_contextMenu->close();
        m_contextMenu = 0;
        emit contextMenuIsShownChanged();
        PlasmaQuick::ContainmentView::mousePressEvent(event);
        return;
    }

    //qDebug() << "1 ...";
    const QString trigger = Plasma::ContainmentActions::eventToString(event);

    if (trigger == "RightButton;NoModifier") {
        Plasma::ContainmentActions *plugin = this->containment()->containmentActions().value(trigger);

        if (!plugin || plugin->contextualActions().isEmpty()) {
            event->setAccepted(false);
            return;
        }

        //qDebug() << "2 ...";
        //the plugin can be a single action or a context menu
        //Don't have an action list? execute as single action
        //and set the event position as action data
        /*if (plugin->contextualActions().length() == 1) {
            QAction *action = plugin->contextualActions().at(0);
            action->setData(event->pos());
            action->trigger();
            event->accept();
            return;
        }*/
        //FIXME: very inefficient appletAt() implementation
        Plasma::Applet *applet = 0;
        bool inSystray = false;

        //! initialize the appletContainsMethod on the first right click
        if (!m_appletContainsMethod.isValid()) {
            updateAppletContainsMethod();
        }

        foreach (Plasma::Applet *appletTemp, this->containment()->applets()) {
            PlasmaQuick::AppletQuickItem *ai = appletTemp->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            bool appletContainsMouse = false;

            if (m_appletContainsMethod.isValid()) {
                QVariant retVal;
                m_appletContainsMethod.invoke(m_appletContainsMethodItem, Qt::DirectConnection, Q_RETURN_ARG(QVariant, retVal)
                                              , Q_ARG(QVariant, appletTemp->id()), Q_ARG(QVariant, event->pos()));
                appletContainsMouse = retVal.toBool();
            } else {
                appletContainsMouse = ai->contains(ai->mapFromItem(contentItem(), event->pos()));
            }

            if (ai && ai->isVisible() && appletContainsMouse) {
                applet = ai->applet();
                KPluginMetaData meta = applet->kPackage().metadata();

                //Try to find applets inside a systray
                if (meta.pluginId() == "org.kde.plasma.systemtray" ||
                    meta.pluginId() == "org.nomad.systemtray") {
                    auto systrayId = applet->config().readEntry("SystrayContainmentId");
                    applet = 0;
                    inSystray = true;
                    Plasma::Containment *cont = containmentById(systrayId.toInt());

                    if (cont) {
                        foreach (Plasma::Applet *appletCont, cont->applets()) {
                            PlasmaQuick::AppletQuickItem *ai2 = appletCont->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

                            if (ai2 && ai2->isVisible() && ai2->contains(ai2->mapFromItem(contentItem(), event->pos()))) {
                                applet = ai2->applet();
                                break;
                            }
                        }
                    }

                    break;
                } else {
                    ai = 0;
                }
            }
        }

        if (!applet && !inSystray) {
            applet = this->containment();
        }

        //qDebug() << "3 ...";

        if (applet) {
            const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

            //qDebug() << "3.5 ...";

            if (!provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
                //qDebug() << "4...";
                QMenu *desktopMenu = new QMenu;
                desktopMenu->setAttribute(Qt::WA_DeleteOnClose);
                m_contextMenu = desktopMenu;

                //! deprecated old code that can be removed if the following plasma approach doesnt
                //! create any issues with context menu creation in Latte
                /*if (this->mouseGrabberItem()) {
                    //workaround, this fixes for me most of the right click menu behavior
                    this->mouseGrabberItem()->ungrabMouse();
                    return;
                }*/

                //!plasma official code
                //this is a workaround where Qt will fail to realise a mouse has been released

                // this happens if a window which does not accept focus spawns a new window that takes focus and X grab
                // whilst the mouse is depressed
                // https://bugreports.qt.io/browse/QTBUG-59044
                // this causes the next click to go missing

                //by releasing manually we avoid that situation
                auto ungrabMouseHack = [this]() {
                    if (this->mouseGrabberItem()) {
                        this->mouseGrabberItem()->ungrabMouse();
                    }
                };

                //pre 5.8.0 QQuickWindow code is "item->grabMouse(); sendEvent(item, mouseEvent)"
                //post 5.8.0 QQuickWindow code is sendEvent(item, mouseEvent); item->grabMouse()
                if (QVersionNumber::fromString(qVersion()) > QVersionNumber(5, 8, 0)) {
                    QTimer::singleShot(0, this, ungrabMouseHack);
                } else {
                    ungrabMouseHack();
                }

                //end workaround
                //!end of plasma official code(workaround)

                //qDebug() << "5 ...";

                if (applet) {
                    //qDebug() << "5.3 ...";
                    emit applet->contextualActionsAboutToShow();
                    addAppletActions(desktopMenu, applet, event);
                } else {
                    //qDebug() << "5.6 ...";
                    emit this->containment()->contextualActionsAboutToShow();
                    addContainmentActions(desktopMenu, event);
                }

                //this is a workaround where Qt now creates the menu widget
                //in .exec before oxygen can polish it and set the following attribute
                desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
                //end workaround
                QPoint pos = event->globalPos();

                if (applet) {
                    //qDebug() << "6 ...";
                    desktopMenu->adjustSize();

                    if (this->screen()) {
                        const QRect scr = this->screen()->geometry();
                        int smallStep = 3;
                        int x = event->globalPos().x() + smallStep;
                        int y = event->globalPos().y() + smallStep;

                        //qDebug()<<x << " - "<<y;

                        if (event->globalPos().x() > scr.center().x()) {
                            x = event->globalPos().x() - desktopMenu->width() - smallStep;
                        }

                        if (event->globalPos().y() > scr.center().y()) {
                            y = event->globalPos().y() - desktopMenu->height() - smallStep;
                        }

                        pos = QPoint(x, y);
                    }
                }

                //qDebug() << "7...";

                if (desktopMenu->isEmpty()) {
                    //qDebug() << "7.5 ...";
                    delete desktopMenu;
                    event->accept();
                    return;
                }

                connect(desktopMenu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
                m_visibility->setBlockHiding(true);
                desktopMenu->popup(pos);
                event->setAccepted(true);
                emit contextMenuIsShownChanged();
                return;
            }

            //qDebug() << "8 ...";
        }

        //qDebug() << "9 ...";
    }

    //qDebug() << "10 ...";
    emit contextMenuIsShownChanged();
    PlasmaQuick::ContainmentView::mousePressEvent(event);
}

//! update the appletContainsPos method from Panel view
void DockView::updateAppletContainsMethod()
{
    for (QQuickItem *item : contentItem()->childItems()) {
        if (auto *metaObject = item->metaObject()) {
            // not using QMetaObject::invokeMethod to avoid warnings when calling
            // this on applets that don't have it or other child items since this
            // is pretty much trial and error.
            // Also, "var" arguments are treated as QVariant in QMetaObject

            int methodIndex = metaObject->indexOfMethod("appletContainsPos(QVariant,QVariant)");

            if (methodIndex == -1) {
                continue;
            }

            m_appletContainsMethod = metaObject->method(methodIndex);
            m_appletContainsMethodItem = item;
        }
    }
}

void DockView::addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    if (!this->containment()) {
        return;
    }

    foreach (QAction *action, applet->contextualActions()) {
        if (action) {
            desktopMenu->addAction(action);
        }
    }

    if (!applet->failedToLaunch()) {
        QAction *runAssociatedApplication = applet->actions()->action(QStringLiteral("run associated application"));

        if (runAssociatedApplication && runAssociatedApplication->isEnabled()) {
            desktopMenu->addAction(runAssociatedApplication);
        }

        QAction *configureApplet = applet->actions()->action(QStringLiteral("configure"));

        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu->addAction(configureApplet);
        }

        QAction *appletAlternatives = applet->actions()->action(QStringLiteral("alternatives"));

        if (appletAlternatives && appletAlternatives->isEnabled() && containment()->isUserConfiguring()) {
            desktopMenu->addAction(appletAlternatives);
        }
    }

    QMenu *containmentMenu = new QMenu(i18nc("%1 is the name of the containment", "%1 Options", this->containment()->title()), desktopMenu);
    addContainmentActions(containmentMenu, event);

    if (!containmentMenu->isEmpty()) {
        int enabled = 0;
        //count number of real actions
        QListIterator<QAction *> actionsIt(containmentMenu->actions());

        while (enabled < 3 && actionsIt.hasNext()) {
            QAction *action = actionsIt.next();

            if (action->isVisible() && !action->isSeparator()) {
                ++enabled;
            }
        }

        desktopMenu->addSeparator();

        if (enabled) {
            //if there is only one, don't create a submenu
            // if (enabled < 2) {
            foreach (QAction *action, containmentMenu->actions()) {
                if (action->isVisible()) {
                    desktopMenu->addAction(action);
                }
            }

            // } else {
            //     desktopMenu->addMenu(containmentMenu);
            // }
        }
    }

    if (this->containment()->immutability() == Plasma::Types::Mutable &&
        (this->containment()->containmentType() != Plasma::Types::PanelContainment || this->containment()->isUserConfiguring())) {
        QAction *closeApplet = applet->actions()->action(QStringLiteral("remove"));

        //qDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu->isEmpty()) {
                desktopMenu->addSeparator();
            }

            //qDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu->addAction(closeApplet);
        }
    }
}

void DockView::addContainmentActions(QMenu *desktopMenu, QEvent *event)
{
    if (!this->containment()) {
        return;
    }

    if (this->containment()->corona()->immutability() != Plasma::Types::Mutable &&
        !KAuthorized::authorizeAction(QStringLiteral("plasma/containment_actions"))) {
        //qDebug() << "immutability";
        return;
    }

    //this is what ContainmentPrivate::prepareContainmentActions was
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    //"RightButton;NoModifier"
    Plasma::ContainmentActions *plugin = this->containment()->containmentActions().value(trigger);

    if (!plugin) {
        return;
    }

    if (plugin->containment() != this->containment()) {
        plugin->setContainment(this->containment());
        // now configure it
        KConfigGroup cfg(this->containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(this->containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    QList<QAction *> actions = plugin->contextualActions();

    if (actions.isEmpty()) {
        //it probably didn't bother implementing the function. give the user a chance to set
        //a better plugin.  note that if the user sets no-plugin this won't happen...
        if ((this->containment()->containmentType() != Plasma::Types::PanelContainment &&
             this->containment()->containmentType() != Plasma::Types::CustomPanelContainment) &&
            this->containment()->actions()->action(QStringLiteral("configure"))) {
            auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

            if (dockCorona) {
                desktopMenu->addAction(dockCorona->layoutManager()->addWidgetsAction());
            }

            desktopMenu->addAction(this->containment()->actions()->action(QStringLiteral("configure")));
        }
    } else {
        auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

        desktopMenu->addSeparator();

        if (dockCorona && dockCorona->layoutManager()->menuLayouts().count() > 1) {
            const QIcon identityIcon = QIcon::fromTheme("user-identity");
            QMenu *layoutsMenu = new QMenu(desktopMenu);

            QAction *layoutsAction = desktopMenu->addMenu(layoutsMenu); //  new QAction(identityIcon, i18n("Layouts"), desktopMenu);
            layoutsAction->setIcon(identityIcon);
            layoutsAction->setCheckable(false);
            layoutsAction->setText(i18n("Layouts"));
            layoutsAction->setStatusTip(i18n("Switch to another layout"));

            foreach (auto layout, dockCorona->layoutManager()->menuLayouts()) {
                QAction *layoutAction = new QAction(layout, layoutsMenu);
                layoutAction->setCheckable(true);

                if (layout == dockCorona->universalSettings()->currentLayoutName()) {
                    layoutAction->setChecked(true);
                } else {
                    layoutAction->setChecked(false);
                }

                connect(layoutAction, &QAction::triggered, this, [this, dockCorona, layout] {
                    dockCorona->layoutManager()->switchToLayout(layout);
                });

                layoutsMenu->addAction(layoutAction);
            }

            layoutsMenu->addSeparator();

            QAction *editLayoutsAction = new QAction(i18n("Configure..."), layoutsMenu);

            connect(editLayoutsAction, &QAction::triggered, this, [this, dockCorona] {
                dockCorona->layoutManager()->showLayoutConfigDialog();
            });

            layoutsMenu->addAction(editLayoutsAction);

        }

        desktopMenu->addAction(dockCorona->layoutManager()->addWidgetsAction());

        desktopMenu->addActions(actions);
    }

    return;
}

Plasma::Containment *DockView::containmentById(uint id)
{
    foreach (auto containment, corona()->containments()) {
        if (id == containment->id()) {
            return containment;
        }
    }

    return 0;
}
//!END overriding context menus behavior

//!BEGIN draw panel shadows outside the dock window
Plasma::FrameSvg::EnabledBorders DockView::enabledBorders() const
{
    return m_enabledBorders;
}

void DockView::updateEnabledBorders()
{
    if (!this->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (location()) {
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
            borders &= ~Plasma::FrameSvg::BottomBorder;
            break;

        default:
            break;
    }

    if ((location() == Plasma::Types::LeftEdge || location() == Plasma::Types::RightEdge)) {
        if (maxLength() == 1 && m_alignment == Dock::Justify && !m_forceDrawCenteredBorders) {
            borders &= ~Plasma::FrameSvg::TopBorder;
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }

        if (m_alignment == Dock::Top && !m_forceDrawCenteredBorders && m_offset == 0) {
            borders &= ~Plasma::FrameSvg::TopBorder;
        }

        if (m_alignment == Dock::Bottom && !m_forceDrawCenteredBorders && m_offset == 0) {
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }
    }

    if (location() == Plasma::Types::TopEdge || location() == Plasma::Types::BottomEdge) {
        if (maxLength() == 1 && m_alignment == Dock::Justify) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
            borders &= ~Plasma::FrameSvg::RightBorder;
        }

        if (m_alignment == Dock::Left && m_offset == 0) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
        }

        if (m_alignment == Dock::Right  && m_offset == 0) {
            borders &= ~Plasma::FrameSvg::RightBorder;
        }
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;
        emit enabledBordersChanged();
    }

    if (!m_behaveAsPlasmaPanel || !m_drawShadows) {
        PanelShadows::self()->removeWindow(this);
    } else {
        PanelShadows::self()->setEnabledBorders(this, borders);
    }
}

//!END draw panel shadows outside the dock window

//!BEGIN configuration functions
void DockView::saveConfig()
{
    if (!this->containment())
        return;

    auto config = this->containment()->config();
    config.writeEntry("onPrimary", m_onPrimary);
    config.writeEntry("dockWindowBehavior", m_dockWinBehavior);
    config.sync();
}

void DockView::restoreConfig()
{
    if (!this->containment())
        return;

    auto config = this->containment()->config();
    setOnPrimary(config.readEntry("onPrimary", true));
    setDockWinBehavior(config.readEntry("dockWindowBehavior", true));
}
//!END configuration functions

}
//!END namespace
