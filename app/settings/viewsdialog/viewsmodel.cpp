/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

const Latte::Data::View Views::currentData(const QString &id)
{
    if (!m_viewsTable.containsId(id)) {
        return Latte::Data::View();
    }

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

int Views::sortingFactorForState(const Data::View &view) const
{
    if (view.isActive) {
        return 1;
    } else if (view.state() == Data::View::IsCreated) {
        return 2;
    }

    //! temp case
    return 3;
}

int Views::sortingFactorForScreen(const Data::View &view) const
{
    if (view.onPrimary) {
        return 1;
    }

    //! explicit screen
    return 2;
}

int Views::sortingFactorForEdge(const Data::View &view) const
{
    if (view.edge == Plasma::Types::TopEdge) {
        return 1;
    } else if (view.edge == Plasma::Types::LeftEdge) {
        return 2;
    } else if (view.edge == Plasma::Types::BottomEdge) {
        return 3;
    }

    //! Right edge
    return 4;
}

int Views::sortingFactorForAlignment(const Data::View &view) const
{
    if (view.alignment == Latte::Types::Top || view.alignment == Latte::Types::Left) {
        return 1;
    } else if (view.alignment == Latte::Types::Center) {
        return 2;
    } else if (view.alignment == Latte::Types::Bottom || view.alignment == Latte::Types::Right) {
        return 3;
    }

    //! Justify alignment
    return 4;
}

int Views::sortingFactorForSubContainments(const Data::View &view) const
{
    return view.subcontainments.rowCount()+1;
}

QString Views::sortableText(const int &priority, const QString &text) const
{
    QString numberPart;

    if (priority < 10) {
        numberPart = "00000" + QString::number(priority);
    } else if (priority < 100) {
        numberPart = "0000" + QString::number(priority);
    } else if (priority < 1000) {
        numberPart = "000" + QString::number(priority);
    } else if (priority < 10000) {
        numberPart = "00" + QString::number(priority);
    } else if (priority < 100000) {
        numberPart = "0" + QString::number(priority);
    }

    return (numberPart + text);
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
    s_edges.clear();

    int i=0;
    s_edges << Data::View(QString::number(Plasma::Types::TopEdge), i18nc("top edge", "Top"));
    s_edges[i].edge = Plasma::Types::TopEdge; s_edges[i].alignment = Latte::Types::Center;
    s_edges[i].setState(Data::View::IsCreated);

    i++;
    s_edges << Data::View(QString::number(Plasma::Types::LeftEdge), i18nc("left edge", "Left"));
    s_edges[i].edge = Plasma::Types::LeftEdge; s_edges[i].alignment = Latte::Types::Center;
    s_edges[i].setState(Data::View::IsCreated);

    i++;
    s_edges << Data::View(QString::number(Plasma::Types::BottomEdge), i18nc("bottom edge", "Bottom"));
    s_edges[i].edge = Plasma::Types::BottomEdge; s_edges[i].alignment = Latte::Types::Center;
    s_edges[i].setState(Data::View::IsCreated);

    i++;
    s_edges << Data::View(QString::number(Plasma::Types::RightEdge), i18nc("right edge", "Right"));
    s_edges[i].edge = Plasma::Types::RightEdge; s_edges[i].alignment = Latte::Types::Center;
    s_edges[i].setState(Data::View::IsCreated);
}

void Views::initAlignments()
{
    s_horizontalAlignments.clear();
    s_verticalAlignments.clear();

    int i=0; // Left / Top
    s_horizontalAlignments << Data::View(QString::number(Latte::Types::Left), i18nc("left alignment", "Left"));
    s_horizontalAlignments[i].edge = Plasma::Types::BottomEdge; s_horizontalAlignments[i].alignment = Latte::Types::Left;
    s_horizontalAlignments[i].setState(Data::View::IsCreated);

    s_verticalAlignments << Data::View(QString::number(Latte::Types::Top), i18nc("top alignment", "Top"));
    s_verticalAlignments[i].edge = Plasma::Types::LeftEdge; s_verticalAlignments[i].alignment = Latte::Types::Top;
    s_verticalAlignments[i].setState(Data::View::IsCreated);

    i++; // Center
    s_horizontalAlignments << Data::View(QString::number(Latte::Types::Center), i18nc("center alignment", "Center"));
    s_horizontalAlignments[i].edge = Plasma::Types::BottomEdge; s_horizontalAlignments[i].alignment = Latte::Types::Center;
    s_horizontalAlignments[i].setState(Data::View::IsCreated);

    s_verticalAlignments << s_horizontalAlignments[1];
    s_verticalAlignments[i].edge = Plasma::Types::LeftEdge;
    s_verticalAlignments[i].setState(Data::View::IsCreated);

    i++; // Right / Bottom
    s_horizontalAlignments << Data::View(QString::number(Latte::Types::Right), i18nc("right alignment", "Right"));
    s_horizontalAlignments[i].edge = Plasma::Types::BottomEdge; s_horizontalAlignments[i].alignment = Latte::Types::Right;
    s_horizontalAlignments[i].setState(Data::View::IsCreated);

    s_verticalAlignments << Data::View(QString::number(Latte::Types::Bottom), i18nc("bottom alignment", "Bottom"));
    s_verticalAlignments[i].edge = Plasma::Types::LeftEdge; s_verticalAlignments[i].alignment = Latte::Types::Bottom;
    s_verticalAlignments[i].setState(Data::View::IsCreated);

    i++; // Justify
    s_horizontalAlignments << Data::View(QString::number(Latte::Types::Justify), i18nc("justify alignment", "Justify"));
    s_horizontalAlignments[i].edge = Plasma::Types::BottomEdge; s_horizontalAlignments[i].alignment = Latte::Types::Justify;
    s_horizontalAlignments[i].setState(Data::View::IsCreated);

    s_verticalAlignments << s_horizontalAlignments[3];
    s_verticalAlignments[i].edge = Plasma::Types::LeftEdge;
    s_verticalAlignments[i].setState(Data::View::IsCreated);
}

Data::ViewsTable Views::edgesChoices(const Data::View &view) const
{
    Data::ViewsTable t_edges = s_edges;

    if (view.alignment == Latte::Types::Left) {
        t_edges[0].alignment = view.alignment;
        t_edges[1].alignment = Latte::Types::Top;
        t_edges[2].alignment = view.alignment;
        t_edges[3].alignment = Latte::Types::Top;
    } else if (view.alignment == Latte::Types::Top) {
        t_edges[0].alignment = Latte::Types::Left;
        t_edges[1].alignment = view.alignment;
        t_edges[2].alignment = Latte::Types::Left;
        t_edges[3].alignment = view.alignment;
    } else if (view.alignment == Latte::Types::Center
               || view.alignment == Latte::Types::Justify) {
        t_edges[0].alignment = view.alignment;
        t_edges[1].alignment = view.alignment;
        t_edges[2].alignment = view.alignment;
        t_edges[3].alignment = view.alignment;
    } else if (view.alignment == Latte::Types::Right) {
        t_edges[0].alignment = view.alignment;
        t_edges[1].alignment = Latte::Types::Bottom;
        t_edges[2].alignment = view.alignment;
        t_edges[3].alignment = Latte::Types::Bottom;
    } else if (view.alignment == Latte::Types::Bottom) {
        t_edges[0].alignment = Latte::Types::Right;
        t_edges[1].alignment = view.alignment;
        t_edges[2].alignment = Latte::Types::Right;
        t_edges[3].alignment = view.alignment;
    }

    t_edges[0].screenEdgeMargin = view.screenEdgeMargin;
    t_edges[1].screenEdgeMargin = view.screenEdgeMargin;
    t_edges[2].screenEdgeMargin = view.screenEdgeMargin;
    t_edges[3].screenEdgeMargin = view.screenEdgeMargin;

    return t_edges;
}

Data::ViewsTable Views::horizontalAlignmentChoices(const Data::View &view) const
{
    Data::ViewsTable t_horizontalAlignments = s_horizontalAlignments;

    t_horizontalAlignments[0].edge = view.edge;
    t_horizontalAlignments[1].edge = view.edge;
    t_horizontalAlignments[2].edge = view.edge;
    t_horizontalAlignments[3].edge = view.edge;

    t_horizontalAlignments[0].screenEdgeMargin = view.screenEdgeMargin;
    t_horizontalAlignments[1].screenEdgeMargin = view.screenEdgeMargin;
    t_horizontalAlignments[2].screenEdgeMargin = view.screenEdgeMargin;
    t_horizontalAlignments[3].screenEdgeMargin = view.screenEdgeMargin;

    return t_horizontalAlignments;
}

Data::ViewsTable Views::verticalAlignmentChoices(const Data::View &view) const
{
    Data::ViewsTable t_verticalAlignments = s_verticalAlignments;

    t_verticalAlignments[0].edge = view.edge;
    t_verticalAlignments[1].edge = view.edge;
    t_verticalAlignments[2].edge = view.edge;
    t_verticalAlignments[3].edge = view.edge;

    t_verticalAlignments[0].screenEdgeMargin = view.screenEdgeMargin;
    t_verticalAlignments[1].screenEdgeMargin = view.screenEdgeMargin;
    t_verticalAlignments[2].screenEdgeMargin = view.screenEdgeMargin;
    t_verticalAlignments[3].screenEdgeMargin = view.screenEdgeMargin;

    return t_verticalAlignments;
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

QString Views::viewForSubContainment(const QString &sid)
{
    for(int i=0; i<m_viewsTable.rowCount(); ++i) {
       if (m_viewsTable[i].hasSubContainment(sid)) {
           return m_viewsTable[i].id;
       }
    }

    return QString();
}

void Views::updateActiveStatesBasedOn(const CentralLayout *layout)
{
    if (!layout) {
        return;
    }

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;
    roles << ISCHANGEDROLE;
    roles << ISACTIVEROLE;
    roles << HASCHANGEDVIEWROLE;

    for (int i=0; i<m_viewsTable.rowCount(); ++i) {
        uint viewid = m_viewsTable[i].id.toUInt();
        auto view = layout->viewForContainment(viewid);

        bool currentactivestate = (view != nullptr);

        if (currentactivestate != m_viewsTable[i].isActive) {
            m_viewsTable[i].isActive = currentactivestate;
            emit dataChanged(this->index(i, IDCOLUMN), this->index(i, SUBCONTAINMENTSCOLUMN), roles);
        }
    }
}

Latte::Data::Screen Views::screenData(const QString &viewId) const
{
    int row = rowForId(viewId);

    if (row < 0) {
        return Latte::Data::Screen();
    }

   // QString primaryid = QString::number(m_corona->screenPool()->primaryScreenId());
    QString explicitid = QString::number(m_viewsTable[row].screen);

    Data::Screen scrData = s_screens[0]; //default

    if (m_viewsTable[row].onPrimary || (m_viewsTable[row].screensGroup == Latte::Types::AllScreensGroup)) {
        scrData = s_screens[0]; //primary, allscreens
    } else if (m_viewsTable[row].screensGroup == Latte::Types::AllSecondaryScreensGroup) {
        scrData = s_screens[2]; //allsecondaryscreens
    } else if (!m_viewsTable[row].onPrimary && s_screens.containsId(explicitid)) {
        scrData = s_screens[explicitid]; //explicit
    }

    if (m_viewsTable[row].screensGroup == Latte::Types::AllScreensGroup) {
        scrData.id = QString::number(Data::Screen::ONALLSCREENSID);
    } else if (m_viewsTable[row].screensGroup == Latte::Types::AllSecondaryScreensGroup) {
        scrData.id = QString::number(Data::Screen::ONALLSECONDARYSCREENSID);
    }

    return scrData;
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

Latte::Data::ViewsTable Views::newViews() const
{
    Latte::Data::ViewsTable views;

    for(int i=0; i<rowCount(); ++i) {
        QString currentId = m_viewsTable[i].id;

        if (!o_viewsTable.containsId(currentId)) {
            views << m_viewsTable[i];
        }
    }

    return views;
}

void Views::clearErrorsAndWarnings()
{
    for(int i=0; i<m_viewsTable.rowCount(); ++i) {
        m_viewsTable[i].errors = 0;
        m_viewsTable[i].warnings = 0;
    }

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;
    roles << ERRORSROLE;
    roles << WARNINGSROLE;

    emit dataChanged(this->index(0, IDCOLUMN), this->index(m_viewsTable.rowCount()-1, SUBCONTAINMENTSCOLUMN), roles);
}

void Views::populateScreens()
{
    s_screens.clear();
    Data::Screen primary(QString::number(Data::Screen::ONPRIMARYID), i18n(" - On Primary Screen - "));
    Data::Screen allscreens(QString::number(Data::Screen::ONALLSCREENSID), i18n(" - On All Screens - "));
    Data::Screen allsecscreens(QString::number(Data::Screen::ONALLSECONDARYSCREENSID), i18n(" - On All Secondary Screens - "));

    primary.isActive = true;    
    allscreens.isActive = true;
    allsecscreens.isActive = (m_corona->screenPool()->secondaryScreenIds().count() > 0);

    s_screens << primary;
    s_screens << allscreens;
    s_screens << allsecscreens;
    int defcount = s_screens.rowCount();
    s_screens << m_corona->screenPool()->screensTable();

    for (int i=defcount; i<s_screens.rowCount(); ++i) {
        s_screens[i].isActive = m_corona->screenPool()->isScreenActive(s_screens[i].id.toInt());
    }
}

void Views::updateCurrentView(QString currentViewId, Latte::Data::View &view)
{
    if (!m_viewsTable.containsId(currentViewId)) {
        return;
    }

    int currentrow = m_viewsTable.indexOf(currentViewId);
    m_viewsTable[currentrow] = view;

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;
    roles << ISCHANGEDROLE;
    roles << ISACTIVEROLE;
    roles << HASCHANGEDVIEWROLE;
    roles << ISMOVEORIGINROLE;
    roles << ERRORSROLE;
    roles << WARNINGSROLE;

    emit dataChanged(this->index(currentrow, IDCOLUMN), this->index(currentrow, SUBCONTAINMENTSCOLUMN), roles);
}

void Views::setOriginalView(QString currentViewId, Latte::Data::View &view)
{
    if (!m_viewsTable.containsId(currentViewId)) {
        return;
    }

    int currentrow = m_viewsTable.indexOf(currentViewId);
    o_viewsTable << view;
    m_viewsTable[currentrow] = view;

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;
    roles << ISCHANGEDROLE;
    roles << ISACTIVEROLE;
    roles << HASCHANGEDVIEWROLE;

    emit dataChanged(this->index(currentrow, IDCOLUMN), this->index(currentrow, SUBCONTAINMENTSCOLUMN), roles);
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
    roles << SORTINGROLE;

    if (role != Qt::DisplayRole) {
        roles << Qt::DisplayRole;
    }

    //! specific roles to each independent cell
    switch (column) {
    case NAMECOLUMN:
        if (role == Qt::UserRole || role == Qt::EditRole) {
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
            bool onallscreens = (screen == Latte::Data::Screen::ONALLSCREENSID);
            bool onallsecscreens = (screen == Latte::Data::Screen::ONALLSECONDARYSCREENSID);

            if (onprimary) {
                m_viewsTable[row].onPrimary = true;
                m_viewsTable[row].screensGroup = Latte::Types::SingleScreenGroup;
            } else if (onallscreens) {
                m_viewsTable[row].onPrimary = true;
                m_viewsTable[row].screensGroup = Latte::Types::AllScreensGroup;
            } else if (onallsecscreens) {
                m_viewsTable[row].onPrimary = false;
                m_viewsTable[row].screensGroup = Latte::Types::AllSecondaryScreensGroup;
            } else {
                m_viewsTable[row].onPrimary = false;
                m_viewsTable[row].screensGroup = Latte::Types::SingleScreenGroup;
                m_viewsTable[row].screen = screen;
            }

            if (onprimary || onallscreens || onallsecscreens) {
                if (o_viewsTable.containsId(m_viewsTable[row].id)) {
                    //! we need to update screen also in order to not show that there are changes even though
                    //! they are not any
                    m_viewsTable[row].screen = o_viewsTable[m_viewsTable[row].id].screen;
                }
            }

            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row, ALIGNMENTCOLUMN), roles);
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

                emit dataChanged(this->index(row, NAMECOLUMN), this->index(row, ALIGNMENTCOLUMN), roles);
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
            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row, ALIGNMENTCOLUMN), roles);
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
                                         i18nc("unknown screen", "Unknown: [%1]", explicitScr.id));
                currentScreens.insertBasedOnId(explicitScr);
            }

            screensVariant.setValue<Latte::Data::ScreensTable>(currentScreens);
            return screensVariant;
        } else if (column == EDGECOLUMN) {
            QVariant edgesVariant;
            edgesVariant.setValue<Latte::Data::ViewsTable>(edgesChoices(m_viewsTable[row]));
            return edgesVariant;
        } else if (column == ALIGNMENTCOLUMN) {
            QVariant alignmentsVariant;

            if (isVertical(m_viewsTable[row].edge)) {
                alignmentsVariant.setValue<Latte::Data::ViewsTable>(verticalAlignmentChoices(m_viewsTable[row]));
            } else {
                alignmentsVariant.setValue<Latte::Data::ViewsTable>(horizontalAlignmentChoices(m_viewsTable[row]));
            }

            return alignmentsVariant;
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
    } else if (role == ISMOVEORIGINROLE) {
        return m_viewsTable[row].isMoveOrigin;
    } else if (role == ERRORSROLE) {
        return m_viewsTable[row].errors;
    } else if (role == WARNINGSROLE) {
        return m_viewsTable[row].warnings;
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
        }  else if (role == SORTINGROLE) {
            int fsta = sortingFactorForState(m_viewsTable[row]);
            int fscr = sortingFactorForScreen(m_viewsTable[row]);
            int fedg = sortingFactorForEdge(m_viewsTable[row]);
            int fali = sortingFactorForAlignment(m_viewsTable[row]);

            int priority = (fsta * HIGHESTPRIORITY);
            return sortableText(priority, m_viewsTable[row].id);
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole || role == Qt::EditRole){
            return m_viewsTable[row].name;
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].name != o_viewsTable[origviewid].name));
        } else if (role == SORTINGROLE) {
            int fsta = sortingFactorForState(m_viewsTable[row]);
            int fscr = sortingFactorForScreen(m_viewsTable[row]);
            int fedg = sortingFactorForEdge(m_viewsTable[row]);
            int fali = sortingFactorForAlignment(m_viewsTable[row]);

            int priority = (fsta * HIGHESTPRIORITY);
            return sortableText(priority, m_viewsTable[row].name);
        }
        break;
    case SCREENCOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].screensGroup == Latte::Types::SingleScreenGroup &&  m_viewsTable[row].onPrimary) {
                return i18nc("primary screen", "Primary");
            } else if (m_viewsTable[row].screensGroup == Latte::Types::AllScreensGroup) {
                return i18n("All Screens");
            } else if (m_viewsTable[row].screensGroup == Latte::Types::AllSecondaryScreensGroup) {
                return i18n("Secondary Screens");
            } else {
                QString scrId = QString::number(m_viewsTable[row].screen);
                if (s_screens.containsId(scrId)) {
                    return s_screens[scrId].name;
                } else {
                    return i18nc("unknown screen", "Unknown: [%1]", scrId);
                }
            }
        } else if (role == Qt::UserRole) {
            if (m_viewsTable[row].screensGroup == Latte::Types::SingleScreenGroup &&  m_viewsTable[row].onPrimary) {
                return QString::number(Data::Screen::ONPRIMARYID);
            } else if (m_viewsTable[row].screensGroup == Latte::Types::AllScreensGroup) {
                return QString::number(Data::Screen::ONALLSCREENSID);
            } else if (m_viewsTable[row].screensGroup == Latte::Types::AllSecondaryScreensGroup) {
                return QString::number(Data::Screen::ONALLSECONDARYSCREENSID);
            } else {
                return QString::number(m_viewsTable[row].screen);
            }
        } else if (role == ISCHANGEDROLE) {
            return (isNewView
                    || (m_viewsTable[row].onPrimary != o_viewsTable[origviewid].onPrimary)
                    || (!m_viewsTable[row].onPrimary && m_viewsTable[row].screen != o_viewsTable[origviewid].screen));
        } else if (role == SORTINGROLE) {
            int fsta = sortingFactorForState(m_viewsTable[row]);
            int fscr = sortingFactorForScreen(m_viewsTable[row]);
            int fedg = sortingFactorForEdge(m_viewsTable[row]);
            int fali = sortingFactorForAlignment(m_viewsTable[row]);

            int priority = (fsta * HIGHESTPRIORITY) + (fscr * HIGHPRIORITY) + (fedg * MEDIUMPRIORITY) + (fali * NORMALPRIORITY);
            return priority;
        }
        break;
    case EDGECOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].edge == Plasma::Types::BottomEdge) {
                return i18nc("bottom location", "Bottom");
            } else if (m_viewsTable[row].edge == Plasma::Types::TopEdge) {
                return i18nc("top location", "Top");
            } else if (m_viewsTable[row].edge == Plasma::Types::LeftEdge) {
                return i18nc("left location", "Left");
            } else if (m_viewsTable[row].edge == Plasma::Types::RightEdge) {
                return i18nc("right location", "Right");
            }

            return i18nc("unknown location", "Unknown");
        } else if (role == Qt::UserRole) {
            return QString::number(m_viewsTable[row].edge);
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].edge != o_viewsTable[origviewid].edge));
        } else if (role == SORTINGROLE) {
            int fsta = sortingFactorForState(m_viewsTable[row]);
            int fscr = sortingFactorForScreen(m_viewsTable[row]);
            int fedg = sortingFactorForEdge(m_viewsTable[row]);
            int fali = sortingFactorForAlignment(m_viewsTable[row]);

            int priority = (fsta * HIGHESTPRIORITY) + (fedg * HIGHPRIORITY) + (fscr * MEDIUMPRIORITY) + (fali * NORMALPRIORITY);
            return priority;
        }
        break;
    case ALIGNMENTCOLUMN:
        if (role == Qt::DisplayRole){
            if (m_viewsTable[row].alignment == Latte::Types::Center) {
                return i18nc("center alignment", "Center");
            } else if (m_viewsTable[row].alignment == Latte::Types::Left) {
                return i18nc("left alignment", "Left");
            } else if (m_viewsTable[row].alignment == Latte::Types::Right) {
                return i18nc("right alignment", "Right");
            } else if (m_viewsTable[row].alignment == Latte::Types::Top) {
                return i18nc("top alignment", "Top");
            } else if (m_viewsTable[row].alignment == Latte::Types::Bottom) {
                return i18nc("bottom alignment", "Bottom");
            } else if (m_viewsTable[row].alignment == Latte::Types::Justify) {
                return i18nc("justify alignment", "Justify");
            }

            return i18nc("unknown alignment", "Unknown");
        } else if (role == Qt::UserRole) {
            return QString::number(m_viewsTable[row].alignment);
        } else if (role == ISCHANGEDROLE) {
            return (isNewView || (m_viewsTable[row].alignment != o_viewsTable[origviewid].alignment));
        } else if (role == SORTINGROLE) {
            int fsta = sortingFactorForState(m_viewsTable[row]);
            int fscr = sortingFactorForScreen(m_viewsTable[row]);
            int fedg = sortingFactorForEdge(m_viewsTable[row]);
            int fali = sortingFactorForAlignment(m_viewsTable[row]);

            int priority = (fsta * HIGHESTPRIORITY) + (fali * HIGHPRIORITY) + (fscr * MEDIUMPRIORITY) + (fedg * NORMALPRIORITY);
            return priority;
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
        } else if (role == SORTINGROLE) {
            int fsta = sortingFactorForState(m_viewsTable[row]);
            int fscr = sortingFactorForScreen(m_viewsTable[row]);
            int fedg = sortingFactorForEdge(m_viewsTable[row]);
            int fali = sortingFactorForAlignment(m_viewsTable[row]);
            int fsub = sortingFactorForSubContainments(m_viewsTable[row]);

            int priority = (fsta * HIGHESTPRIORITY) + (fsub * HIGHPRIORITY) + (fscr * MEDIUMPRIORITY) + (fedg * NORMALPRIORITY);
            return priority;
        }
    };

    return QVariant{};
}

}
}
}
