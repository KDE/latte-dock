/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SETTINGSLAYOUTSMODEL_H
#define SETTINGSLAYOUTSMODEL_H

// local
#include "../data/layoutstable.h"
// Qt
#include <QAbstractTableModel>
#include <QModelIndex>


namespace Latte {
namespace Settings {
namespace Model {

class Layouts : public QAbstractTableModel
{
public:
    const int IDCOLUMN = 0;
    const int HIDDENTEXTCOLUMN = 1;
    const int COLORCOLUMN = 2;
    const int NAMECOLUMN = 3;
    const int MENUCOLUMN = 4;
    const int BORDERSCOLUMN = 5;
    const int ACTIVITYCOLUMN = 6;
    const int SHAREDCOLUMN = 7;

    explicit Layouts(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    Data::LayoutsTable m_layoutsTable;
};

}
}
}

#endif
