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

#include "exporttemplatehandler.h"

// local
#include "ui_exporttemplatedialog.h"
#include "../controllers/layoutscontroller.h"
#include "../delegates/normalcheckboxdelegate.h"
#include "../dialogs/exporttemplatedialog.h"
#include "../models/appletsmodel.h"
#include "../../data/appletdata.h"
#include "../../layout/genericlayout.h"
#include "../../layouts/storage.h"
#include "../../view/view.h"

//! KDE
#include <KLocalizedString>

//! Plasma
#include <Plasma/Containment>

namespace Latte {
namespace Settings {
namespace Handler {

ExportTemplateHandler::ExportTemplateHandler(Dialog::ExportTemplateDialog *parentDialog)
    : Generic(parentDialog),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui()),
      m_appletsModel(new Model::Applets(this))
{
    init();
}

ExportTemplateHandler::ExportTemplateHandler(Dialog::ExportTemplateDialog *parentDialog, const QString &layoutName, const QString &layoutId)
    : ExportTemplateHandler(parentDialog)
{
    loadLayoutApplets(layoutName, layoutId);
}

ExportTemplateHandler::ExportTemplateHandler(Dialog::ExportTemplateDialog *parentDialog, Latte::View *view)
    : ExportTemplateHandler(parentDialog)
{
    init();
}

ExportTemplateHandler::~ExportTemplateHandler()
{
}


void ExportTemplateHandler::init()
{
    m_ui->appletsTable->horizontalHeader()->setStretchLastSection(true);
    m_ui->appletsTable->verticalHeader()->setVisible(false);

    m_ui->appletsTable->setItemDelegateForColumn(Model::Applets::NAMECOLUMN, new Settings::Applets::Delegate::NormalCheckBox(this));

    //! Applets Model
    m_appletsProxyModel = new QSortFilterProxyModel(this);
    m_appletsProxyModel->setSourceModel(m_appletsModel);
    m_appletsProxyModel->setSortRole(Model::Applets::SORTINGROLE);
    m_appletsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  //  m_appletsProxyModel->sort(Model::Applets::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->appletsTable->setModel(m_appletsProxyModel);

    //! Buttons
    connect(m_ui->deselectAllBtn, &QPushButton::clicked, this, &ExportTemplateHandler::onDeselectAll);
    connect(m_ui->selectAllBtn, &QPushButton::clicked, this, &ExportTemplateHandler::onSelectAll);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ExportTemplateHandler::onReset);
}

void ExportTemplateHandler::loadLayoutApplets(const QString &layoutName, const QString &layoutId)
{
    Data::AppletsTable c_data = Latte::Layouts::Storage::self()->plugins(layoutId);
    m_appletsModel->setData(c_data);
    m_parentDialog->setWindowTitle(i18n("Export Layout Template"));
}

void ExportTemplateHandler::loadViewApplets(Latte::View *view)
{
    Data::AppletsTable c_data = Latte::Layouts::Storage::self()->plugins(view->layout(), view->containment()->id());
    m_appletsModel->setData(c_data);
    m_parentDialog->setWindowTitle(i18n("Export View Template"));
}

void ExportTemplateHandler::onReset()
{
    m_appletsModel->reset();
}

void ExportTemplateHandler::onSelectAll()
{
    m_appletsModel->selectAll();

}

void ExportTemplateHandler::onDeselectAll()
{
    m_appletsModel->deselectAll();
}

bool ExportTemplateHandler::dataAreChanged() const
{
    return m_appletsModel->dataAreChanged();
}

bool ExportTemplateHandler::inDefaultValues() const
{
    return !dataAreChanged();
}


void ExportTemplateHandler::reset()
{
    m_appletsModel->reset();
}

void ExportTemplateHandler::resetDefaults()
{
    reset();
}

void ExportTemplateHandler::save()
{
    //do nothing
}

}
}
}
