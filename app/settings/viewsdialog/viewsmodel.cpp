/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "viewsmodel.h"

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Model {

Views::Views(QObject *parent, Latte::Corona *corona)
    : QAbstractTableModel(parent),
      m_corona(corona)
{
}

Views::~Views()
{
}

void Views::clear()
{
    if (m_viewsTable.rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_viewsTable.rowCount() - 1);
        m_viewsTable.clear();
        endRemoveRows();
    }
}

int Views::rowCount() const
{
    return m_viewsTable.rowCount();
}

int Views::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_viewsTable.rowCount();
}

int Views::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return IDCOLUMN+1;
}

const Latte::Data::ViewsTable &Views::currentViewsData()
{
    return m_viewsTable;
}

const Latte::Data::ViewsTable &Views::originalViewsData()
{
    return o_viewsTable;
}


void Views::setOriginalData(Latte::Data::ViewsTable &data)
{
    clear();

    beginInsertRows(QModelIndex(), 0, data.rowCount() - 1);
    o_viewsTable = data;
    m_viewsTable = data;
    endInsertRows();

    emit rowsInserted();
}

QVariant Views::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (role == Qt::FontRole) {
        QFont font = qvariant_cast<QFont>(QAbstractTableModel::headerData(section, orientation, role));
        font.setBold(true);
        return font;
    }

    switch(section) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString("#");
        }
        break;
    case SCREENCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18n("Screen"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("desktop");
        }
        break;
    case EDGECOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("screen edge", "Edge"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("transform-move");
        }
        break;
    case ALIGNMENTCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18n("Alignment"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("format-justify-center");
        }
        break;
    default:
        break;
    };

    return QAbstractTableModel::headerData(section, orientation, role);
}


QVariant Views::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();
    //bool isNewLayout = !o_layoutsTable.containsId(m_layoutsTable[row].id);

    if (!m_viewsTable.rowExists(row)) {
        return QVariant{};
    }

    return QVariant{};
}

}
}
}
