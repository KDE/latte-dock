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

#include "dockcorona.h"
#include "globalshortcuts.h"
#include "layoutmanager.h"
#include "dock/dockview.h"

#include <QAction>
#include <QDebug>
#include <QQuickItem>
#include <QMetaMethod>
#include <QX11Info>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginMetaData>

#include <Plasma/Applet>
#include <Plasma/Containment>

// X11
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xlib.h>

#include <array>

//this code is used by activityswitcher in plasma in order to check if the
//user has release all the modifier keys from the globalshortcut
namespace {
bool isPlatformX11()
{
    static const bool isX11 = QX11Info::isPlatformX11();
    return isX11;
}

// Taken from kwin/tabbox/tabbox.cpp
Display *x11_display()
{
    static Display *s_display = nullptr;

    if (!s_display) {
        s_display = QX11Info::display();
    }

    return s_display;
}

bool x11_areKeySymXsDepressed(bool bAll, const uint keySyms[], int nKeySyms)
{
    char keymap[32];

    XQueryKeymap(x11_display(), keymap);

    for (int iKeySym = 0; iKeySym < nKeySyms; iKeySym++) {
        uint keySymX = keySyms[ iKeySym ];
        uchar keyCodeX = XKeysymToKeycode(x11_display(), keySymX);
        int i = keyCodeX / 8;
        char mask = 1 << (keyCodeX - (i * 8));

        // Abort if bad index value,
        if (i < 0 || i >= 32)
            return false;

        // If ALL keys passed need to be depressed,
        if (bAll) {
            if ((keymap[i] & mask) == 0)
                return false;
        } else {
            // If we are looking for ANY key press, and this key is depressed,
            if (keymap[i] & mask)
                return true;
        }
    }

    // If we were looking for ANY key press, then none was found, return false,
    // If we were looking for ALL key presses, then all were found, return true.
    return bAll;
}

bool x11_areModKeysDepressed(const QKeySequence &seq)
{
    uint rgKeySyms[10];
    int nKeySyms = 0;

    if (seq.isEmpty()) {
        return false;
    }

    int mod = seq[seq.count() - 1] & Qt::KeyboardModifierMask;

    if (mod & Qt::SHIFT) {
        rgKeySyms[nKeySyms++] = XK_Shift_L;
        rgKeySyms[nKeySyms++] = XK_Shift_R;
    }

    if (mod & Qt::CTRL) {
        rgKeySyms[nKeySyms++] = XK_Control_L;
        rgKeySyms[nKeySyms++] = XK_Control_R;
    }

    if (mod & Qt::ALT) {
        rgKeySyms[nKeySyms++] = XK_Alt_L;
        rgKeySyms[nKeySyms++] = XK_Alt_R;
    }

    if (mod & Qt::META) {
        // It would take some code to determine whether the Win key
        // is associated with Super or Meta, so check for both.
        // See bug #140023 for details.
        rgKeySyms[nKeySyms++] = XK_Super_L;
        rgKeySyms[nKeySyms++] = XK_Super_R;
        rgKeySyms[nKeySyms++] = XK_Meta_L;
        rgKeySyms[nKeySyms++] = XK_Meta_R;
    }

    return x11_areKeySymXsDepressed(false, rgKeySyms, nKeySyms);
}
}



namespace Latte {

const int APPLETEXECUTIONDELAY = 400;

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        init();
    }

    m_hideDockTimer.setSingleShot(true);

    if (isPlatformX11()) {
        //in X11 the timer is a poller that checks to see if the modifier keys
        //from user global shortcut have been released
        m_hideDockTimer.setInterval(300);
    } else {
        //on wayland in acting just as simple timer that hides the dock afterwards
        m_hideDockTimer.setInterval(2500);
    }

    connect(&m_hideDockTimer, &QTimer::timeout, this, &GlobalShortcuts::hideDockTimerSlot);
}

GlobalShortcuts::~GlobalShortcuts()
{
}

