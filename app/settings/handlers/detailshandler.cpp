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

#include "detailshandler.h"

// local
#include "ui_detailsdialog.h"
#include "../controllers/layoutscontroller.h"
#include "../data/layoutdata.h"
#include "../data/layoutstable.h"
#include "../dialogs/detailsdialog.h"
#include "../models/layoutsmodel.h"

namespace Latte {
namespace Settings {
namespace Handler {

DetailsHandler::DetailsHandler(Dialog::DetailsDialog *parentDialog)
    : Generic(parentDialog),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui())
{
    init();

    //! create it after initializing
    m_infoHandler = new DetailsInfoHandler(parentDialog, this);   
}

DetailsHandler::~DetailsHandler()
{
}

void DetailsHandler::init()
{
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_parentDialog->layoutsController()->baseModel());
    m_proxyModel->setSortRole(Model::Layouts::SORTINGROLE);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->sort(Model::Layouts::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->layoutsCmb->setModel(m_proxyModel);
    m_ui->layoutsCmb->setModelColumn(Model::Layouts::NAMECOLUMN);

    reload();

    //! connect combobox after the selected layout has been loaded
    connect(m_ui->layoutsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DetailsHandler::on_currentIndexChanged);
}

void DetailsHandler::reload()
{
    o_data = m_parentDialog->layoutsController()->selectedLayoutCurrentData();
    c_data = o_data;

    m_ui->layoutsCmb->setCurrentText(o_data.name);
}

Data::Layout DetailsHandler::currentData() const
{
    return c_data;
}

bool DetailsHandler::dataAreChanged() const
{
    return o_data != c_data;
}

bool DetailsHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void DetailsHandler::reset()
{
    c_data = o_data;
    emit currentLayoutChanged();
}

void DetailsHandler::resetDefaults()
{
    //do nothing
}

void DetailsHandler::save()
{
}

void DetailsHandler::on_currentIndexChanged(int row)
{
    QString layoutId = m_proxyModel->data(m_proxyModel->index(row, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
    m_parentDialog->layoutsController()->selectRow(layoutId);
    reload();

    emit currentLayoutChanged();
}

void DetailsHandler::setIsShownInMenu(bool inMenu)
{
    c_data.isShownInMenu = inMenu;
}

void DetailsHandler::setHasDisabledBorders(bool disabled)
{
    c_data.hasDisabledBorders = disabled;
}

}
}
}
