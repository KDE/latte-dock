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
#include "../../screenpool.h"
#include "../../data/genericdata.h"
#include "../../data/screendata.h"

// KDE
#include <KLocalizedString>

#define TEMPIDDISPLAY "#"

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

bool Views::hasChangedData() const
{
    return o_viewsTable != m_viewsTable;
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

int Views::rowForId(const QString &id) const
{
    return m_viewsTable.indexOf(id);
}

const Latte::Data::View &Views::at(const int &row)
{
    return m_viewsTable[row];
}

const Latte::Data::View &Views::currentData(const QString &id)
{
    return m_viewsTable[id];
}


const Latte::Data::View Views::originalData(const QString &id)
{
    if (o_viewsTable.containsId(id)){
        return o_viewsTable[id];
    }

    return Latte::Data::View();
}

const Latte::Data::ViewsTable &Views::currentViewsData()
{
    return m_viewsTable;
}

const Latte::Data::ViewsTable &Views::originalViewsData()
{
    return o_viewsTable;
}

void Views::clear()
{
    if (m_viewsTable.rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_viewsTable.rowCount() - 1);
        m_viewsTable.clear();
        endRemoveRows();
    }
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
    verticals << Data::Generic(QString::number(Latte::Types::Top), i18nc("top alignment", "Top"));


    horizontals << Data::Generic(QString::number(Latte::Types::Center), i18nc("center alignment", "Center"));
    verticals << horizontals[1];

    horizontals << Data::Generic(QString::number(Latte::Types::Right), i18nc("right alignment", "Right"));
    verticals << Data::Generic(QString::number(Latte::Types::Bottom), i18nc("bottom alignment", "Bottom"));

    horizontals << Data::Generic(QString::number(Latte::Types::Justify), i18nc("justify alignment", "Justify"));
    verticals << horizontals[3];

    s_horizontalAlignments.setValue<Latte::Data::GenericBasicTable>(horizontals);
    s_verticalAlignments.setValue<Latte::Data::GenericBasicTable>(verticals);
}

bool Views::containsCurrentName(const QString &name) const
{
    return m_viewsTable.containsName(name);
}

void Views::resetData()
{
    clear();
    setOriginalData(o_viewsTable);
}

void Views::appendTemporaryView(const Latte::Data::View &view)
{
    //int newRow = m_layoutsTable.sortedPosForName(layout.name);

    beginInsertRows(QModelIndex(), m_viewsTable.rowCount(), m_viewsTable.rowCount());
    m_viewsTable.appendTemporaryView(view);
    endInsertRows();

    emit rowsInserted();
}

void Views::removeView(const QString &id)
{
    int index = m_viewsTable.indexOf(id);

    if (index >= 0) {
        removeRows(index, 1);
        emit rowsRemoved();
    }
}

bool Views::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    int firstRow = row;
    int lastRow = row+count-1;

    if (count > 0 && m_viewsTable.rowExists(firstRow) && (m_viewsTable.rowExists(lastRow))) {
        beginRemoveRows(QModelIndex(), firstRow, lastRow);
        for(int i=0; i<count; ++i) {
            m_viewsTable.remove(firstRow);
        }
        endRemoveRows();
        return true;
    }

    return false;
}

bool Views::isVertical(const Plasma::Types::Location &location) const
{
    return (location == Plasma::Types::LeftEdge || location == Plasma::Types::RightEdge);
}

Latte::Data::Screen Views::screenData(const QString &viewId) const
{
    int row = rowForId(viewId);

    if (row < 0) {
        return Latte::Data::Screen();
    }

    QString primaryid = QString::number(m_corona->screenPool()->primaryScreenId());
    QString explicitid = QString::number(m_viewsTable[row].screen);

    if (m_viewsTable[row].onPrimary && s_screens.containsId(primaryid)) {
        return s_screens[primaryid];
    } else if (!m_viewsTable[row].onPrimary && s_screens.containsId(explicitid)) {
        return s_screens[explicitid];
    }

    return Latte::Data::Screen();
}

Latte::Data::ViewsTable Views::alteredViews() const
{
    Latte::Data::ViewsTable views;

    for(int i=0; i<rowCount(); ++i) {
        QString currentId = m_viewsTable[i].id;

        if (!o_viewsTable.containsId(currentId)
            || m_viewsTable[currentId] != o_viewsTable[currentId]) {
            views << m_viewsTable[i];
        }
    }

    return views;
}

