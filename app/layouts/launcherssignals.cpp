/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
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

#include "launcherssignals.h"

// local
#include <coretypes.h>
#include "../lattecorona.h"
#include "../layout/centrallayout.h"
#include "../layouts/manager.h"
#include "../layouts/synchronizer.h"

// Qt
#include <QQuickItem>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>


namespace Latte {
namespace Layouts {

LaunchersSignals::LaunchersSignals(QObject *parent)
    : QObject(parent)
{
    m_manager = qobject_cast<Layouts::Manager *>(parent);
}

LaunchersSignals::~LaunchersSignals()
{
}

void LaunchersSignals::addAbilityClient(QQuickItem *client)
{
    if (m_clients.contains(client)) {
        return;
    }

    m_clients << client;

    connect(client, &QObject::destroyed, this, &LaunchersSignals::removeClientObject);
}

void LaunchersSignals::removeAbilityClient(QQuickItem *client)
{
    if (!m_clients.contains(client)) {
        return;
    }

    disconnect(client, &QObject::destroyed, this, &LaunchersSignals::removeClientObject);
    m_clients.removeAll(client);
}

void LaunchersSignals::removeClientObject(QObject *obj)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(obj);

    if (item) {
        removeAbilityClient(item);
    }
}

QList<QQuickItem *> LaunchersSignals::clients(QString layoutName)
{
    QList<QQuickItem *> items;

    if (!layoutName.isEmpty()) {
        for(const auto client: m_clients) {
            QString cLayoutName = client->property("layoutName").toString();
            if (cLayoutName == layoutName) {
                items << client;
            }
        }
    } else {
        items = m_clients;
    }

    return items;
}

void LaunchersSignals::addLauncher(QString layoutName, int launcherGroup, QString launcher)
{
    Types::LaunchersGroup group = static_cast<Types::LaunchersGroup>(launcherGroup);

    if ((Types::LaunchersGroup)group == Types::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Types::LayoutLaunchers) ? layoutName : "";

    for(const auto client : clients(lName)) {
        if (auto *metaObject = client->metaObject()) {
            int methodIndex = metaObject->indexOfMethod("addSyncedLauncher(QVariant,QVariant)");

            if (methodIndex == -1) {
                qDebug() << "Launchers Syncer Ability: addSyncedLauncher(QVariant,QVariant) was NOT found...";
                continue;
            }

            QMetaMethod method = metaObject->method(methodIndex);
            method.invoke(client, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher));
        }
    }
}

void LaunchersSignals::removeLauncher(QString layoutName, int launcherGroup, QString launcher)
{
    Types::LaunchersGroup group = static_cast<Types::LaunchersGroup>(launcherGroup);

    if ((Types::LaunchersGroup)group == Types::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Types::LayoutLaunchers) ? layoutName : "";

    for(const auto client : clients(lName)) {
        if (auto *metaObject = client->metaObject()) {
            int methodIndex = metaObject->indexOfMethod("removeSyncedLauncher(QVariant,QVariant)");

            if (methodIndex == -1) {
                qDebug() << "Launchers Syncer Ability: removeSyncedLauncher(QVariant,QVariant) was NOT found...";
                continue;
            }

            QMetaMethod method = metaObject->method(methodIndex);
            method.invoke(client, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher));
        }
    }
}

void LaunchersSignals::addLauncherToActivity(QString layoutName, int launcherGroup, QString launcher, QString activity)
{
    Types::LaunchersGroup group = static_cast<Types::LaunchersGroup>(launcherGroup);

    if ((Types::LaunchersGroup)group == Types::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Types::LayoutLaunchers) ? layoutName : "";

    for(const auto client : clients(lName)) {
        if (auto *metaObject = client->metaObject()) {
            int methodIndex = metaObject->indexOfMethod("addSyncedLauncherToActivity(QVariant,QVariant,QVariant)");

            if (methodIndex == -1) {
                qDebug() << "Launchers Syncer Ability: addSyncedLauncherToActivity(QVariant,QVariant,QVariant) was NOT found...";
                continue;
            }

            QMetaMethod method = metaObject->method(methodIndex);
            method.invoke(client, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher), Q_ARG(QVariant, activity));
        }
    }
}

void LaunchersSignals::removeLauncherFromActivity(QString layoutName, int launcherGroup, QString launcher, QString activity)
{
    Types::LaunchersGroup group = static_cast<Types::LaunchersGroup>(launcherGroup);

    if ((Types::LaunchersGroup)group == Types::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Types::LayoutLaunchers) ? layoutName : "";

    for(const auto client : clients(lName)) {
        if (auto *metaObject = client->metaObject()) {
            int methodIndex = metaObject->indexOfMethod("removeSyncedLauncherFromActivity(QVariant,QVariant,QVariant)");

            if (methodIndex == -1) {
                qDebug() << "Launchers Syncer Ability: removeSyncedLauncherFromActivity(QVariant,QVariant,QVariant) was NOT found...";
                continue;
            }

            QMetaMethod method = metaObject->method(methodIndex);
            method.invoke(client, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher), Q_ARG(QVariant, activity));
        }
    }
}

void LaunchersSignals::urlsDropped(QString layoutName, int launcherGroup, QStringList urls)
{
    Types::LaunchersGroup group = static_cast<Types::LaunchersGroup>(launcherGroup);

    if ((Types::LaunchersGroup)group == Types::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Types::LayoutLaunchers) ? layoutName : "";

    for(const auto client : clients(lName)) {
        if (auto *metaObject = client->metaObject()) {
            int methodIndex = metaObject->indexOfMethod("dropSyncedUrls(QVariant,QVariant)");

            if (methodIndex == -1) {
                qDebug() << "Launchers Syncer Ability: dropSyncedUrls(QVariant,QVariant) was NOT found...";
                continue;
            }

            QMetaMethod method = metaObject->method(methodIndex);
            method.invoke(client, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, urls));
        }
    }
}

void LaunchersSignals::validateLaunchersOrder(QString layoutName, uint senderId, int launcherGroup, QStringList launchers)
{
    Types::LaunchersGroup group = static_cast<Types::LaunchersGroup>(launcherGroup);

    if ((Types::LaunchersGroup)group == Types::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Types::LayoutLaunchers) ? layoutName : "";

    for(const auto client : clients(lName)) {
        uint clientId = client->property("clientId").toUInt();

        if (clientId != senderId) {
            if (auto *metaObject = client->metaObject()) {
                int methodIndex = metaObject->indexOfMethod("validateSyncedLaunchersOrder(QVariant,QVariant)");

                if (methodIndex == -1) {
                    qDebug() << "Launchers Syncer Ability: validateSyncedLaunchersOrder(QVariant,QVariant) was NOT found...";
                    continue;
                }

                QMetaMethod method = metaObject->method(methodIndex);
                method.invoke(client, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launchers));
            }
        }
    }
}

}
} //end of namespace
