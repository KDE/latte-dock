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

#include "layoutsmodel.h"

namespace Latte {
namespace Settings {
namespace Model {

Layouts::Layouts(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int Layouts::rowCount(const QModelIndex &parent) const
{
    return m_layoutsTable.rowCount();
}

int Layouts::columnCount(const QModelIndex &parent) const
{
    return SHAREDCOLUMN;
}

QVariant Layouts::data(const QModelIndex &index, int role) const
{
    return QVariant();
}


}
}
}

