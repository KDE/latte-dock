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
#include "visibilitymanager.h"
#include "panelshadows_p.h"
#include "../liblattedock/extras.h"

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QMenu>
#include <QMetaEnum>

#include <KActionCollection>
#include <KAuthorized>
#include <KLocalizedContext>
#include <KLocalizedString>

#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <PlasmaQuick/AppletQuickItem>

namespace Latte {

DockView::DockView(Plasma::Corona *corona, QScreen *targetScreen)
    : PlasmaQuick::ContainmentView(corona),
      m_contextMenu(nullptr)
{
    setVisible(false);
    setTitle(corona->kPackage().metadata().name());
    setIcon(QIcon::fromTheme(corona->kPackage().metadata().iconName()));
    setResizeMode(QuickViewSharedEngine::SizeRootObjectToView);
    setClearBeforeRendering(true);
    setFlags(Qt::FramelessWindowHint
             | Qt::WindowStaysOnTopHint
             | Qt::NoDropShadowWindowHint
             | Qt::WindowDoesNotAcceptFocus);

    if (targetScreen)
        setScreenToFollow(targetScreen);
    else
        setScreenToFollow(qGuiApp->primaryScreen());

    connect(this, &DockView::containmentChanged
    , this, [&]() {

        if (!this->containment())
            return;

        restoreConfig();
        reconsiderScreen();

        if (!m_visibility) {
            m_visibility = new VisibilityManager(this);
        }

        QAction *lockWidgetsAction = this->containment()->actions()->action("lock widgets");
        this->containment()->actions()->removeAction(lockWidgetsAction);
        QAction *removeAction = containment()->actions()->action("remove");
        removeAction->setVisible(false);
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
    m_screenSyncTimer.stop();

    qDebug() << "dock view deleting...";
    rootContext()->setContextProperty(QStringLiteral("dock"), nullptr);
    this->disconnect();
    qDebug() << "dock view connections deleted...";

    if (m_configView) {
        m_configView->hide();
        m_configView->deleteLater();
    }

    if (m_visibility) {
        delete m_visibility;
    }
}

void DockView::init()
{
    connect(this, &QQuickWindow::screenChanged, this, &DockView::screenChanged);

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &DockView::screenChanged);
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &DockView::screenChanged);
    connect(this, &DockView::screenGeometryChanged, this, &DockView::syncGeometry);
    connect(this, &QQuickWindow::xChanged, this, &DockView::xChanged);
    connect(this, &QQuickWindow::yChanged, this, &DockView::yChanged);
    connect(this, &QQuickWindow::widthChanged, this, &DockView::widthChanged);
    connect(this, &QQuickWindow::heightChanged, this, &DockView::heightChanged);

    connect(corona(), &Plasma::Corona::availableScreenRectChanged, this, [&]() {
        if (formFactor() == Plasma::Types::Vertical)
            syncGeometry();
    });
    connect(this, &DockView::drawShadowsChanged, this, &DockView::syncGeometry);
    connect(this, &DockView::maxLengthChanged, this, &DockView::syncGeometry);
    connect(this, &DockView::alignmentChanged, this, &DockView::updateEnabledBorders);

    connect(this, &DockView::onPrimaryChanged, this, &DockView::saveConfig);
    connect(this, &DockView::onPrimaryChanged, this, &DockView::reconsiderScreen);

    connect(this, &DockView::locationChanged, this, [&]() {
        updateFormFactor();
        syncGeometry();
    });

    rootContext()->setContextProperty(QStringLiteral("dock"), this);
    setSource(corona()->kPackage().filePath("lattedockui"));
    setVisible(true);
    syncGeometry();
    qDebug() << "SOURCE:" << source();
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

    syncGeometry();
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
        if (m_screenToFollowId == scr->name())
            screenExists = true;
    }

    qDebug() << "dock screen exists  ::: " << screenExists;

