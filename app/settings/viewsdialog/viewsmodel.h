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
#include "../../data/genericbasictable.h"
#include "../../data/screendata.h"
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
        IDCOLUMN = 0,
        NAMECOLUMN,
        SCREENCOLUMN,
        EDGECOLUMN,
        ALIGNMENTCOLUMN,
        SUBCONTAINMENTSCOLUMN,
        LASTCOLUMN
    };

    enum Roles
    {
        IDROLE = Qt::UserRole + 1,
        NAMEROLE,
        ISACTIVEROLE,
        CHOICESROLE,
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

    bool hasChangedData() const;
    bool containsCurrentName(const QString &name) const;

    //! all original data will become also current
    void resetData();

    void appendTemporaryView(const Latte::Data::View &view);
    void removeView(const QString &id);

    int rowCount() const;
    static int columnCount();
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    const Latte::Data::ViewsTable &currentViewsData();
    const Latte::Data::ViewsTable &originalViewsData();

    void setOriginalData(Latte::Data::ViewsTable &data);

signals:
    void rowsInserted();
    void rowsRemoved();

private slots:
    void clear();

    void initEdges();
    void initAlignments();
    void populateScreens();

private:
    bool isVertical(const Plasma::Types::Location &location) const;

private:
    Latte::Data::ViewsTable m_viewsTable;
    Latte::Data::ViewsTable o_viewsTable;

    Latte::Corona *m_corona{nullptr};

    QVariant s_edges;
    QVariant s_horizontalAlignments;
    QVariant s_verticalAlignments;
    Latte::Data::ScreensTable s_screens;
};

}
}
}

#endif
