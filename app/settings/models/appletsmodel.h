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

#ifndef APPLETSMODEL_H
#define APPLETSMODEL_H

// local
#include "../../data/appletdata.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>

namespace Latte {
namespace Settings {
namespace Model {

class Applets : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum AppletsRoles
    {
        IDROLE = Qt::UserRole + 1,
        NAMEROLE,
        ICONROLE,
        SELECTEDROLE,
        SORTINGROLE,
        DESCRIPTIONROLE
    };

    enum Columns
    {
        NAMECOLUMN = 0
    };

    explicit Applets(QObject *parent);
    ~Applets();

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    int row(const QString &id);

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setData(const Latte::Data::AppletsTable &applets);
    void setSelected(const Latte::Data::AppletsTable &applets);

private:
    void clear();

private:
    Latte::Data::AppletsTable m_appletsTable;

};

}
}
}

#endif
