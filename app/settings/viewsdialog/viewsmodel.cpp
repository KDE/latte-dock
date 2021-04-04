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

// local
#include <coretypes.h>
#include "../../data/genericdata.h"
#include "../../data/screendata.h"

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Model {

Views::Views(QObject *parent, Latte::Corona *corona)
    : QAbstractTableModel(parent),
      m_corona(corona)
{
    initEdges();
    initAlignments();
    populateScreens();
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

int Views::columnCount()
{
    return LASTCOLUMN;
}

int Views::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_viewsTable.rowCount();
}

int Views::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columnCount();
}

const Latte::Data::ViewsTable &Views::currentViewsData()
{
    return m_viewsTable;
}

const Latte::Data::ViewsTable &Views::originalViewsData()
{
    return o_viewsTable;
}

void Views::initEdges()
{
    Latte::Data::GenericBasicTable edges;
    edges << Data::Generic(QString::number(Plasma::Types::TopEdge), i18nc("top edge", "Top"));
    edges << Data::Generic(QString::number(Plasma::Types::LeftEdge), i18nc("left edge", "Left"));
    edges << Data::Generic(QString::number(Plasma::Types::BottomEdge), i18nc("bottom edge", "Bottom"));
    edges << Data::Generic(QString::number(Plasma::Types::RightEdge), i18nc("right edge", "Right"));

    s_edges.setValue<Latte::Data::GenericBasicTable>(edges);
}

void Views::initAlignments()
{
    Latte::Data::GenericBasicTable horizontals;
    Latte::Data::GenericBasicTable verticals;

    horizontals << Data::Generic(QString::number(Latte::Types::Left), i18nc("left alignment", "Left"));
    verticals << Data::Generic(QString::number(Latte::Types::Left), i18nc("top alignment", "Top"));


    horizontals << Data::Generic(QString::number(Latte::Types::Center), i18nc("center alignment", "Center"));
    verticals << horizontals[1];

    horizontals << Data::Generic(QString::number(Latte::Types::Right), i18nc("right alignment", "Right"));
    verticals << Data::Generic(QString::number(Latte::Types::Left), i18nc("bottom alignment", "Bottom"));

    horizontals << Data::Generic(QString::number(Latte::Types::Right), i18nc("justify alignment", "Justify"));
    verticals << horizontals[3];

    s_horizontalAlignments.setValue<Latte::Data::GenericBasicTable>(horizontals);
    s_verticalAlignments.setValue<Latte::Data::GenericBasicTable>(verticals);
}

void Views::populateScreens()
{
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
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18n("Name"));
        }
        break;
    case SCREENCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18n("Screen"));
        }
        /*  } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("desktop");
        }*/
        break;
    case EDGECOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("screen edge", "Edge"));
        }
        /*  } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("transform-move");
        }*/
        break;
    case ALIGNMENTCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18n("Alignment"));
        }
        /*} else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("format-justify-center");
        }*/
        break;
    case SUBCONTAINMENTSCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18n("Includes"));
        }
    default:
        break;
    };

    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags Views::flags(const QModelIndex &index) const
{
    const int column = index.column();
    const int row = index.row();

    auto flags = QAbstractTableModel::flags(index);

    if (column == SCREENCOLUMN
            || column == EDGECOLUMN
            || column == ALIGNMENTCOLUMN) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}


QVariant Views::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();
    //bool isNewLayout = !o_layoutsTable.containsId(m_layoutsTable[row].id);

    if (!m_viewsTable.rowExists(row)) {
        return QVariant{};
    }

    if (role == IDROLE) {
        return m_viewsTable[row].id;
    } else if (role == ISACTIVEROLE) {
        return m_viewsTable[row].isActive;
    } else if (role == CHOICESROLE) {
        if (column == EDGECOLUMN) {
            return s_edges;
        } else if (column == ALIGNMENTCOLUMN) {
            bool isVertical = (m_viewsTable[row].edge == Plasma::Types::LeftEdge || m_viewsTable[row].edge == Plasma::Types::RightEdge);
            return isVertical ? s_verticalAlignments : s_horizontalAlignments;
        }
    }


    if (role == Qt::TextAlignmentRole){
        return static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    switch (column) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole){
            return m_viewsTable[row].id;
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole){
            return m_viewsTable[row].name;
        }
        break;
    case SCREENCOLUMN:
        if (role == Qt::DisplayRole){
            return (m_viewsTable[row].onPrimary ? QString("Primary") : QString::number(m_viewsTable[row].screen));
        } else if (role == Qt::UserRole) {
            return m_viewsTable[row].onPrimary ? QString::number(Data::Screen::ONPRIMARYID) : QString::number(m_viewsTable[row].screen);
        }
        break;
    case EDGECOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].edge == Plasma::Types::BottomEdge) {
                return QString("Bottom");
            } else if (m_viewsTable[row].edge == Plasma::Types::TopEdge) {
                return QString("Top");
            } else if (m_viewsTable[row].edge == Plasma::Types::LeftEdge) {
                return QString("Left");
            } else if (m_viewsTable[row].edge == Plasma::Types::RightEdge) {
                return QString("Right");
            }

            return QString("Unknown");
        } else if (role == Qt::UserRole) {
            return QString::number(m_viewsTable[row].edge);
        }
        break;
    case ALIGNMENTCOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].alignment == Latte::Types::Center) {
                return QString("Center");
            } else if (m_viewsTable[row].alignment == Latte::Types::Left) {
                return QString("Left");
            } else if (m_viewsTable[row].alignment == Latte::Types::Right) {
                return QString("Right");
            } else if (m_viewsTable[row].alignment == Latte::Types::Top) {
                return QString("Top");
            } else if (m_viewsTable[row].alignment == Latte::Types::Bottom) {
                return QString("Bottom");
            } else if (m_viewsTable[row].alignment == Latte::Types::Justify) {
                return QString("Justify");
            }

            return QString("Unknown");
        } else if (role == Qt::UserRole) {
            return QString::number(m_viewsTable[row].alignment);
        }
        break;
    case SUBCONTAINMENTSCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole){
            return m_viewsTable[row].subcontainments.rowCount()>0 ? QString("{" + m_viewsTable[row].subcontainments + "}") : QString();
        }
    };

    return QVariant{};
}

}
}
}
