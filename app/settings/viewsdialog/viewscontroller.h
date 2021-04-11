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

#ifndef VIEWSCONTROLLER_H
#define VIEWSCONTROLLER_H

// local
#include <coretypes.h>
#include "viewsmodel.h"
#include "../../lattecorona.h"
#include "../../data/viewdata.h"
#include "../../data/viewstable.h"

// Qt
#include <QAbstractItemModel>
#include <QHash>
#include <QSortFilterProxyModel>
#include <QTableView>


namespace Latte {
class Corona;
class ViewsDialog;

namespace Settings {
namespace Handler {
class ViewsHandler;
}
}
}

namespace Latte {
namespace Settings {
namespace Controller {

class Views : public QObject
{
    Q_OBJECT

public:
    explicit Views(Settings::Handler::ViewsHandler *parent);
    ~Views();

    QAbstractItemModel *proxyModel() const;
    QAbstractItemModel *baseModel() const;
    QTableView *view() const;

    bool hasChangedData() const;

    void sortByColumn(int column, Qt::SortOrder order);

    bool hasSelectedView() const;
    const Latte::Data::View selectedViewCurrentData() const;
    const Latte::Data::View selectedViewOriginalData() const;

    const Latte::Data::View appendViewFromViewTemplate(const Data::View &view);

    void selectRow(const QString &id);

    //! actions
    void reset();
    void save();
    void removeSelected();

signals:
    void dataChanged();

private:
    void init();

    int rowForId(QString id) const;
    QString uniqueViewName(QString name);

private slots:
    void loadConfig();
    void saveConfig();
    void storeColumnWidths();
    void applyColumnWidths();

    void onCurrentLayoutChanged();

private:
    Settings::Handler::ViewsHandler *m_handler{nullptr};

    QTableView *m_view{nullptr};

    //! layoutsView ui settings
    int m_viewSortColumn{Model::Views::SCREENCOLUMN};
    Qt::SortOrder m_viewSortOrder;
    QStringList m_viewColumnWidths;

    KConfigGroup m_storage;

    //! current data
    Model::Views *m_model{nullptr};
    QSortFilterProxyModel *m_proxyModel{nullptr};
};

}
}
}

#endif
