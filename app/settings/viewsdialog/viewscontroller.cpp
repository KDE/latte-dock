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
#include "delegates/singleoptiondelegate.h"
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
    loadConfig();
    m_proxyModel->setSourceModel(m_model);

    connect(m_model, &QAbstractItemModel::dataChanged, this, &Views::dataChanged);
    connect(m_model, &Model::Views::rowsInserted, this, &Views::dataChanged);
    connect(m_model, &Model::Views::rowsRemoved, this, &Views::dataChanged);

    connect(m_handler, &Handler::ViewsHandler::currentLayoutChanged, this, &Views::onCurrentLayoutChanged);

    init();
}

Views::~Views()
{
    saveConfig();
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

    m_view->setItemDelegateForColumn(Model::Views::SCREENCOLUMN, new Settings::View::Delegate::SingleOption(this));
    m_view->setItemDelegateForColumn(Model::Views::EDGECOLUMN, new Settings::View::Delegate::SingleOption(this));
    m_view->setItemDelegateForColumn(Model::Views::ALIGNMENTCOLUMN, new Settings::View::Delegate::SingleOption(this));

    applyColumnWidths();

    connect(m_view, &QObject::destroyed, this, &Views::storeColumnWidths);

    connect(m_view->horizontalHeader(), &QObject::destroyed, this, [&]() {
        m_viewSortColumn = m_view->horizontalHeader()->sortIndicatorSection();
        m_viewSortOrder = m_view->horizontalHeader()->sortIndicatorOrder();
    });
}

void Views::reset()
{
    m_model->resetData();
}

bool Views::hasChangedData() const
{
    return m_model->hasChangedData();
}

bool Views::hasSelectedView() const
{
    int selectedRow = m_view->currentIndex().row();

    return (selectedRow >= 0);
}

const Latte::Data::View Views::appendViewFromViewTemplate(const Data::View &view)
{
    Data::View newview = view;
    newview.name = uniqueViewName(view.name);
    m_model->appendTemporaryView(newview);
    return newview;
}

void Views::selectRow(const QString &id)
{
  //  m_view->selectRow(rowForId(id));
}

void Views::onCurrentLayoutChanged()
{   
    Data::Layout layout = m_handler->currentData();
    m_model->setOriginalData(layout.views);
}


QString Views::uniqueViewName(QString name)
{
    if (name.isEmpty()) {
            return name;
    }

    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (m_model->containsCurrentName(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (m_model->containsCurrentName(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}

void Views::applyColumnWidths()
{
    m_view->horizontalHeader()->setSectionResizeMode(Model::Views::NAMECOLUMN, QHeaderView::Stretch);

    if (m_viewColumnWidths.count()<(Model::Views::columnCount()-1)) {
        return;
    }

    m_view->setColumnWidth(Model::Views::IDCOLUMN, m_viewColumnWidths[0].toInt());
    m_view->setColumnWidth(Model::Views::SCREENCOLUMN, m_viewColumnWidths[1].toInt());
    m_view->setColumnWidth(Model::Views::EDGECOLUMN, m_viewColumnWidths[2].toInt());
    m_view->setColumnWidth(Model::Views::ALIGNMENTCOLUMN, m_viewColumnWidths[3].toInt());
    m_view->setColumnWidth(Model::Views::SUBCONTAINMENTSCOLUMN, m_viewColumnWidths[4].toInt());
}

void Views::storeColumnWidths()
{
    if (m_viewColumnWidths.isEmpty() || (m_viewColumnWidths.count()<Model::Views::columnCount()-1)) {
        m_viewColumnWidths.clear();
        for (int i=0; i<Model::Views::columnCount(); ++i) {
            m_viewColumnWidths << "";
        }
    }

    m_viewColumnWidths[0] = QString::number(m_view->columnWidth(Model::Views::IDCOLUMN));
    m_viewColumnWidths[1] = QString::number(m_view->columnWidth(Model::Views::SCREENCOLUMN));
    m_viewColumnWidths[2] = QString::number(m_view->columnWidth(Model::Views::EDGECOLUMN));
    m_viewColumnWidths[3] = QString::number(m_view->columnWidth(Model::Views::ALIGNMENTCOLUMN));
    m_viewColumnWidths[4] = QString::number(m_view->columnWidth(Model::Views::SUBCONTAINMENTSCOLUMN));
}

void Views::loadConfig()
{
    m_viewColumnWidths = m_storage.readEntry("columnWidths", QStringList());
    m_viewSortColumn = m_storage.readEntry("sortColumn", (int)Model::Views::SCREENCOLUMN);
    m_viewSortOrder = static_cast<Qt::SortOrder>(m_storage.readEntry("sortOrder", (int)Qt::AscendingOrder));
}

void Views::saveConfig()
{
    m_storage.writeEntry("columnWidths", m_viewColumnWidths);
    m_storage.writeEntry("sortColumn", m_viewSortColumn);
    m_storage.writeEntry("sortOrder", (int)m_viewSortOrder);
}

}
}
}
