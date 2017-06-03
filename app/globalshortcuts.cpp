#include "globalshortcuts.h"

#include <QAction>
#include <QDebug>
#include <QQuickItem>
#include <QMetaMethod>

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginMetaData>

#include <Plasma/Applet>
#include <Plasma/Containment>


namespace Latte {

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        init();
    }
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

                            if (methodIndex == -1) {
                                continue;
                            }

                            QMetaMethod method = metaObject->method(methodIndex);

                            if (method.invoke(item, Q_ARG(QVariant, index))) {
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
            return;
        }
    }

    // we didn't find anything on primary, try all the panels
    for (auto it = m_corona->m_dockViews.constBegin(), end = m_corona->m_dockViews.constEnd(); it != end; ++it) {
        if (activateTaskManagerEntryOnContainment(it.key(), index, modifier)) {
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


}

#include "moc_globalshortcuts.cpp"
