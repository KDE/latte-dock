/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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
