#include "globalshortcuts.h"

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
        m_hideDockTimer.setInterval(3000);
    }

    connect(&m_hideDockTimer, &QTimer::timeout, this, &GlobalShortcuts::hideDockTimerSlot);
}

GlobalShortcuts::~GlobalShortcuts()
{
}

void GlobalShortcuts::init()
{
    KActionCollection *taskbarActions = new KActionCollection(m_corona);

    //activate actions
    for (int i = 0; i < 10; ++i) {
        const int entryNumber = i + 1;
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_0 + (entryNumber % 10));

        QAction *action = taskbarActions->addAction(QStringLiteral("activate task manager entry %1").arg(QString::number(entryNumber)));
        action->setText(i18n("Activate Task Manager Entry %1", entryNumber));
        action->setShortcut(QKeySequence(Qt::META + key));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + key));
        connect(action, &QAction::triggered, this, [this, i] {
            // qDebug() << "meta action...";
            activateTaskManagerEntry(i, static_cast<Qt::Key>(Qt::META));
        });
    }

    //new instance actions
    for (int i = 0; i < 10; ++i) {
        const int entryNumber = i + 1;
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_0 + (entryNumber % 10));

        QAction *action = taskbarActions->addAction(QStringLiteral("new instance for task manager entry %1").arg(QString::number(entryNumber)));
        action->setText(i18n("New Instance for Task Manager Entry %1", entryNumber));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + Qt::CTRL + key));
        connect(action, &QAction::triggered, this, [this, i] {
            // qDebug() << "meta + ctrl + action...";
            activateTaskManagerEntry(i, static_cast<Qt::Key>(Qt::CTRL));
        });
    }

    //show-hide the main dock in the primary screen
    QAction *showAction = taskbarActions->addAction(QStringLiteral("show latte dock"));
    showAction->setText(i18n("Show Latte Dock"));
    showAction->setShortcut(QKeySequence(Qt::META + '`'));
    KGlobalAccel::setGlobalShortcut(showAction, QKeySequence(Qt::META + '`'));
    connect(showAction, &QAction::triggered, this, [this]() {
        showDock();
    });

}

//! Activate launcher menu through dbus interface
void GlobalShortcuts::activateLauncherMenu()
{
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        const auto applets = it.key()->applets();

        for (auto applet : applets) {
            const auto provides = applet->kPackage().metadata().value(QStringLiteral("X-Plasma-Provides"));

            if (provides.contains(QLatin1String("org.kde.plasma.launchermenu"))) {
                emit applet->activated();
                return;
            }
        }
    }
}


//! Activate task manager entry
void GlobalShortcuts::activateTaskManagerEntry(int index, Qt::Key modifier)
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());

    auto activateTaskManagerEntryOnContainment = [this](const Plasma::Containment * c, int index, Qt::Key modifier) {
        const auto &applets = c->applets();

        for (auto *applet : applets) {
            const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

            if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
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
                            int methodIndex = modifier == static_cast<Qt::Key>(Qt::META) ?
                                              metaObject->indexOfMethod("activateTaskAtIndex(QVariant)") :
                                              metaObject->indexOfMethod("newInstanceForTaskAtIndex(QVariant)");

                            int methodIndex2 = metaObject->indexOfMethod("setShowTasksNumbers(QVariant)");

                            if (methodIndex == -1 || methodIndex2 == -1) {
                                continue;
                            }

                            m_tasksPlasmoid = item;
                            m_tasksMethodIndex = methodIndex2;
                            m_methodShowNumbers = metaObject->method(m_tasksMethodIndex);

                            QMetaMethod method = metaObject->method(methodIndex);

                            if (method.invoke(item, Q_ARG(QVariant, index))
                                && m_methodShowNumbers.invoke(item, Q_ARG(QVariant, true))) {
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    };

    // To avoid overly complex configuration, we'll try to get the 90% usecase to work
    // which is activating a task on the task manager on a panel on the primary screen.
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        if (it.value()->screen() != qGuiApp->primaryScreen()) {
            continue;
        }

        if (activateTaskManagerEntryOnContainment(it.key(), index, modifier)) {
            m_hideDock = it.value();
            m_hideDock->visibility()->setBlockHiding(true);
            m_hideDockTimer.start();
            return;
        }
    }

    // we didn't find anything on primary, try all the panels
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        if (activateTaskManagerEntryOnContainment(it.key(), index, modifier)) {
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

    // update badges in all Latte Tasks plasmoids
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        updateBadgeForTaskInContainment(it.key(), identifier, value);
    }
}

void GlobalShortcuts::showDock()
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());

    auto containsLattePlasmoid = [this](const Plasma::Containment * c) {
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
                            int methodIndex2 = metaObject->indexOfMethod("setShowTasksNumbers(QVariant)");

                            if (methodIndex2 == -1) {
                                continue;
                            }

                            m_tasksPlasmoid = item;
                            m_tasksMethodIndex = methodIndex2;
                            m_methodShowNumbers = metaObject->method(m_tasksMethodIndex);

                            if (m_methodShowNumbers.invoke(item, Q_ARG(QVariant, true))) {
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return false;
    };

    // To avoid overly complex configuration, we'll try to get the 90% usecase to work
    // which is activating a task on the task manager on a panel on the primary screen.
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        if (it.value()->screen() != qGuiApp->primaryScreen()) {
            continue;
        }

        if (containsLattePlasmoid(it.key())) {
            m_hideDock = it.value();
            m_hideDock->visibility()->setBlockHiding(true);
            m_hideDockTimer.start();
            return;
        }
    }

    // we didn't find anything on primary, try all the panels
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        if (containsLattePlasmoid(it.key())) {
            m_hideDock = it.value();
            m_hideDock->visibility()->setBlockHiding(true);
            m_hideDockTimer.start();
            return;
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

            if (m_tasksPlasmoid) {
                m_methodShowNumbers.invoke(m_tasksPlasmoid, Q_ARG(QVariant, false));
                m_tasksPlasmoid = Q_NULLPTR;
                m_tasksMethodIndex = -1;
            }

            return;
        }

        m_hideDockTimer.start();
    } else {
        // TODO: This is needs to be fixed in wayland
        m_lastInvokedAction = Q_NULLPTR;
        m_hideDock->visibility()->setBlockHiding(false);
        m_hideDock = Q_NULLPTR;
    }

}


}

#include "moc_globalshortcuts.cpp"