void Views::populateScreens()
{
    s_screens.clear();
    Data::Screen primary(QString::number(Data::Screen::ONPRIMARYID),
                         i18nc("primary screen", " - Follow Primary Screen - "));

    primary.isActive = true;
    s_screens << primary;
    s_screens << m_corona->screenPool()->screensTable();

    for (int i=1; i<s_screens.rowCount(); ++i) {
        s_screens[i].isActive = m_corona->screenPool()->isScreenActive(s_screens[i].id.toInt());
    }
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

    if (column == NAMECOLUMN
            || column == SCREENCOLUMN
            || column == EDGECOLUMN
            || column == ALIGNMENTCOLUMN) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool Views::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int column = index.column();

    if (!m_viewsTable.rowExists(row) || column<0 || column >= SUBCONTAINMENTSCOLUMN) {
        return false;
    }

    QVector<int> roles;
    roles << role;
    roles << ISCHANGEDROLE;
    roles << HASCHANGEDVIEWROLE;

    if (role != Qt::DisplayRole) {
        roles << Qt::DisplayRole;
    }

    //! specific roles to each independent cell
    switch (column) {
    case NAMECOLUMN:
        if (role == Qt::UserRole || role == Qt::EditRole ) {
            if (m_viewsTable[row].name == value.toString()) {
                return false;
            }

            m_viewsTable[row].name = value.toString();
            emit dataChanged(index, index, roles);
        }
        break;
    case SCREENCOLUMN:
        if (role == Qt::UserRole) {
            int screen = value.toString().toInt();
            bool onprimary = (screen == Latte::Data::Screen::ONPRIMARYID);

            if ((m_viewsTable[row].onPrimary == onprimary) && (m_viewsTable[row].screen == screen)) {
                return false;
            }

            if (onprimary) {
                m_viewsTable[row].onPrimary = true;

                if (o_viewsTable.containsId(m_viewsTable[row].id)) {
                    //! we need to update screen also in order to not show that there are changes even though
                    //! they are not any
                    m_viewsTable[row].screen = o_viewsTable[m_viewsTable[row].id].screen;
                }
            } else {
                m_viewsTable[row].onPrimary = false;
                m_viewsTable[row].screen = screen;
            }

            emit dataChanged(index, index, roles);
        }
        break;
    case EDGECOLUMN:
        if (role == Qt::UserRole) {
            Plasma::Types::Location edge = static_cast<Plasma::Types::Location>(value.toString().toInt());

            if (m_viewsTable[row].edge == edge) {
                return false;
            }

            Plasma::Types::Location previousEdge = m_viewsTable[row].edge;
            m_viewsTable[row].edge = edge;
            emit dataChanged(index, index, roles);

            bool previousFactor = isVertical(previousEdge);
            bool currentFactor = isVertical(edge);

            if (previousFactor != currentFactor) {
                if (m_viewsTable[row].alignment == Latte::Types::Left) {
                    m_viewsTable[row].alignment = Latte::Types::Top;
                } else if (m_viewsTable[row].alignment == Latte::Types::Right) {
                    m_viewsTable[row].alignment = Latte::Types::Bottom;
                } else if (m_viewsTable[row].alignment == Latte::Types::Top) {
                    m_viewsTable[row].alignment = Latte::Types::Left;
                } else if (m_viewsTable[row].alignment == Latte::Types::Bottom) {
                    m_viewsTable[row].alignment = Latte::Types::Right;
                }

                emit dataChanged(this->index(row, ALIGNMENTCOLUMN), this->index(row, ALIGNMENTCOLUMN), roles);
            }

            return true;
        }
        break;
    case ALIGNMENTCOLUMN:
        if (role == Qt::UserRole)  {
            int alignment = value.toString().toInt();

            if (m_viewsTable[row].alignment == alignment) {
                return false;
            }

            m_viewsTable[row].alignment = static_cast<Latte::Types::Alignment>(alignment);
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    };

    return false;
}


QVariant Views::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();
    //bool isNewLayout = !o_layoutsTable.containsId(m_layoutsTable[row].id);

    if (!m_viewsTable.rowExists(row)) {
        return QVariant{};
    }

    bool isNewView = !o_viewsTable.containsId(m_viewsTable[row].id);
    QString origviewid = !isNewView ? m_viewsTable[row].id : "";


    if (role == IDROLE) {
        return (m_viewsTable[row].state() == Data::View::IsCreated ? m_viewsTable[row].id : "#");
    } else if (role == ISACTIVEROLE) {
        return m_viewsTable[row].isActive;
    } else if (role == CHOICESROLE) {
        if (column == SCREENCOLUMN) {
            QVariant screensVariant;

            Latte::Data::ScreensTable currentScreens = s_screens;

            if (!m_viewsTable[row].onPrimary && !currentScreens.containsId(QString::number(m_viewsTable[row].screen))) {
                Data::Screen explicitScr(QString::number(m_viewsTable[row].screen),
                                         i18nc("unknown screen", "Unknown : [%0]").arg(explicitScr.id));
                currentScreens.insertBasedOnId(explicitScr);
            }

            screensVariant.setValue<Latte::Data::ScreensTable>(currentScreens);
            return screensVariant;
        } else if (column == EDGECOLUMN) {
            return s_edges;
        } else if (column == ALIGNMENTCOLUMN) {
            return isVertical(m_viewsTable[row].edge) ? s_verticalAlignments : s_horizontalAlignments;
        }
    } else if (role == HASCHANGEDVIEWROLE) {
        return (isNewView || (m_viewsTable[row] != o_viewsTable[origviewid]));
    } else if (role == SCREENROLE) {
        QVariant scrVariant;
        Latte::Data::Screen scrdata = screenData(m_viewsTable[row].id);
        scrVariant.setValue<Latte::Data::Screen>(scrdata);
        return scrVariant;
    } else if (role == VIEWROLE) {
        QVariant viewVariant;
        viewVariant.setValue<Latte::Data::View>(m_viewsTable[row]);
        return viewVariant;
    }

    if (role == Qt::TextAlignmentRole && column != NAMECOLUMN){
        return static_cast<Qt::Alignment::Int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    switch (column) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole){
            return (m_viewsTable[row].state() == Data::View::IsCreated ? m_viewsTable[row].id : "#");
        } else if (role == Qt::UserRole) {
            return m_viewsTable[row].id;
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].id != o_viewsTable[origviewid].id));
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole || role == Qt::EditRole){
            return m_viewsTable[row].name;
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].name != o_viewsTable[origviewid].name));
        }
        break;
    case SCREENCOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].onPrimary) {
                return QString("Primary");
            } else {
                QString scrId = QString::number(m_viewsTable[row].screen);
                if (s_screens.containsId(scrId)) {
                    return s_screens[scrId].name;
                } else {
                    return i18nc("unknown screen", "Unknown : [%0]").arg(scrId);
                }
            }
        } else if (role == Qt::UserRole) {
            return m_viewsTable[row].onPrimary ? QString::number(Data::Screen::ONPRIMARYID) : QString::number(m_viewsTable[row].screen);
        } else if (role == ISCHANGEDROLE) {
            return (isNewView
                    || (m_viewsTable[row].onPrimary != o_viewsTable[origviewid].onPrimary)
                    || (!m_viewsTable[row].onPrimary && m_viewsTable[row].screen != o_viewsTable[origviewid].screen));
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
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].edge != o_viewsTable[origviewid].edge));
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
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].alignment != o_viewsTable[origviewid].alignment));
        }
        break;
    case SUBCONTAINMENTSCOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].subcontainments.rowCount()>0) {
                QString result = "{";

                for (int i=0; i<m_viewsTable[row].subcontainments.rowCount(); ++i) {
                    if (i>0) {
                        result += " ";
                    }
                    result += (m_viewsTable[row].state() == Data::View::IsCreated ? m_viewsTable[row].subcontainments[i].id : TEMPIDDISPLAY);

                    if (i<m_viewsTable[row].subcontainments.rowCount()-1) {
                        result += ",";
                    }
                }

                result += "}";
                return result;
            }

            return QString();
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].subcontainments != o_viewsTable[origviewid].subcontainments));
        }
    };

    return QVariant{};
}

}
}
}