void GlobalShortcuts::init()
{
    KActionCollection *generalActions = new KActionCollection(m_corona);

    //show-hide the main dock in the primary screen
    QAction *showAction = generalActions->addAction(QStringLiteral("show latte dock"));
    showAction->setText(i18n("Show Dock"));
    showAction->setShortcut(QKeySequence(Qt::META + '`'));
    KGlobalAccel::setGlobalShortcut(showAction, QKeySequence(Qt::META + '`'));
    connect(showAction, &QAction::triggered, this, [this]() {
        showDock();
    });

    //show-cycle between Latte settings windows
    QAction *settingsAction = generalActions->addAction(QStringLiteral("show dock settings"));
    settingsAction->setText(i18n("Show Dock Settings"));
    KGlobalAccel::setGlobalShortcut(settingsAction, QKeySequence(Qt::META + Qt::Key_A));
    connect(settingsAction, &QAction::triggered, this, [this] {
        showSettings();
    });

    //show the layouts editor
    QAction *layoutsAction = generalActions->addAction(QStringLiteral("show layout settings"));
    layoutsAction->setText(i18n("Show Layout Settings"));
    layoutsAction->setShortcut(QKeySequence(Qt::META + Qt::Key_W));
    KGlobalAccel::setGlobalShortcut(layoutsAction, QKeySequence(Qt::META + Qt::Key_W));
    connect(layoutsAction, &QAction::triggered, this, [this]() {
        m_corona->layoutManager()->showLatteSettingsDialog(Dock::LayoutPage);
    });

    //show the latter universal settings
    QAction *universalSettingsAction = generalActions->addAction(QStringLiteral("show latte universal settings"));
    universalSettingsAction->setText(i18n("Show Latte Settings"));
    universalSettingsAction->setShortcut(QKeySequence(Qt::META + Qt::Key_E));
    KGlobalAccel::setGlobalShortcut(universalSettingsAction, QKeySequence(Qt::META + Qt::Key_E));
    connect(universalSettingsAction, &QAction::triggered, this, [this]() {
        m_corona->layoutManager()->showLatteSettingsDialog(Dock::PreferencesPage);
    });

    KActionCollection *taskbarActions = new KActionCollection(m_corona);

    //activate actions [1-9]
    for (int i = 1; i < 10; ++i) {
        const int entryNumber = i;
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_0 + i);

        QAction *action = taskbarActions->addAction(QStringLiteral("activate entry %1").arg(QString::number(entryNumber)));
        action->setText(i18n("Activate Entry %1", entryNumber));
        action->setShortcut(QKeySequence(Qt::META + key));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + key));
        connect(action, &QAction::triggered, this, [this, i] {
            // qDebug() << "meta action...";
            activateEntry(i, static_cast<Qt::Key>(Qt::META));
        });
    }

    //! Array that is used to register correctly actions for task index>=10 and <19
    std::array<Qt::Key, 10> keysAboveTen{  Qt::Key_0,  Qt::Key_Z,  Qt::Key_X, Qt::Key_C, Qt::Key_V, Qt::Key_B, Qt::Key_N, Qt::Key_M, Qt::Key_Comma, Qt::Key_Period };

    //activate actions [10-19]
    for (int i = 10; i < 20; ++i) {
        QAction *action = taskbarActions->addAction(QStringLiteral("activate entry %1").arg(QString::number(i)));
        action->setText(i18n("Activate Entry %1", i));
        action->setShortcut(QKeySequence(Qt::META + keysAboveTen[i - 10]));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + keysAboveTen[i - 10]));
        connect(action, &QAction::triggered, this, [this, i] {
            activateEntry(i, static_cast<Qt::Key>(Qt::META));
        });
    }

    //new instance actions [1-9]
    for (int i = 1; i < 9; ++i) {
        const int entryNumber = i;
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_0 + i);

        QAction *action = taskbarActions->addAction(QStringLiteral("new instance for entry %1").arg(QString::number(entryNumber)));
        action->setText(i18n("New Instance for Entry %1", entryNumber));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + Qt::CTRL + key));
        connect(action, &QAction::triggered, this, [this, i] {
            // qDebug() << "meta + ctrl + action...";
            activateEntry(i, static_cast<Qt::Key>(Qt::CTRL));
        });
    }

    //new instance actions [10-19]
    for (int i = 10; i < 20; ++i) {
        QAction *action = taskbarActions->addAction(QStringLiteral("new instance for entry %1").arg(QString::number(i)));
        action->setText(i18n("New Instance for Entry %1", i));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + Qt::CTRL + keysAboveTen[i - 10]));
        connect(action, &QAction::triggered, this, [this, i] {
            activateEntry(i, static_cast<Qt::Key>(Qt::CTRL));
        });
    }

    m_singleMetaAction = new QAction(this);
    m_singleMetaAction->setShortcut(QKeySequence(Qt::META));
}

