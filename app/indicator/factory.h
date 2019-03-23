/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef INDICATORFACTORY_H
#define INDICATORFACTORY_H

// Qt
#include <QHash>
#include <QObject>

class KPluginMetaData;

namespace Latte {
namespace Indicator {

class Factory : public QObject
{
    Q_OBJECT

public:
    Factory(QObject *parent);
    ~Factory() override;

    void reload();

    KPluginMetaData metadata(QString pluginId);

private:
    QHash<QString, KPluginMetaData> m_plugins;

};

}
}

#endif
