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
#include "../liblattedock/extras.h"
#include "../liblattedock/windowsystem.h"

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

    if (targetScreen)
        adaptToScreen(targetScreen);
    else
        adaptToScreen(qGuiApp->primaryScreen());

    connect(this, &DockView::containmentChanged
    , this, [&]() {
        if (!containment())
            return;

        if (!m_visibility) {
            m_visibility = new VisibilityManager(this);
        }

        QAction *lockWidgetsAction = containment()->actions()->action("lock widgets");
        containment()->actions()->removeAction(lockWidgetsAction);
        QAction *removeAction = containment()->actions()->action("remove");
        removeAction->setVisible(false);
        //containment()->actions()->removeAction(removeAction);
        //FIX: hide and not delete in order to disable a nasty behavior from
        //ContainmentInterface. If only one action exists for containment the
        //this action is triggered directly
        QAction *addWidgetsAction = containment()->actions()->action("add widgets");
        addWidgetsAction->setVisible(false);
        //containment()->actions()->removeAction(addWidgetsAction);
        connect(containment(), SIGNAL(statusChanged(Plasma::Types::ItemStatus)), SLOT(statusChanged(Plasma::Types::ItemStatus)));
    }, Qt::DirectConnection);
    auto *dockCorona = qobject_cast<DockCorona *>(this->corona());

    if (dockCorona) {
        connect(dockCorona, &DockCorona::docksCountChanged, this, &DockView::docksCountChanged);
        connect(dockCorona, &DockCorona::dockLocationChanged, this, &DockView::dockLocationChanged);
    }
}