//! Activate launcher menu through dbus interface
void GlobalShortcuts::activateLauncherMenu()
{
    QHash<const Plasma::Containment *, DockView *> *views = m_corona->layoutManager()->currentDockViews();

    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        const auto applets = it.key()->applets();

        for (auto applet : applets) {
            const auto provides = applet->kPackage().metadata().value(QStringLiteral("X-Plasma-Provides"));

            if (provides.contains(QLatin1String("org.kde.plasma.launchermenu"))) {
                if (it.value()->visibility()->isHidden()) {
                    m_lastInvokedAction = m_singleMetaAction;
                    m_hideDock = it.value();
                    m_hideDock->visibility()->setBlockHiding(true);
                    m_hideDockTimer.start();

                    //! delay the execution in order to show first the dock
                    QTimer::singleShot(APPLETEXECUTIONDELAY, [this, it, applet]() {
                        it.value()->toggleAppletExpanded(applet->id());
                    });
                } else {
                    it.value()->toggleAppletExpanded(applet->id());
                }

                return;
            }
        }
    }
}

bool GlobalShortcuts::activatePlasmaTaskManagerEntryAtContainment(const Plasma::Containment *c, int index, Qt::Key modifier)
{
    const auto &applets = c->applets();

    for (auto *applet : applets) {
        const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

        if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
            if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
                const auto &childItems = appletInterface->childItems();

                if (childItems.isEmpty()) {
                    continue;
                }

                KPluginMetaData meta = applet->kPackage().metadata();

                for (QQuickItem *item : childItems) {
                    if (auto *metaObject = item->metaObject()) {
                        // not using QMetaObject::invokeMethod to avoid warnings when calling
                        // this on applets that don't have it or other child items since this
                        // is pretty much trial and error.

                        // Also, "var" arguments are treated as QVariant in QMetaObject
                        int methodIndex = modifier == static_cast<Qt::Key>(Qt::META) ?
                                          metaObject->indexOfMethod("activateTaskAtIndex(QVariant)") :
                                          metaObject->indexOfMethod("newInstanceForTaskAtIndex(QVariant)");

                        int methodIndex2 = metaObject->indexOfMethod("setShowTasksNumbers(QVariant)");

                        if (methodIndex == -1 || (methodIndex2 == -1 && meta.pluginId() == "org.kde.latte.plasmoid")) {
                            continue;
                        }

                        m_calledItem = item;
                        m_numbersMethodIndex = methodIndex;
                        m_methodShowNumbers = metaObject->method(m_numbersMethodIndex);

                        QMetaMethod method = metaObject->method(methodIndex);

                        if (method.invoke(item, Q_ARG(QVariant, index - 1))) {
                            if (methodIndex2 != -1) {
                                m_methodShowNumbers.invoke(item, Q_ARG(QVariant, true));
                            }

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool GlobalShortcuts::activateLatteEntryAtContainment(const DockView *view, int index, Qt::Key modifier)
{
    if (QQuickItem *containmentInterface = view->containment()->property("_plasma_graphicObject").value<QQuickItem *>()) {
        const auto &childItems = containmentInterface->childItems();

        for (QQuickItem *item : childItems) {
            if (auto *metaObject = item->metaObject()) {
                // not using QMetaObject::invokeMethod to avoid warnings when calling
                // this on applets that don't have it or other child items since this
                // is pretty much trial and error.

                // Also, "var" arguments are treated as QVariant in QMetaObject
                int methodIndex = modifier == static_cast<Qt::Key>(Qt::META) ?
                                  metaObject->indexOfMethod("activateEntryAtIndex(QVariant)") :
                                  metaObject->indexOfMethod("newInstanceForEntryAtIndex(QVariant)");

                int methodIndex2 = metaObject->indexOfMethod("setShowAppletsNumbers(QVariant)");

                if (methodIndex == -1 || (methodIndex2 == -1)) {
                    continue;
                }

                m_calledItem = item;
                m_numbersMethodIndex = methodIndex2;
                m_methodShowNumbers = metaObject->method(m_numbersMethodIndex);

                QMetaMethod method = metaObject->method(methodIndex);

                if (view->visibility()->isHidden()) {
                    //! delay the execution in order to show first the dock
                    if (m_methodShowNumbers.invoke(item, Q_ARG(QVariant, true))) {
                        QTimer::singleShot(APPLETEXECUTIONDELAY, [this, item, method, index]() {
                            method.invoke(item, Q_ARG(QVariant, index));
                        });
                    }

                    return true;
                } else {
                    if (method.invoke(item, Q_ARG(QVariant, index))) {
                        m_methodShowNumbers.invoke(item, Q_ARG(QVariant, true));

                        return true;
                    }
                }
            }
        }
    }

    return false;
}


//! Activate task manager entry
void GlobalShortcuts::activateEntry(int index, Qt::Key modifier)
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());

    QHash<const Plasma::Containment *, DockView *> *views =  m_corona->layoutManager()->currentDockViews();

    // To avoid overly complex configuration, we'll try to get the 90% usecase to work
    // which is activating a task on the task manager on a panel on the primary screen.
    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        if (it.value()->screen() != qGuiApp->primaryScreen()) {
            continue;
        }

        if ((it.value()->latteTasksPresent() && activateLatteEntryAtContainment(it.value(), index, modifier))
            || (!it.value()->latteTasksPresent() && it.value()->tasksPresent() &&
                activatePlasmaTaskManagerEntryAtContainment(it.key(), index, modifier))) {
            m_hideDock = it.value();
            m_hideDock->visibility()->setBlockHiding(true);
            m_hideDockTimer.start();
            return;
        }
    }

    // we didn't find anything on primary, try all the panels
    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        if ((it.value()->latteTasksPresent() && activateLatteEntryAtContainment(it.value(), index, modifier))
            || (!it.value()->latteTasksPresent() && it.value()->tasksPresent() &&
                activatePlasmaTaskManagerEntryAtContainment(it.key(), index, modifier))) {
            m_hideDock = it.value();
            m_hideDock->visibility()->setBlockHiding(true);
            m_hideDockTimer.start();
            return;
        }
    }
}

//! update badge for specific dock item
void GlobalShortcuts::updateDockItemBadge(QString identifier, QString value)
{
    //qDebug() << "DBUS CALL ::: " << identifier << " - " << value;
    auto updateBadgeForTaskInContainment = [this](const Plasma::Containment * c, QString identifier, QString value) {
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

                            int methodIndex = metaObject->indexOfMethod("updateBadge(QVariant,QVariant)");

                            if (methodIndex == -1) {
                                continue;
                            }

                            QMetaMethod method = metaObject->method(methodIndex);

                            if (method.invoke(item, Q_ARG(QVariant, identifier), Q_ARG(QVariant, value))) {
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    };

    QHash<const Plasma::Containment *, DockView *> *views =  m_corona->layoutManager()->currentDockViews();

    // update badges in all Latte Tasks plasmoids
    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        updateBadgeForTaskInContainment(it.key(), identifier, value);
    }
}

void GlobalShortcuts::showDock()
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());

    auto invokeShowNumbers = [this](const Plasma::Containment * c) {
        if (QQuickItem *containmentInterface = c->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = containmentInterface->childItems();

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    // not using QMetaObject::invokeMethod to avoid warnings when calling
                    // this on applets that don't have it or other child items since this
                    // is pretty much trial and error.

                    // Also, "var" arguments are treated as QVariant in QMetaObject
                    int methodIndex = metaObject->indexOfMethod("setShowAppletsNumbers(QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    m_calledItem = item;
                    m_numbersMethodIndex = methodIndex;
                    m_methodShowNumbers = metaObject->method(m_numbersMethodIndex);

                    if (m_methodShowNumbers.invoke(item, Q_ARG(QVariant, true))) {
                        return true;
                    }
                }
            }
        }

        return false;
    };

    QHash<const Plasma::Containment *, DockView *> *views =  m_corona->layoutManager()->currentDockViews();

    // To avoid overly complex configuration, we'll try to get the 90% usecase to work
    // which is activating a task on the task manager on a panel on the primary screen.
    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        if (it.value()->screen() != qGuiApp->primaryScreen()) {
            continue;
        }

        if (it.value()->latteTasksPresent() && invokeShowNumbers(it.key())) {
            if (!m_hideDockTimer.isActive()) {
                m_hideDock = it.value();
                m_hideDock->visibility()->setBlockHiding(true);
                m_hideDockTimer.start();
            } else {
                m_hideDockTimer.stop();
                hideDockTimerSlot();
            }

            return;
        }
    }

    // we didn't find anything on primary, try all the panels
    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        if (it.value()->latteTasksPresent() && invokeShowNumbers(it.key())) {
            if (!m_hideDockTimer.isActive()) {
                m_hideDock = it.value();
                m_hideDock->visibility()->setBlockHiding(true);
                m_hideDockTimer.start();
            } else {
                m_hideDockTimer.stop();
                hideDockTimerSlot();
            }

            return;
        }
    }
}

