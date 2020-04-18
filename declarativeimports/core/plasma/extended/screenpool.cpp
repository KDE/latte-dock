/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "screenpool.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QScreen>

// KDE
#include <KConfigGroup>
#include <KDirWatch>
#include <KSharedConfig>

#define PLASMARC "plasmashellrc"

namespace Latte {
namespace PlasmaExtended {

ScreenPool::ScreenPool(QObject *parent)
    : QObject(parent)
{
    KSharedConfigPtr plasmaPtr = KSharedConfig::openConfig(PLASMARC);
    m_screensGroup = KConfigGroup(plasmaPtr, "ScreenConnectors");

    load();

    QString plasmaSettingsFile = QDir::homePath() + "/.config/" + PLASMARC;

    KDirWatch::self()->addFile(plasmaSettingsFile);

    connect(KDirWatch::self(), &KDirWatch::dirty, this, [ &, plasmaSettingsFile](const QString & path) {
        if (path == plasmaSettingsFile) {
            load();
        }
    });

    connect(KDirWatch::self(), &KDirWatch::created, this, [ &, plasmaSettingsFile](const QString & path) {
        if (path == plasmaSettingsFile) {
            load();
        }
    });
}


ScreenPool::~ScreenPool()
{
}

void ScreenPool::load()
{
    QMap<int, QString> connectorForId = m_connectorForId;
    QHash<QString, int> idForConnector = m_idForConnector;

    m_connectorForId.clear();
    m_idForConnector.clear();

    bool updated{false};

    for (const auto &screenId : m_screensGroup.keyList()) {
        QString screenName =  m_screensGroup.readEntry(screenId, QString());
        if (screenId != 0) {
            int scrId = screenId.toInt();
            insertScreenMapping(scrId, screenName);

            if (!connectorForId.contains(scrId) || connectorForId[scrId] != m_connectorForId[scrId]) {
                updated = true;
            }
        }
    }

    //! If there are changes then print the new plasma screen ids and send a relevant signal
    if (connectorForId.count() != m_connectorForId.count()) {
        updated = true;
    }

    if (updated) {
        qDebug() << "---------------- Plasma Screen Ids ------------------";
        for (const auto &id : m_connectorForId.keys()) {
            qDebug() << id << "  __  " << m_connectorForId[id];
        }
        qDebug() << "----------------  ---------------  ------------------";

        emit idsChanged();
    }
}

void ScreenPool::insertScreenMapping(int id, const QString &connector)
{
    if (id==0 || connector.startsWith(":")) {
        return;
    }

    m_connectorForId[id] = connector;
    m_idForConnector[connector] = id;
}

int ScreenPool::id(const QString &connector) const
{
    if (!m_idForConnector.contains(connector)) {
        //! return 0 for primary screen, -1 for not found
        return qGuiApp->primaryScreen()->name() == connector ? 0 : -1;
    }

    return m_idForConnector.value(connector);
}

QString ScreenPool::connector(int id) const
{
    if (!m_connectorForId.contains(id)) {
        return id == 0 ? qGuiApp->primaryScreen()->name() : QString();
    }

    return m_connectorForId.value(id);
}


}
}