DockView::~DockView()
{
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
    connect(this, &DockView::screenChanged, this, &DockView::adaptToScreen, Qt::QueuedConnection);
    connect(this, &DockView::screenGeometryChanged, this, &DockView::syncGeometry, Qt::QueuedConnection);
    connect(this, &QQuickWindow::widthChanged, this, &DockView::widthChanged);
    connect(this, &QQuickWindow::heightChanged, this, &DockView::heightChanged);
    connect(this, &DockView::localDockGeometryChanged, this, &DockView::updateAbsDockGeometry);
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

void DockView::adaptToScreen(QScreen *screen)
{
    setScreen(screen);

    if (formFactor() == Plasma::Types::Vertical)
        m_maxLength = screen->size().height();
    else
        m_maxLength = screen->size().width();

    if (containment())
        containment()->reactToScreenChange();

    syncGeometry();
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
        QAction *removeAct = containment()->actions()->action(QStringLiteral("remove"));

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

void DockView::showConfigurationInterface(Plasma::Applet *applet)
{
    if (!applet || !applet->containment())
        return;

    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);

    if (m_configView && c && c->isContainment() && c == containment()) {
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

    if (c && containment() && c->isContainment() && c->id() == containment()->id()) {
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

void DockView::resizeWindow()
{
    QSize screenSize = screen()->size();

    if (formFactor() == Plasma::Types::Vertical) {
        const QSize size{maxThickness(), screenSize.height()};
        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);
    } else {
        const QSize size{screenSize.width(), maxThickness()};
        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);
    }
}

void DockView::setLocalDockGeometry(const QRect &geometry)
{
    if (geometry == m_localDockGeometry) {
        return;
    }

    m_localDockGeometry = geometry;
    emit localDockGeometryChanged();
}

void DockView::updateAbsDockGeometry()
{
    if (!m_visibility)
        return;

    QRect absoluteGeometry {x() + m_localDockGeometry.x(), y() + m_localDockGeometry.y(), m_localDockGeometry.width(), m_localDockGeometry.height()};
    m_visibility->updateDockGeometry(absoluteGeometry);
}

void DockView::updatePosition()
{
    if (!containment())
        return;

    const QRect screenGeometry = screen()->geometry();
    QPoint position;
    position = {0, 0};

    switch (location()) {
        case Plasma::Types::TopEdge:
            position = {screenGeometry.x(), screenGeometry.y()};
            m_maxLength = screenGeometry.width();
            break;
        case Plasma::Types::BottomEdge:
            position = {screenGeometry.x(), screenGeometry.y() + screenGeometry.height() - height()};
            m_maxLength = screenGeometry.width();
            break;
        case Plasma::Types::RightEdge:
            position = {screenGeometry.x() + screenGeometry.width() - width(), screenGeometry.y()};
            m_maxLength = screenGeometry.height();
            break;
        case Plasma::Types::LeftEdge:
            position = {screenGeometry.x(), screenGeometry.y()};
            m_maxLength = screenGeometry.height();
            break;
        default:
            qWarning() << "wrong location, couldn't update the panel position"
                       << location();
    }

    emit maxLengthChanged();
    setPosition(position);
}

inline void DockView::syncGeometry()
{
    resizeWindow();
    updatePosition();
    updateAbsDockGeometry();
    qDebug() << "dock geometry:" << qRectToStr(geometry());
}

void DockView::statusChanged(Plasma::Types::ItemStatus status)
{
    if ((status == Plasma::Types::NeedsAttentionStatus) ||
        (status == Plasma::Types::RequiresAttentionStatus)) {
        m_visibility->setBlockHiding(true);
    } else {
        m_visibility->setBlockHiding(false);
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

int DockView::docksCount() const
{
    auto dockCorona = qobject_cast<DockCorona *>(corona());

    if (!dockCorona || !containment())
        return 0;

    return dockCorona->docksCount(containment()->screen());
}

void DockView::updateFormFactor()
{
    if (!containment())
        return;

    switch (location()) {
        case Plasma::Types::TopEdge:
        case Plasma::Types::BottomEdge:
            containment()->setFormFactor(Plasma::Types::Horizontal);
            break;

        case Plasma::Types::LeftEdge:
        case Plasma::Types::RightEdge:
            containment()->setFormFactor(Plasma::Types::Vertical);
            break;

        default:
            qWarning() << "wrong location, couldn't update the panel position" << location();
    }
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

int DockView::maxLength() const
{
    return m_maxLength;
}

void DockView::setMaxLength(int maxLength)
{
    if (m_maxLength == maxLength)
        return;

    m_maxLength = maxLength;
    emit maxLengthChanged();
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

int DockView::shadow() const
{
    return m_shadow;
}

void DockView::setShadow(int shadow)
{
    if (m_shadow == shadow)
        return;

    m_shadow = shadow;
    emit shadowChanged();
}

bool DockView::tasksPresent()
{
    foreach (Plasma::Applet *applet, containment()->applets()) {
        KPluginMetaData meta = applet->kPackage().metadata();

        if (meta.pluginId() == "org.kde.latte.plasmoid")
            return true;
    }

    return false;
}

VisibilityManager *DockView::visibility()
{
    return m_visibility;
}

bool DockView::event(QEvent *e)
{
    emit eventTriggered(e);
    return ContainmentView::event(e);
}

QList<int> DockView::freeEdges() const
{
    if (!corona() || !containment()) {
        const QList<int> emptyEdges;
        return emptyEdges;
    }

    const auto edges = corona()->freeEdges(containment()->screen());
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
    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (!plugin) {
        return actions;
    }

    if (plugin->containment() != containment()) {
        plugin->setContainment(containment());
        // now configure it
        KConfigGroup cfg(containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(containment()->containmentType()));
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
    if (!event || !containment()) {
        return;
    }

    PlasmaQuick::ContainmentView::mouseReleaseEvent(event);
    event->setAccepted(containment()->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
}

void DockView::mousePressEvent(QMouseEvent *event)
{
    if (!event || !containment()) {
        return;
    }

    // PlasmaQuick::ContainmentView::mousePressEvent(event);

    //even if the menu is executed synchronously, other events may be processed
    //by the qml incubator when plasma is loading, so we need to guard there
    if (m_contextMenu) {
        m_contextMenu->close();
        PlasmaQuick::ContainmentView::mousePressEvent(event);
        return;
    }

    //qDebug() << "1...";
    const QString trigger = Plasma::ContainmentActions::eventToString(event);

    if (trigger == "RightButton;NoModifier") {
        Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

        if (!plugin || plugin->contextualActions().isEmpty()) {
            event->setAccepted(false);
            return;
        }

        //qDebug() << "2...";
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
        //qDebug() << "3...";
        //FIXME: very inefficient appletAt() implementation
        Plasma::Applet *applet = 0;

        foreach (Plasma::Applet *appletTemp, containment()->applets()) {
            PlasmaQuick::AppletQuickItem *ai = appletTemp->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai && ai->isVisible() && ai->contains(ai->mapFromItem(contentItem(), event->pos()))) {
                applet = ai->applet();
                break;
            } else {
                ai = 0;
            }
        }

        if (applet) {
            KPluginMetaData meta = applet->kPackage().metadata();

            if ((meta.pluginId() != "org.kde.plasma.systemtray") &&
                (meta.pluginId() != "org.kde.latte.plasmoid")) {
                //qDebug() << "4...";
                QMenu *desktopMenu = new QMenu;
                desktopMenu->setAttribute(Qt::WA_DeleteOnClose);
                m_contextMenu = desktopMenu;

                if (this->mouseGrabberItem()) {
                    //workaround, this fixes for me most of the right click menu behavior
                    if (applet) {
                        KPluginMetaData meta = applet->kPackage().metadata();

                        //gives the systemtray direct right click behavior for its applets
                        if (meta.pluginId() != "org.kde.plasma.systemtray") {
                            this->mouseGrabberItem()->ungrabMouse();
                        }
                    }

                    return;
                }

                //qDebug() << "5...";

                if (applet) {
                    emit applet->contextualActionsAboutToShow();
                    addAppletActions(desktopMenu, applet, event);
                } else {
                    emit containment()->contextualActionsAboutToShow();
                    addContainmentActions(desktopMenu, event);
                }

                //qDebug() << "6...";
                //this is a workaround where Qt now creates the menu widget
                //in .exec before oxygen can polish it and set the following attribute
                desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
                //end workaround
                QPoint pos = event->globalPos();

                if (applet) {
                    desktopMenu->adjustSize();

                    if (screen()) {
                        const QRect scr = screen()->geometry();
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
        }
    }

    PlasmaQuick::ContainmentView::mousePressEvent(event);
}

void DockView::addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    if (!containment()) {
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

    QMenu *containmentMenu = new QMenu(i18nc("%1 is the name of the containment", "%1 Options", containment()->title()), desktopMenu);
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

    if (containment()->immutability() == Plasma::Types::Mutable &&
        (containment()->containmentType() != Plasma::Types::PanelContainment || containment()->isUserConfiguring())) {
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
    if (!containment()) {
        return;
    }

    if (containment()->corona()->immutability() != Plasma::Types::Mutable &&
        !KAuthorized::authorizeAction(QStringLiteral("plasma/containment_actions"))) {
        //qDebug() << "immutability";
        return;
    }

    //this is what ContainmentPrivate::prepareContainmentActions was
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    //"RightButton;NoModifier"
    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (!plugin) {
        return;
    }

    if (plugin->containment() != containment()) {
        plugin->setContainment(containment());
        // now configure it
        KConfigGroup cfg(containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    QList<QAction *> actions = plugin->contextualActions();

    if (actions.isEmpty()) {
        //it probably didn't bother implementing the function. give the user a chance to set
        //a better plugin.  note that if the user sets no-plugin this won't happen...
        if ((containment()->containmentType() != Plasma::Types::PanelContainment &&
             containment()->containmentType() != Plasma::Types::CustomPanelContainment) &&
            containment()->actions()->action(QStringLiteral("configure"))) {
            desktopMenu->addAction(containment()->actions()->action(QStringLiteral("configure")));
        }
    } else {
        desktopMenu->addActions(actions);
    }

    return;
}
//!END overriding context menus behavior

}
//!END namespace
