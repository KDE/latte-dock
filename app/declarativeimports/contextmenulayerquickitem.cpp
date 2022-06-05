/*
    SPDX-FileCopyrightText: 2022 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "contextmenulayerquickitem.h"

// local
#include "../lattecorona.h"
#include "../layouts/storage.h"
#include "../view/view.h"

// Qt
#include <QMouseEvent>
#include <QVersionNumber>
#include <QLatin1String>

// KDE
#include <KAcceleratorManager>
#include <KActionCollection>
#include <KAuthorized>
#include <KLocalizedString>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <PlasmaQuick/AppletQuickItem>

namespace Latte {

ContextMenuLayerQuickItem::ContextMenuLayerQuickItem(QQuickItem *parent) :
    QQuickItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

ContextMenuLayerQuickItem::~ContextMenuLayerQuickItem()
{
}

bool ContextMenuLayerQuickItem::menuIsShown() const
{
    return m_contextMenu != nullptr;
}

QObject *ContextMenuLayerQuickItem::view() const
{
    return m_latteView;
}

void ContextMenuLayerQuickItem::setView(QObject *view)
{
    if (m_latteView == view) {
        return;
    }

    m_latteView = qobject_cast<Latte::View *>(view);
    emit viewChanged();
}

void ContextMenuLayerQuickItem::onMenuAboutToHide()
{
    if (!m_latteView) {
        return;
    }

    m_latteView->containment()->setStatus(m_lastContainmentStatus);
    m_contextMenu = nullptr;
    emit menuChanged();
}

QPoint ContextMenuLayerQuickItem::popUpRelevantToParent(const QRect &parentItem, const QRect popUpRect)
{
    QPoint resultPoint;

    if (!m_latteView) {
        return resultPoint;
    }

    if (m_latteView->location() == Plasma::Types::TopEdge) {
        resultPoint.setX(parentItem.left());
        resultPoint.setY(parentItem.bottom());
    } else if (m_latteView->location() == Plasma::Types::BottomEdge) {
        resultPoint.setX(parentItem.left());
        resultPoint.setY(parentItem.top() - popUpRect.height() - 1);
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        resultPoint.setX(parentItem.right());
        resultPoint.setY(parentItem.top());
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        resultPoint.setX(parentItem.left() - popUpRect.width());
        resultPoint.setY(parentItem.top());
    }

    return resultPoint;
}

QPoint ContextMenuLayerQuickItem::popUpRelevantToGlobalPoint(const QRect &parentItem, const QRect popUpRect)
{
    QPoint resultPoint;

    if (!m_latteView) {
        return resultPoint;
    }

    if (m_latteView->location() == Plasma::Types::TopEdge) {
        resultPoint.setX(popUpRect.x());
        resultPoint.setY(popUpRect.y() + 1);
    } else if (m_latteView->location() == Plasma::Types::BottomEdge) {
        resultPoint.setX(popUpRect.x());
        resultPoint.setY(popUpRect.y() - popUpRect.height() - 1);
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        resultPoint.setX(popUpRect.x() + 1);
        resultPoint.setY(popUpRect.y());
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        resultPoint.setX(popUpRect.x() - popUpRect.width() - 1);
        resultPoint.setY(popUpRect.y());
    }

    return resultPoint;
}

QPoint ContextMenuLayerQuickItem::popUpTopLeft(Plasma::Applet *applet, const QRect popUpRect)
{
    PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

    QRect globalItemRect = m_latteView->absoluteGeometry();

    if (ai && applet != m_latteView->containment()) {
        QPointF appletGlobalTopLeft = ai->mapToGlobal(QPointF(ai->x(), ai->y()));
        globalItemRect = QRect(appletGlobalTopLeft.x(), appletGlobalTopLeft.y(), ai->width(), ai->height());
    }

    int itemLength = (m_latteView->formFactor() == Plasma::Types::Horizontal ? globalItemRect.width() : globalItemRect.height());
    int menuLength = (m_latteView->formFactor() == Plasma::Types::Horizontal ? popUpRect.width() : popUpRect.height());

    if ((itemLength > menuLength)
            || (applet == m_latteView->containment())
            || (m_latteView && Layouts::Storage::self()->isSubContainment(m_latteView->corona(), applet)) ) {
        return popUpRelevantToGlobalPoint(globalItemRect, popUpRect);
    } else {
        return popUpRelevantToParent(globalItemRect, popUpRect);
    }
}


void ContextMenuLayerQuickItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (!event || !m_latteView) {
        return;
    }

    event->setAccepted(m_latteView->containment()->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
    emit menuChanged();
}

void ContextMenuLayerQuickItem::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "Step -1 ...";

    if (!event || !m_latteView || !m_latteView->containment()) {
        return;
    }

    //qDebug() << "Step 0...";

    //even if the menu is executed synchronously, other events may be processed
    //by the qml incubator when plasma is loading, so we need to guard there
    if (m_contextMenu) {
        //qDebug() << "Step 0.5 ...";
        m_contextMenu->close();
        m_contextMenu = nullptr;
        return;
    }

    //qDebug() << "1 ...";
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = m_latteView->containment()->containmentActions().value(trigger);

    if (!plugin || plugin->contextualActions().isEmpty()) {
        event->setAccepted(false);
        return;
    }

    // the plugin can be a single action or a context menu
    // Don't have an action list? execute as single action
    // and set the event position as action data
    if (plugin->contextualActions().length() == 1) {
        QAction *action = plugin->contextualActions().at(0);
        action->setData(event->pos());
        action->trigger();
        event->accept();
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

    //! initialize the appletContainsMethod on the first right click
    if (!m_appletContainsMethod.isValid()) {
        updateAppletContainsMethod();
    }

    for (const Plasma::Applet *appletTemp : m_latteView->containment()->applets()) {
        PlasmaQuick::AppletQuickItem *ai = appletTemp->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

        bool appletContainsMouse = false;

        if (m_appletContainsMethod.isValid()) {
            QVariant retVal;
            m_appletContainsMethod.invoke(m_appletContainsMethodItem, Qt::DirectConnection, Q_RETURN_ARG(QVariant, retVal)
                                          , Q_ARG(QVariant, appletTemp->id()), Q_ARG(QVariant, event->pos()));
            appletContainsMouse = retVal.toBool();
        } else {
            appletContainsMouse = ai->contains(ai->mapFromItem(this, event->pos()));
        }

        if (ai && ai->isVisible() && appletContainsMouse) {
            applet = ai->applet();
            break;
        }
    }

    if (!applet) {
        applet = m_latteView->containment();
    }

    //qDebug() << "3 ...";

    QMenu *desktopMenu = new QMenu;

    //this is a workaround where Qt now creates the menu widget
    //in .exec before oxygen can polish it and set the following attribute
    desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
    //end workaround

    if (desktopMenu->winId()) {
        desktopMenu->windowHandle()->setTransientParent(window());
    }

    desktopMenu->setAttribute(Qt::WA_DeleteOnClose);
    m_contextMenu = desktopMenu;
    emit menuChanged();

    //end workaround
    //!end of plasma official code(workaround)

    //qDebug() << "5 ...";

    emit m_latteView->containment()->contextualActionsAboutToShow();

    if (applet && applet != m_latteView->containment()) {
        //qDebug() << "5.3 ...";
        emit applet->contextualActionsAboutToShow();
        addAppletActions(desktopMenu, applet, event);
    } else {
        //qDebug() << "5.6 ...";
        addContainmentActions(desktopMenu, event);
    }

    //!plasma official code
    //this is a workaround where Qt will fail to realize a mouse has been released

    // this happens if a window which does not accept focus spawns a new window that takes focus and X grab
    // whilst the mouse is depressed
    // https://bugreports.qt.io/browse/QTBUG-59044
    // this causes the next click to go missing

    //by releasing manually we avoid that situation
    auto ungrabMouseHack = [this]() {
        if (window() && window()->mouseGrabberItem()) {
            window()->mouseGrabberItem()->ungrabMouse();
        }
    };

    //post 5.8.0 QQuickWindow code is sendEvent(item, mouseEvent); item->grabMouse()
    QTimer::singleShot(0, this, ungrabMouseHack);

    //this is a workaround where Qt now creates the menu widget
    //in .exec before oxygen can polish it and set the following attribute
    desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
    //end workaround
    QPoint globalPos = event->globalPos();
    desktopMenu->adjustSize();

    QRect popUpRect(globalPos.x(), globalPos.y(), desktopMenu->width(), desktopMenu->height());

    if (applet) {
        globalPos = popUpTopLeft(applet, popUpRect);
    } else {
        globalPos = popUpRelevantToGlobalPoint(QRect(0,0,0,0), popUpRect);
    }

    //qDebug() << "7...";

    if (desktopMenu->isEmpty()) {
        //qDebug() << "7.5 ...";
        delete desktopMenu;
        event->accept();
        return;
    }

    // Bug 344205 keep panel visible while menu is open
    m_lastContainmentStatus = m_latteView->containment()->status();
    m_latteView->containment()->setStatus(Plasma::Types::RequiresAttentionStatus);

    connect(desktopMenu, SIGNAL(aboutToHide()), this, SLOT(onMenuAboutToHide()));

    KAcceleratorManager::manage(desktopMenu);

    for (auto action : desktopMenu->actions()) {
        if (action->menu()) {
            connect(action->menu(), &QMenu::aboutToShow, desktopMenu, [action, desktopMenu] {
                if (action->menu()->windowHandle()) {
                    // Need to add the transient parent otherwise Qt will create a new toplevel
                    action->menu()->windowHandle()->setTransientParent(desktopMenu->windowHandle());
                }
            });
        }
    }

    //qDebug() << "8 ...";
    desktopMenu->popup(globalPos);
    event->setAccepted(true);
}

//! update the appletContainsPos method from Panel view
void ContextMenuLayerQuickItem::updateAppletContainsMethod()
{
    if (!m_latteView) {
        return;
    }

    for (QQuickItem *item : m_latteView->contentItem()->childItems()) {
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

void ContextMenuLayerQuickItem::addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    desktopMenu->addSection(applet->pluginMetaData().name());

    for (QAction *action : applet->contextualActions()) {
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

        if (appletAlternatives && appletAlternatives->isEnabled() && m_latteView->containment()->isUserConfiguring()) {
            desktopMenu->addAction(appletAlternatives);
        }
    }

    QAction *containmentAction = desktopMenu->menuAction();
    containmentAction->setText(i18nc("%1 is the name of the containment", "%1 Options", m_latteView->containment()->title()));

    if (desktopMenu->actions().count()>1) { /*take into account the Applet Name Section*/
        addContainmentActions(containmentAction->menu(), event);
    }

    if (!containmentAction->menu()->isEmpty()) {
        int enabled = 0;
        //count number of real actions
        QListIterator<QAction *> actionsIt(containmentAction->menu()->actions());

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
            for (QAction *action : containmentAction->menu()->actions()) {
                if (action && action->isVisible()) {
                    desktopMenu->addAction(action);
                }
            }

            // } else {
            //     desktopMenu->addMenu(containmentMenu);
            // }
        }
    }

    if (m_latteView->containment()->immutability() == Plasma::Types::Mutable &&
            (m_latteView->containment()->containmentType() != Plasma::Types::PanelContainment || m_latteView->containment()->isUserConfiguring())) {
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

void ContextMenuLayerQuickItem::addContainmentActions(QMenu *desktopMenu, QEvent *event)
{
    if (!m_latteView || !m_latteView->containment()) {
        return;
    }

    if (m_latteView->containment()->corona()->immutability() != Plasma::Types::Mutable &&
            !KAuthorized::authorizeAction(QStringLiteral("plasma/containment_actions"))) {
        //qDebug() << "immutability";
        return;
    }

    //this is what ContainmentPrivate::prepareContainmentActions was
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    //"RightButton;NoModifier"
    Plasma::ContainmentActions *plugin = m_latteView->containment()->containmentActions().value(trigger);

    if (!plugin) {
        return;
    }

    if (plugin->containment() != m_latteView->containment()) {
        plugin->setContainment(m_latteView->containment());
        // now configure it
        KConfigGroup cfg(m_latteView->containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(m_latteView->containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    QList<QAction *> actions = plugin->contextualActions();

    /*   for (const QAction *act : actions) {
        if (act->menu()) {
            //this is a workaround where Qt now creates the menu widget
            //in .exec before oxygen can polish it and set the following attribute
            act->menu()->setAttribute(Qt::WA_TranslucentBackground);
            //end workaround

            if (act->menu()->winId()) {
                act->menu()->windowHandle()->setTransientParent(m_latteView);
            }
        }
    }*/

    desktopMenu->addActions(actions);
}

Plasma::Containment *ContextMenuLayerQuickItem::containmentById(uint id)
{
    if (!m_latteView) {
        return nullptr;
    }

    for (const auto containment : m_latteView->corona()->containments()) {
        if (id == containment->id()) {
            return containment;
        }
    }

    return nullptr;
}

}