bool GlobalShortcuts::dockAtLowerScreenPriority(DockView *test, DockView *base)
{
    if (!base || ! test) {
        return true;
    }

    if (base->screen() == test->screen()) {
        return false;
    } else if (base->screen() != qGuiApp->primaryScreen() && test->screen() == qGuiApp->primaryScreen()) {
        return false;
    } else if (base->screen() == qGuiApp->primaryScreen() && test->screen() != qGuiApp->primaryScreen()) {
        return true;
    } else {
        int basePriority = -1;
        int testPriority = -1;

        for (int i = 0; i < qGuiApp->screens().count(); ++i) {
            if (base->screen() == qGuiApp->screens()[i]) {
                basePriority = i;
            }

            if (test->screen() == qGuiApp->screens()[i]) {
                testPriority = i;
            }
        }

        if (testPriority <= basePriority) {
            return true;
        } else {
            return false;
        }

    }

    qDebug() << "dockAtLowerScreenPriority : shouldnt had reached here...";
    return false;
}

bool GlobalShortcuts::dockAtLowerEdgePriority(DockView *test, DockView *base)
{
    if (!base || ! test) {
        return true;
    }

    QList<Plasma::Types::Location> edges{Plasma::Types::RightEdge, Plasma::Types::TopEdge,
                                         Plasma::Types::LeftEdge, Plasma::Types::BottomEdge};

    int testPriority = -1;
    int basePriority = -1;

    for (int i = 0; i < edges.count(); ++i) {
        if (edges[i] == base->location()) {
            basePriority = i;
        }

        if (edges[i] == test->location()) {
            testPriority = i;
        }
    }

    if (testPriority < basePriority)
        return true;
    else
        return false;
}


