/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "screenpool.h"

// local
#include "../../primaryoutputwatcher.h"
#include "../../tools/commontools.h"

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
    : QObject(parent),
      m_primaryWatcher(new PrimaryOutputWatcher(this))
{
    m_plasmarcConfig = KSharedConfig::openConfig(PLASMARC);
    m_screensGroup = KConfigGroup(m_plasmarcConfig, "ScreenConnectors");

    load();

    QString plasmaSettingsFile = Latte::configPath() + "/" + PLASMARC;

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

    m_plasmarcConfig->reparseConfiguration();

    bool updated{false};

    for (const auto &screenId : m_screensGroup.keyList()) {
        QString screenName =  m_screensGroup.readEntry(screenId, QString());
        int scrId = screenId.toInt();
        if (scrId != 0) {
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
        return m_primaryWatcher->primaryScreen()->name() == connector ? 0 : -1;
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
