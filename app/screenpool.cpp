/*
 *  Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "screenpool.h"

#include <QGuiApplication>
#include <QScreen>

ScreenPool::ScreenPool(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent),
      m_configGroup(KConfigGroup(config, QStringLiteral("ScreenConnectors")))
{
    m_configSaveTimer.setSingleShot(true);
    connect(&m_configSaveTimer, &QTimer::timeout, this, [this]() {
        m_configGroup.sync();
    });
}

void ScreenPool::load()
{
    m_primaryConnector = QString();
    m_connectorForId.clear();
    m_idForConnector.clear();

    QScreen *primary = qGuiApp->primaryScreen();

    if (primary) {
        m_primaryConnector = primary->name();

        if (!m_primaryConnector.isEmpty()) {
            m_connectorForId[0] = m_primaryConnector;
            m_idForConnector[m_primaryConnector] = 0;
        }
    }

    //restore the known ids to connector mappings
    foreach (const QString &key, m_configGroup.keyList()) {
        QString connector = m_configGroup.readEntry(key, QString());

        if (!key.isEmpty() && !connector.isEmpty() &&
            !m_connectorForId.contains(key.toInt()) &&
            !m_idForConnector.contains(connector)) {
            m_connectorForId[key.toInt()] = connector;
            m_idForConnector[connector] = key.toInt();
        } else if (m_idForConnector.value(connector) != key.toInt()) {
            m_configGroup.deleteEntry(key);
        }
    }

    // if there are already connected unknown screens, map those
    // all needs to be populated as soon as possible, otherwise
    // containment->screen() will return an incorrect -1
    // at startup, if it' asked before corona::addOutput()
    // is performed, driving to the creation of a new containment
    for (QScreen *screen : qGuiApp->screens()) {
        if (!m_idForConnector.contains(screen->name())) {
            insertScreenMapping(firstAvailableId(), screen->name());
        }
    }
}

ScreenPool::~ScreenPool()
{
    m_configGroup.sync();
}

QString ScreenPool::primaryConnector() const
{
    return m_primaryConnector;
}

void ScreenPool::setPrimaryConnector(const QString &primary)
{
    if (m_primaryConnector == primary) {
        return;
    }

    Q_ASSERT(m_idForConnector.contains(primary));

    int oldIdForPrimary = m_idForConnector.value(primary);

    m_idForConnector[primary] = 0;
    m_connectorForId[0] = primary;
    m_idForConnector[m_primaryConnector] = oldIdForPrimary;
    m_connectorForId[oldIdForPrimary] = m_primaryConnector;
    m_primaryConnector = primary;
    save();
}

void ScreenPool::save()
{
    QMap<int, QString>::const_iterator i;

    for (i = m_connectorForId.constBegin(); i != m_connectorForId.constEnd(); ++i) {
        m_configGroup.writeEntry(QString::number(i.key()), i.value());
    }

    //write to disck every 30 seconds at most
    m_configSaveTimer.start(30000);
}

void ScreenPool::insertScreenMapping(int id, const QString &connector)
{
    Q_ASSERT(!m_connectorForId.contains(id) || m_connectorForId.value(id) == connector);
    Q_ASSERT(!m_idForConnector.contains(connector) || m_idForConnector.value(connector) == id);

    if (id == 0) {
        m_primaryConnector = connector;
    }

    m_connectorForId[id] = connector;
    m_idForConnector[connector] = id;
    save();
}

int ScreenPool::id(const QString &connector) const
{
    if (!m_idForConnector.contains(connector)) {
        return -1;
    }

    return m_idForConnector.value(connector);
}

QString ScreenPool::connector(int id) const
{
    Q_ASSERT(m_connectorForId.contains(id));

    return m_connectorForId.value(id);
}

int ScreenPool::firstAvailableId() const
{
    int i = 0;

    //find the first integer not stored in m_connectorForId
    //m_connectorForId is the only map, so the ids are sorted
    foreach (int existingId, m_connectorForId.keys()) {
        if (i != existingId) {
            return i;
        }

        ++i;
    }

    return i;
}

QList <int> ScreenPool::knownIds() const
{
    return m_connectorForId.keys();
}

#include "moc_screenpool.cpp"
