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

#ifndef PLASMASCREENPOOL_H
#define PLASMASCREENPOOL_H

// Qt
#include <QHash>
#include <QMap>
#include <QObject>

// KDE
#include <KConfigGroup>

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

    KConfigGroup m_screensGroup;
};

}
}


#endif
