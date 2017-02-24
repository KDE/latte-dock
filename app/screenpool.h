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

#ifndef SCREENPOOL_H
#define SCREENPOOL_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QTimer>

#include <KConfigGroup>
#include <KSharedConfig>

class ScreenPool : public QObject {
    Q_OBJECT

public:
    ScreenPool(KSharedConfig::Ptr config, QObject *parent = nullptr);
    void load();
    ~ScreenPool() override;

    QString primaryConnector() const;
    void setPrimaryConnector(const QString &primary);

    void insertScreenMapping(int id, const QString &connector);

    int id(const QString &connector) const;

    QString connector(int id) const;

    int firstAvailableId() const;

    //all ids that are known, included screens not enabled at the moment
    QList <int> knownIds() const;

private:
    void save();

    KConfigGroup m_configGroup;
    QString m_primaryConnector;
    //order is important
    QMap<int, QString> m_connectorForId;
    QHash<QString, int> m_idForConnector;

    QTimer m_configSaveTimer;
};

#endif // SCREENPOOL_H
