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
#include "../data/layoutdata.h"
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
    enum Columns
    {
        IDCOLUMN = 0,
        HIDDENTEXTCOLUMN,
        BACKGROUNDCOLUMN,
        NAMECOLUMN,
        MENUCOLUMN,
        BORDERSCOLUMN,
        ACTIVITYCOLUMN,
        SHAREDCOLUMN
    };

    enum LayoutUserRoles
    {
        LAYOUTISACTIVEROLE = Qt::UserRole + 1,
        LAYOUTISLOCKEDROLE,
        LAYOUTISSHAREDROLE,
        LAYOUTNAMEWASEDITED,
        INMULTIPLELAYOUTSMODE,
        ACTIVITIESROLE,
        RUNNINGACTIVITIESROLE,
        SHARESROLE,
        ACTIVESHARESROLE
    };

    explicit Layouts(QObject *parent = nullptr);

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);

    QString idForOriginalName(const QString &name);
    QString idForEditedName(const QString &name);

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    const Data::Layout &at(const int &row);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;


    void appendLayout(const Settings::Data::Layout &layout);
    void clear();
    void removeLayout(const QString &id);
    void remove(const int &row);

    const Data::LayoutsTable &currentData();
    void setCurrentData(Data::LayoutsTable &data);

private:
    bool m_inMultipleMode{false};
    Data::LayoutsTable m_layoutsTable;
};

}
}
}

#endif