    //! 1.a primary dock must be always on the primary screen
    //! 2.the last tasks dock must also always on the primary screen
    //! even though it has been configured as an explicit
    if ((m_onPrimary || (tasksPresent() && dockCorona->noDocksWithTasks() == 1) && !screenExists)
        && m_screenToFollowId != qGuiApp->primaryScreen()->name()
        && m_screenToFollow != qGuiApp->primaryScreen()) {
        //change to primary screen only if the specific edge is free
        if (dockCorona->freeEdges(qGuiApp->primaryScreen()).contains(location())) {
            connect(qGuiApp->primaryScreen(), &QScreen::geometryChanged, this, &DockView::screenGeometryChanged);

            //! case 2
            if (!m_onPrimary && !screenExists && tasksPresent() && (dockCorona->noDocksWithTasks() == 1)) {
                setScreenToFollow(qGuiApp->primaryScreen(), false);
            } else {
                //! case 1
                setScreenToFollow(qGuiApp->primaryScreen());
            }

            syncGeometry();
        }
    } else {
        //! 3.an explicit dock must be always on the correct associated screen
        //! there are cases that window manager misplaces the dock, this function
        //! ensures that this dock will return at its correct screen
        foreach (auto scr, qGuiApp->screens()) {
            if (scr && scr->name() == m_screenToFollowId) {
                connect(scr, &QScreen::geometryChanged, this, &DockView::screenGeometryChanged);
                setScreenToFollow(scr);
                syncGeometry();
            }
        }
    }

    emit docksCountChanged();
}

void DockView::screenChanged(QScreen *scr)
{
    m_screenSyncTimer.start();
}

void DockView::addNewDock()
{
    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona) {
        dockCorona->loadDefaultLayout();
    }
}

