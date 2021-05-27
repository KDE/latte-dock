/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYNCEDLAUNCHERS_H
#define SYNCEDLAUNCHERS_H

// Qt
#include <QList>
#include <QObject>
#include <QQuickItem>

namespace Plasma {
class Applet;
}

namespace Latte {
namespace Layouts {
class Manager;
}
}

namespace Latte {
namespace Layouts {

//! in order to support property the launcher groups Layout and Global
//! the latte plasmoids must communicate between them with signals when
//! there are changes in their models. This way we are trying to avoid
//! crashes that occur by setting the launcherList of the tasksModel so
//! often. The plasma devs of libtaskmanager have designed the launchers
//! model to be initialized only once during startup
class SyncedLaunchers : public QObject
{
    Q_OBJECT

public:
    SyncedLaunchers(QObject *parent);
    ~SyncedLaunchers() override;

public slots:
    Q_INVOKABLE void addAbilityClient(QQuickItem *client);
    Q_INVOKABLE void removeAbilityClient(QQuickItem *client);

    Q_INVOKABLE void addLauncher(QString layoutName, int launcherGroup, QString launcher);
    Q_INVOKABLE void removeLauncher(QString layoutName, int launcherGroup, QString launcher);
    Q_INVOKABLE void addLauncherToActivity(QString layoutName, int launcherGroup, QString launcher, QString activity);
    Q_INVOKABLE void removeLauncherFromActivity(QString layoutName, int launcherGroup, QString launcher, QString activity);
    Q_INVOKABLE void urlsDropped(QString layoutName, int launcherGroup, QStringList urls);
    Q_INVOKABLE void validateLaunchersOrder(QString layoutName, uint senderId, int launcherGroup, QStringList launchers);

private:
    QList<QQuickItem *> clients(QString layoutName = QString());

private slots:
    void removeClientObject(QObject *obj);

private:
    Layouts::Manager *m_manager{nullptr};

    QList<QQuickItem *> m_clients;
};

}
}

#endif