void GlobalShortcuts::showSettings()
{
    QList<DockView *> docks;

    QHash<const Plasma::Containment *, DockView *> *views =  m_corona->layoutManager()->currentDockViews();

    //! create a docks list to sorted out
    for (auto it = views->constBegin(), end = views->constEnd(); it != end; ++it) {
        docks.append(it.value());
    }

    qDebug() << " -------- ";

    for (int i = 0; i < docks.count(); ++i) {
        qDebug() << i << ". " << docks[i]->screen()->name() << " - " << docks[i]->location();
    }

    //! sort the docks based on screens and edges priorities
    //! docks on primary screen have higher priority and
    //! for docks in the same screen the priority goes to
    //! Bottom,Left,Top,Right
    for (int i = 0; i < docks.size(); ++i) {
        for (int j = 0; j < docks.size() - i - 1; ++j) {
            if (dockAtLowerScreenPriority(docks[j], docks[j + 1])
                || (docks[j]->screen() == docks[j + 1]->screen()
                    && dockAtLowerEdgePriority(docks[j], docks[j + 1]))) {
                DockView *temp = docks[j + 1];
                docks[j + 1] = docks[j];
                docks[j] = temp;
            }
        }
    }

    qDebug() << " -------- sorted -----";

    for (int i = 0; i < docks.count(); ++i) {
        qDebug() << i << ". " << docks[i]->screen()->name() << " - " << docks[i]->location();
    }


    //! find which is the next dock to show its settings
    if (docks.count() > 0) {
        int openSettings = -1;

        //! check if there is a dock with opened settings window
        for (int i = 0; i < docks.size(); ++i) {
            if (docks[i]->settingsWindowIsShown()) {
                openSettings = i;
                break;
            }
        }

        if (openSettings >= 0 && docks.count() > 1) {
            openSettings = openSettings + 1;

            if (openSettings >= docks.size()) {
                openSettings = 0;
            }

            docks[openSettings]->showSettingsWindow();
        } else {
            docks[0]->showSettingsWindow();
        }
    }
}

void GlobalShortcuts::hideDockTimerSlot()
{
    if (!m_lastInvokedAction || !m_hideDock) {
        return;
    }

    if (isPlatformX11()) {
        if (!x11_areModKeysDepressed(m_lastInvokedAction->shortcut())) {
            m_lastInvokedAction = Q_NULLPTR;
            m_hideDock->visibility()->setBlockHiding(false);
            m_hideDock = Q_NULLPTR;

            if (m_calledItem) {
                m_methodShowNumbers.invoke(m_calledItem, Q_ARG(QVariant, false));
                m_calledItem = Q_NULLPTR;
                m_numbersMethodIndex = -1;
            }

            return;
        }

        m_hideDockTimer.start();
    } else {
        // TODO: This is needs to be fixed in wayland
        m_lastInvokedAction = Q_NULLPTR;
        m_hideDock->visibility()->setBlockHiding(false);
        m_hideDock = Q_NULLPTR;

        if (m_calledItem) {
            m_methodShowNumbers.invoke(m_calledItem, Q_ARG(QVariant, false));
            m_calledItem = Q_NULLPTR;
            m_numbersMethodIndex = -1;
        }
    }

}

}

#include "moc_globalshortcuts.cpp"
