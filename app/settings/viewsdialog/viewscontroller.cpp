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

#include "viewscontroller.h"

// local
#include "ui_viewsdialog.h"
#include "viewsdialog.h"
#include "viewshandler.h"
#include "viewsmodel.h"
#include "../generic/generictools.h"

// Qt
#include <QHeaderView>
#include <QItemSelection>

// KDE
#include <KMessageWidget>


namespace Latte {
namespace Settings {
namespace Controller {


Views::Views(Settings::Handler::ViewsHandler *parent)
    : QObject(parent),
      m_handler(parent),
      m_model(new Model::Views(this, m_handler->corona())),
      m_proxyModel(new QSortFilterProxyModel(this)),
      m_view(m_handler->ui()->viewsTable),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog").group("ViewsDialog"))
{
   // loadConfig();
    m_proxyModel->setSourceModel(m_model);

    connect(m_model, &QAbstractItemModel::dataChanged, this, &Views::dataChanged);
    connect(m_model, &Model::Views::rowsInserted, this, &Views::dataChanged);
    connect(m_model, &Model::Views::rowsRemoved, this, &Views::dataChanged);

    init();
}

Views::~Views()
{
   // saveConfig();
}

QAbstractItemModel *Views::proxyModel() const
{
    return m_proxyModel;
}

QAbstractItemModel *Views::baseModel() const
{
    return m_model;
}

QTableView *Views::view() const
{
    return m_view;
}

void Views::init()
{
    m_view->setModel(m_proxyModel);
    //m_view->setHorizontalHeader(m_headerView);
    m_view->verticalHeader()->setVisible(false);
    m_view->setSortingEnabled(true);

    m_proxyModel->setSortRole(Model::Views::SORTINGROLE);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_view->sortByColumn(m_viewSortColumn, m_viewSortOrder);

    //m_view->setItemDelegateForColumn(Model::Layouts::NAMECOLUMN, new Settings::Layout::Delegate::LayoutName(this));
    //m_view->setItemDelegateForColumn(Model::Layouts::BACKGROUNDCOLUMN, new Settings::Layout::Delegate::BackgroundDelegate(this));
    //m_view->setItemDelegateForColumn(Model::Layouts::MENUCOLUMN, new Settings::Layout::Delegate::CheckBox(this));
    //m_view->setItemDelegateForColumn(Model::Layouts::BORDERSCOLUMN, new Settings::Layout::Delegate::CheckBox(this));
    //m_view->setItemDelegateForColumn(Model::Layouts::ACTIVITYCOLUMN, new Settings::Layout::Delegate::Activities(this));
}

bool Views::hasChangedData() const
{
    return true;// m_model->hasChangedData();
}

bool Views::hasSelectedView() const
{
    int selectedRow = m_view->currentIndex().row();

    return (selectedRow >= 0);
}

void Views::selectRow(const QString &id)
{
  //  m_view->selectRow(rowForId(id));
}

}
}
}
