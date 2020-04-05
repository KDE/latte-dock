/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "contextmenu.h"

// local
#include "view.h"
#include "visibilitymanager.h"
#include "../lattecorona.h"

// Qt
#include <QMouseEvent>
#include <QVersionNumber>

// KDE
#include <KActionCollection>
#include <KAuthorized>
#include <KLocalizedString>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <PlasmaQuick/AppletQuickItem>

#define BLOCKHIDINGTYPE "View::contextMenu()"

namespace Latte {
namespace ViewPart {

ContextMenu::ContextMenu(Latte::View *view) :
    QObject(view),
    m_latteView(view)
{
}

ContextMenu::~ContextMenu()
{
}

QMenu *ContextMenu::menu()
{
    return m_contextMenu;
}

void ContextMenu::menuAboutToHide()
{
    if (!m_latteView) {
        return;
    }

    m_contextMenu = 0;

    emit menuChanged();
}

bool ContextMenu::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "Step -1 ...";

    if (!event || !m_latteView->containment()) {
        return false;
    }

    //qDebug() << "Step 0...";

    //even if the menu is executed synchronously, other events may be processed
    //by the qml incubator when plasma is loading, so we need to guard there
    if (m_contextMenu) {
        //qDebug() << "Step 0.5 ...";
        m_contextMenu->close();
        m_contextMenu = 0;
        emit menuChanged();
        // PlasmaQuick::ContainmentView::mousePressEvent(event);
        return false;
    }

    //qDebug() << "1 ...";
    QString trigger = Plasma::ContainmentActions::eventToString(event);

    if (trigger == "RightButton;NoModifier") {
        Plasma::ContainmentActions *plugin = m_latteView->containment()->containmentActions().value(trigger);

        if (!plugin || plugin->contextualActions().isEmpty()) {
            event->setAccepted(false);
            return false;
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

        for (const Plasma::Applet *appletTemp : m_latteView->containment()->applets()) {
            PlasmaQuick::AppletQuickItem *ai = appletTemp->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            bool appletContainsMouse = false;

            if (m_appletContainsMethod.isValid()) {
                QVariant retVal;
                m_appletContainsMethod.invoke(m_appletContainsMethodItem, Qt::DirectConnection, Q_RETURN_ARG(QVariant, retVal)
                                              , Q_ARG(QVariant, appletTemp->id()), Q_ARG(QVariant, event->pos()));
                appletContainsMouse = retVal.toBool();
            } else {
                appletContainsMouse = ai->contains(ai->mapFromItem(m_latteView->contentItem(), event->pos()));
            }

            if (ai && ai->isVisible() && appletContainsMouse) {
                applet = ai->applet();

                if (m_latteView && m_latteView->layout() && m_latteView->layout()->isInternalContainment(applet)) {
                    Plasma::Containment *internalC = m_latteView->layout()->internalContainmentOf(applet);

                    if (internalC) {
                        Plasma::Applet *internalApplet{nullptr};

                        for (const Plasma::Applet *appletCont : internalC->applets()) {
                            PlasmaQuick::AppletQuickItem *ai2 = appletCont->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

                            if (ai2 && ai2->isVisible() && ai2->contains(ai2->mapFromItem(m_latteView->contentItem(), event->pos()))) {
                                internalApplet = ai2->applet();
                                break;
                            }
                        }

                        if (!internalApplet) {
                            return true;
                        } else {
                            applet = internalApplet;
                        }
                    }

                    break;
                } else {
                    ai = 0;
                }
            }
        }

        if (!applet && !inSystray) {
            applet = m_latteView->containment();
        }

        //qDebug() << "3 ...";

        if (applet) {
            const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

            //qDebug() << "3.5 ...";

            if (!provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
                //qDebug() << "4...";
                QMenu *desktopMenu = new QMenu;

                //this is a workaround where Qt now creates the menu widget
                //in .exec before oxygen can polish it and set the following attribute
                desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
                //end workaround

                if (desktopMenu->winId()) {
                    desktopMenu->windowHandle()->setTransientParent(m_latteView);
                }

                desktopMenu->setAttribute(Qt::WA_DeleteOnClose);
                m_contextMenu = desktopMenu;

                //! deprecated old code that can be removed if the following plasma approach doesn't
                //! create any issues with context menu creation in Latte
                /*if (m_latteView->mouseGrabberItem()) {
                    //workaround, this fixes for me most of the right click menu behavior
                    m_latteView->mouseGrabberItem()->ungrabMouse();
                    return;
                }*/

                //!plasma official code
                //this is a workaround where Qt will fail to realize a mouse has been released

                // this happens if a window which does not accept focus spawns a new window that takes focus and X grab
                // whilst the mouse is depressed
                // https://bugreports.qt.io/browse/QTBUG-59044
                // this causes the next click to go missing

                //by releasing manually we avoid that situation
                auto ungrabMouseHack = [this]() {
                    if (m_latteView->mouseGrabberItem()) {
                        m_latteView->mouseGrabberItem()->ungrabMouse();
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

                if (applet && applet != m_latteView->containment()) {
                    //qDebug() << "5.3 ...";
                    emit applet->contextualActionsAboutToShow();
                    addAppletActions(desktopMenu, applet, event);
                } else {
                    //qDebug() << "5.6 ...";
                    emit m_latteView->containment()->contextualActionsAboutToShow();
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

                    if (m_latteView->screen()) {
                        const QRect scr = m_latteView->screen()->geometry();
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
                    return false;
                }

                connect(desktopMenu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));

                desktopMenu->popup(pos);
                event->setAccepted(true);
                emit menuChanged();
                return false;
            }

            //qDebug() << "8 ...";
        }

        //qDebug() << "9 ...";
    }

    //qDebug() << "10 ...";
    emit menuChanged();
    return true;
    //  PlasmaQuick::ContainmentView::mousePressEvent(event);
}

//! update the appletContainsPos method from Panel view
void ContextMenu::updateAppletContainsMethod()
{
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

void ContextMenu::addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    if (!m_latteView->containment()) {
        return;
    }

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

    addContainmentActions(containmentAction->menu(), event);

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

void ContextMenu::addContainmentActions(QMenu *desktopMenu, QEvent *event)
{
    if (!m_latteView->containment()) {
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

    for (const QAction *act : actions) {
        if (act->menu()) {
            //this is a workaround where Qt now creates the menu widget
            //in .exec before oxygen can polish it and set the following attribute
            act->menu()->setAttribute(Qt::WA_TranslucentBackground);
            //end workaround

            if (act->menu()->winId()) {
                act->menu()->windowHandle()->setTransientParent(m_latteView);
            }
        }
    }

    desktopMenu->addActions(actions);

    return;
}

Plasma::Containment *ContextMenu::containmentById(uint id)
{
    for (const auto containment : m_latteView->corona()->containments()) {
        if (id == containment->id()) {
            return containment;
        }
    }

    return 0;
}

}
}