void DockView::removeDock()
{
    if (docksCount() > 1) {
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

void DockView::showConfigurationInterface(Plasma::Applet *applet)
{
    if (!applet || !applet->containment())
        return;

    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);

    if (m_configView && c && c->isContainment() && c == this->containment()) {
        if (m_configView->isVisible()) {
            m_configView->hide();
        } else {
            m_configView->show();
        }

        return;
    } else if (m_configView) {
        if (m_configView->applet() == applet) {
            m_configView->show();
            m_configView->requestActivate();
            return;
        } else {
            m_configView->hide();
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
        m_configView.data()->show();
    } else {
        //add a timer for showing the configuration window the first time it is
        //created in order to give the containmnent's layouts the time to
        //calculate the window's height
        QTimer::singleShot(150, m_configView, SLOT(show()));
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
    }

    return maxGeometry;
}

void DockView::resizeWindow()
{
    if (formFactor() == Plasma::Types::Vertical) {
        QRegion freeRegion = corona()->availableScreenRegion(this->containment()->screen());
        QRect maximumRect = maximumNormalGeometry();
        QRect availableRect = freeRegion.intersected(maximumRect).boundingRect();

        //qDebug() << "MAXIMUM RECT :: " << maximumRect << " - AVAILABLE RECT :: " << availableRect;
        QSize size{maxThickness(), availableRect.height()};

        if (m_drawShadows) {
            size.setWidth(normalThickness());
            size.setHeight(static_cast<int>(maxLength() * availableRect.height()));
        }

        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);
    } else {
        QSize screenSize = this->screen()->size();
        QSize size{screenSize.width(), maxThickness()};

        if (m_drawShadows) {
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

void DockView::setLocalDockGeometry(const QRect &geometry)
{
    updateAbsDockGeometry(geometry);
}

void DockView::updateAbsDockGeometry(const QRect &localDockGeometry)
{
    QRect absGeometry {x() + localDockGeometry.x(), y() + localDockGeometry.y()
                       , localDockGeometry.width() - 1, localDockGeometry.height() - 1};

    if (m_absGeometry == absGeometry)
        return;

    m_absGeometry = absGeometry;
    syncGeometry();
    emit absGeometryChanged(m_absGeometry);
}

void DockView::updatePosition()
{
    QRect screenGeometry;
    QPoint position;
    QRegion freeRegion;
    QRect maximumRect;
    QRect availableRect;

    position = {0, 0};

    const auto length = [&](int length) -> int {
        return static_cast<int>(length * (1 - maxLength()) / 2);
    };

    int cleanThickness = normalThickness() - shadow();

    switch (location()) {
        case Plasma::Types::TopEdge:
            screenGeometry = this->screen()->geometry();

            if (m_drawShadows) {
                position = {screenGeometry.x() + length(screenGeometry.width()), screenGeometry.y()};
            } else {
                position = {screenGeometry.x(), screenGeometry.y()};
            }

            break;

        case Plasma::Types::BottomEdge:
            screenGeometry = this->screen()->geometry();

            if (m_drawShadows) {
                position = {screenGeometry.x() + length(screenGeometry.width()),
                            screenGeometry.y() + screenGeometry.height() - cleanThickness
                           };
            } else {
                position = {screenGeometry.x(), screenGeometry.y() + screenGeometry.height() - height()};
            }

            break;

        case Plasma::Types::RightEdge:
            //screenGeometry = corona()->availableScreenRect(this->containment()->screen());
            freeRegion = corona()->availableScreenRegion(this->containment()->screen());
            maximumRect = maximumNormalGeometry();
            availableRect = freeRegion.intersected(maximumRect).boundingRect();

            if (m_drawShadows && !mask().isNull()) {
                position = {availableRect.right() - cleanThickness + 1,
                            availableRect.y() + length(availableRect.height())
                           };
            } else {
                position = {availableRect.right() - width() + 1, availableRect.y()};
            }

            break;

        case Plasma::Types::LeftEdge:
            //screenGeometry = corona()->availableScreenRect(this->containment()->screen());
            freeRegion = corona()->availableScreenRegion(this->containment()->screen());
            maximumRect = maximumNormalGeometry();
            availableRect = freeRegion.intersected(maximumRect).boundingRect();

            if (m_drawShadows && !mask().isNull()) {
                position = {availableRect.x(), availableRect.y() + length(availableRect.height())};
            } else {
                position = {availableRect.x(), availableRect.y()};
            }

            break;

        default:
            qWarning() << "wrong location, couldn't update the panel position"
                       << location();
    }

    setPosition(position);
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
        updateEnabledBorders();
        resizeWindow();
        updatePosition();
    }

    // qDebug() << "dock geometry:" << qRectToStr(geometry());
}

void DockView::statusChanged(Plasma::Types::ItemStatus status)
{
    if (containment()) {
        if (containment()->status() >= Plasma::Types::NeedsAttentionStatus &&
            containment()->status() != Plasma::Types::HiddenStatus) {
            m_visibility->setBlockHiding(true);
        } else {
            m_visibility->setBlockHiding(false);
        }
    }
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

    if (!dockCorona || !this->containment())
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

    if (m_drawShadows) {
        PanelShadows::self()->addWindow(this, enabledBorders());
    } else {
        PanelShadows::self()->removeWindow(this);
        m_enabledBorders = Plasma::FrameSvg::AllBorders;
        emit enabledBordersChanged();
    }

    emit drawShadowsChanged();
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

QRect DockView::maskArea() const
{
    return m_maskArea;
}

void DockView::setMaskArea(QRect area)
{
    if (m_maskArea == area)
        return;

    m_maskArea = area;
    setMask(m_maskArea);
    //qDebug() << "dock mask set:" << m_maskArea;
    emit maskAreaChanged();
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

int DockView::shadow() const
{
    return m_shadow;
}

void DockView::setShadow(int shadow)
{
    if (m_shadow == shadow)
        return;

    m_shadow = shadow;

    if (m_drawShadows) {
        syncGeometry();
    }

    emit shadowChanged();
}

//! check if the tasks plasmoid exist in the dock
bool DockView::tasksPresent()
{
    foreach (Plasma::Applet *applet, this->containment()->applets()) {
        KPluginMetaData meta = applet->kPackage().metadata();

        if (meta.pluginId() == "org.kde.latte.plasmoid")
            return true;
    }

    return false;
}

VisibilityManager *DockView::visibility() const
{
    return m_visibility;
}

bool DockView::event(QEvent *e)
{
    emit eventTriggered(e);

    if (e->type() == QEvent::Leave) {
        engine()->collectGarbage();
        engine()->trimComponentCache();
        //! Important! this code creates a crash when there are two docks
        //! running and the user clicks the Quit button, it is also
        //! suspicious for some rare cases when removing a dock and the
        //! dock is deleted after the 1min time limit of plasma
        //!     engine()->clearComponentCache();
    }

    return ContainmentView::event(e);
}

QList<int> DockView::freeEdges() const
{
    if (!this->corona() || !this->containment()) {
        const QList<int> emptyEdges;
        return emptyEdges;
    }

    const auto edges = corona()->freeEdges(this->containment()->screen());
    QList<int> edgesInt;

    foreach (Plasma::Types::Location edge, edges) {
        edgesInt.append(static_cast<int>(edge));
    }

    return edgesInt;
}

void DockView::closeApplication()
{
    DockCorona *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona)
        dockCorona->closeApplication();
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

        foreach (Plasma::Applet *appletTemp, this->containment()->applets()) {
            PlasmaQuick::AppletQuickItem *ai = appletTemp->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai && ai->isVisible() && ai->contains(ai->mapFromItem(contentItem(), event->pos()))) {
                applet = ai->applet();
                KPluginMetaData meta = applet->kPackage().metadata();

                //Try to find applets inside a systray
                if (meta.pluginId() == "org.kde.plasma.systemtray") {
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
            KPluginMetaData meta = applet->kPackage().metadata();

            //qDebug() << "3.5 ...";

            if (meta.pluginId() != "org.kde.latte.plasmoid") {
                //qDebug() << "4...";
                QMenu *desktopMenu = new QMenu;
                desktopMenu->setAttribute(Qt::WA_DeleteOnClose);
                m_contextMenu = desktopMenu;

                if (this->mouseGrabberItem()) {
                    //workaround, this fixes for me most of the right click menu behavior
                    this->mouseGrabberItem()->ungrabMouse();

                    return;
                }

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
                return;
            }

            //qDebug() << "8 ...";
        }

        //qDebug() << "9 ...";
    }

    //qDebug() << "10 ...";
    PlasmaQuick::ContainmentView::mousePressEvent(event);
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

        if (appletAlternatives && appletAlternatives->isEnabled()) {
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
            if (enabled < 2) {
                foreach (QAction *action, containmentMenu->actions()) {
                    if (action->isVisible() && !action->isSeparator()) {
                        desktopMenu->addAction(action);
                    }
                }
            } else {
                desktopMenu->addMenu(containmentMenu);
            }
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
            desktopMenu->addAction(this->containment()->actions()->action(QStringLiteral("configure")));
        }
    } else {
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
    // qDebug() << "draw shadow!!!! :" << m_drawShadows;

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
        if (maxLength() == 1 && m_alignment == Dock::Justify) {
            borders &= ~Plasma::FrameSvg::TopBorder;
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }

        if (m_alignment == Dock::Top) {
            borders &= ~Plasma::FrameSvg::TopBorder;
        }

        if (m_alignment == Dock::Bottom) {
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }

    }

    if (location() == Plasma::Types::TopEdge || location() == Plasma::Types::BottomEdge) {
        if (maxLength() == 1 && m_alignment == Dock::Justify) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
            borders &= ~Plasma::FrameSvg::RightBorder;
        }

        if (m_alignment == Dock::Left) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
        }

        if (m_alignment == Dock::Right) {
            borders &= ~Plasma::FrameSvg::RightBorder;
        }
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;
        emit enabledBordersChanged();
    }

    if (!m_drawShadows) {
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
    this->containment()->configNeedsSaving();
}

void DockView::restoreConfig()
{
    if (!this->containment())
        return;

    auto config = this->containment()->config();
    setOnPrimary(config.readEntry("onPrimary", true));
}
//!END configuration functions

}
//!END namespace
