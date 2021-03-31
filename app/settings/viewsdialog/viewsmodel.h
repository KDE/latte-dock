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

#ifndef VIEWSMODEL_H
#define VIEWSMODEL_H

// local
#include "../../lattecorona.h"
#include "../../data/viewdata.h"
#include "../../data/viewstable.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>

namespace Latte {
namespace Settings {
namespace Model {

class Views : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns
    {
        SCREENCOLUMN = 0,
        EDGECOLUMN,
        ALIGNMENTCOLUMN,
        IDCOLUMN
    };

    enum Roles
    {
        IDROLE = Qt::UserRole + 1,
        ISACTIVEROLE,
        SCREENSLISTROLE,
        EDGESLISTROLE,
        SUBCONTAINMENTSIDSROLE,
        SORTINGROLE
    };

    enum SortingPriority
    {
        NORMALPRIORITY = 8000,
        MEDIUMPRIORITY = 6000,
        HIGHPRIORITY = 4000,
        HIGHESTPRIORITY = 2000
    };

    explicit Views(QObject *parent, Latte::Corona *corona);
    ~Views();

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const Latte::Data::ViewsTable &currentViewsData();
    const Latte::Data::ViewsTable &originalViewsData();

    void setOriginalData(Latte::Data::ViewsTable &data);

signals:
    void rowsInserted();
    void rowsRemoved();

private slots:
    void clear();

private:
    Latte::Data::ViewsTable m_viewsTable;
    Latte::Data::ViewsTable o_viewsTable;

    Latte::Corona *m_corona{nullptr};
};

}
}
}

#endif
