/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "screensmodel.h"

// Qt
#include <QFont>
#include <QIcon>

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Model {

Screens::Screens(QObject *parent)
    : QAbstractTableModel(parent)
{
}

Screens::~Screens()
{
}

bool Screens::hasChangedData() const
{
    return c_screens != o_screens;
}

int Screens::rowCount() const
{
    return c_screens.rowCount();
}

int Screens::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return c_screens.rowCount();
}

int Screens::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

int Screens::row(const QString &id)
{
    for (int i=0; i<c_screens.rowCount(); ++i){
        if (c_screens[i].id == id) {
            return i;
        }
    }

    return -1;
}

bool Screens::inDefaultValues() const
{
    return c_screens == o_screens;
}

void Screens::initDefaults()
{
    for(int i=0; i<c_screens.rowCount(); ++i) {
        c_screens[i].isSelected = false;
    }
}

void Screens::clear()
{
    if (c_screens.rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, c_screens.rowCount() - 1);
        c_screens.clear();
        endRemoveRows();

        emit screensDataChanged();
    }
}

void Screens::reset()
{
    c_screens = o_screens;

    QVector<int> roles;
    roles << Qt::CheckStateRole;

    emit dataChanged(index(0, SCREENCOLUMN), index(c_screens.rowCount()-1, SCREENCOLUMN), roles);
    emit screensDataChanged();
}

void Screens::setData(const Latte::Data::ScreensTable &screens)
{
    clear();

    if (screens.rowCount() > 0) {
        beginInsertRows(QModelIndex(), 0, screens.rowCount()-1);
        c_screens = screens;
        initDefaults();
        o_screens = c_screens;
        endInsertRows();

        emit screensDataChanged();
    }
}

void Screens::setSelected(const Latte::Data::ScreensTable &screens)
{
    bool changed{false};

    for(int i=0; i<screens.rowCount(); ++i) {
        int pos = c_screens.indexOf(screens[i].id);

        if (pos>=0 && screens[i].isSelected != c_screens[pos].isSelected) {
            QVector<int> roles;
            roles << Qt::CheckStateRole;

            c_screens[pos].isSelected = screens[i].isSelected;
            emit dataChanged(index(pos, SCREENCOLUMN), index(pos, SCREENCOLUMN), roles);
            changed = true;
        }
    }

    if (changed) {
        emit screensDataChanged();
    }
}

Latte::Data::ScreensTable Screens::selectedScreens()
{
    Data::ScreensTable selected;

    for(int i=0; i<c_screens.rowCount(); ++i) {
        if (!c_screens[i].isActive && c_screens[i].isSelected) {
            selected << c_screens[i];
        }
    }
    return selected;
}

Qt::ItemFlags Screens::flags(const QModelIndex &index) const
{
    const int column = index.column();
    const int row = index.row();

    auto flags = QAbstractTableModel::flags(index);

    if (c_screens[row].isRemovable) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

QVariant Screens::headerData(int section, Qt::Orientation orientation, int role) const
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
    case SCREENCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for screens", "Screens"));
        }
        break;
    default:
        break;
    };

    return QAbstractTableModel::headerData(section, orientation, role);
}

bool Screens::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int column = index.column();

    if (!c_screens.rowExists(row) || column<0 || column > SCREENCOLUMN) {
        return false;
    }

    //! specific roles to each independent cell
    switch (column) {
    case SCREENCOLUMN:
        if (role == Qt::CheckStateRole) {
            c_screens[row].isSelected = (value.toInt() > 0 ? true : false);
            emit screensDataChanged();
            return true;
        }
        break;
    };

    return false;
}

QVariant Screens::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();

    if (row >= rowCount()) {
        return QVariant{};
    }

    if (role == IDROLE) {
        return c_screens[row].id;
    } else if (role == Qt::DisplayRole) {
        QString display = "{" + c_screens[row].id + "} " + c_screens[row].name;
        return display;
    } else if (role == Qt::CheckStateRole) {
        return (c_screens[row].isSelected ? Qt::Checked : Qt::Unchecked);
    } else if (role == ISSCREENACTIVEROLE) {
        return c_screens[row].isActive;
    } else if (role == ISSELECTEDROLE) {
        return c_screens[row].isSelected;
    } else if (role == SCREENDATAROLE) {
        QVariant scrVariant;
        Latte::Data::Screen scrdata = c_screens[row];
        scrVariant.setValue<Latte::Data::Screen>(scrdata);
        return scrVariant;
    } else if (role == SORTINGROLE) {
        return c_screens[row].id.toInt();
    }

    return QVariant{};
}

}
}
}
