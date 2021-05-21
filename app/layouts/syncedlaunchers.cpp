/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "syncedlaunchers.h"

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

SyncedLaunchers::SyncedLaunchers(QObject *parent)
    : QObject(parent)
{
    m_manager = qobject_cast<Layouts::Manager *>(parent);
}

SyncedLaunchers::~SyncedLaunchers()
{
}

void SyncedLaunchers::addAbilityClient(QQuickItem *client)
{
    if (m_clients.contains(client)) {
        return;
    }

    m_clients << client;

    connect(client, &QObject::destroyed, this, &SyncedLaunchers::removeClientObject);
}

void SyncedLaunchers::removeAbilityClient(QQuickItem *client)
{
    if (!m_clients.contains(client)) {
        return;
    }

    disconnect(client, &QObject::destroyed, this, &SyncedLaunchers::removeClientObject);
    m_clients.removeAll(client);
}

void SyncedLaunchers::removeClientObject(QObject *obj)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(obj);

    if (item) {
        removeAbilityClient(item);
    }
}

QList<QQuickItem *> SyncedLaunchers::clients(QString layoutName)
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

void SyncedLaunchers::addLauncher(QString layoutName, int launcherGroup, QString launcher)
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

void SyncedLaunchers::removeLauncher(QString layoutName, int launcherGroup, QString launcher)
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

void SyncedLaunchers::addLauncherToActivity(QString layoutName, int launcherGroup, QString launcher, QString activity)
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

void SyncedLaunchers::removeLauncherFromActivity(QString layoutName, int launcherGroup, QString launcher, QString activity)
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

void SyncedLaunchers::urlsDropped(QString layoutName, int launcherGroup, QStringList urls)
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

void SyncedLaunchers::validateLaunchersOrder(QString layoutName, uint senderId, int launcherGroup, QStringList launchers)
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
