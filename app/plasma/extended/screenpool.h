/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMASCREENPOOL_H
#define PLASMASCREENPOOL_H

// Qt
#include <QHash>
#include <QMap>
#include <QObject>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

class PrimaryOutputWatcher;

namespace Latte {
namespace PlasmaExtended {

class ScreenPool: public QObject
{
    Q_OBJECT

public:
    ScreenPool(QObject *parent = nullptr);
    ~ScreenPool() override;

    int id(const QString &connector) const;
    QString connector(int id) const;

signals:
    void idsChanged();

private slots:
    void load();
    void insertScreenMapping(int id, const QString &connector);

private:
    QHash<int, QString> m_screens;

    //order is important
    QMap<int, QString> m_connectorForId;
    QHash<QString, int> m_idForConnector;

    KSharedConfig::Ptr m_plasmarcConfig;
    KConfigGroup m_screensGroup;

    PrimaryOutputWatcher *m_primaryWatcher;
};

}
}


#endif
