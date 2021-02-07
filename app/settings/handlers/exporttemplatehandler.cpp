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
    m_appletsWithNoPersonalData = {
        "org.kde.latte.separator",
        "org.kde.latte.plasmoid",
        "org.kde.latte.windowtitle",
        "org.kde.latte.windowbuttons",
        "org.kde.latte.windowappmenu"
    };

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

    //! Applets Model
    m_appletsProxyModel = new QSortFilterProxyModel(this);
    m_appletsProxyModel->setSourceModel(m_appletsModel);
    m_appletsProxyModel->setSortRole(Model::Applets::SORTINGROLE);
    m_appletsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_appletsProxyModel->sort(Model::Applets::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->appletsTable->setModel(m_appletsProxyModel);

    //! Reset
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &ExportTemplateHandler::onReset);
}

void ExportTemplateHandler::initDefaults()
{
    for(int i=0; i<c_data.rowCount(); ++i) {
        c_data[i].isSelected = m_appletsWithNoPersonalData.contains(c_data[i].id);
    }
}

void ExportTemplateHandler::loadLayoutApplets(const QString &layoutName, const QString &layoutId)
{
    c_data = Latte::Layouts::Storage::self()->plugins(layoutId);

    initDefaults();

    o_data = c_data;

    m_appletsModel->setData(c_data);
    m_parentDialog->setWindowTitle(i18n("Export Layout Template"));
}

void ExportTemplateHandler::loadViewApplets(Latte::View *view)
{
    c_data = Latte::Layouts::Storage::self()->plugins(view->layout(), view->containment()->id());
    o_data = c_data;

    m_appletsModel->setData(c_data);
    m_parentDialog->setWindowTitle(i18n("Export View Template"));
}

void ExportTemplateHandler::onReset()
{
    m_appletsModel->setSelected(o_data);
    c_data = o_data;
}

bool ExportTemplateHandler::dataAreChanged() const
{
    return o_data != c_data;
}

bool ExportTemplateHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void ExportTemplateHandler::reset()
{
    c_data = o_data;
}

void ExportTemplateHandler::resetDefaults()
{
    //do nothing
}

void ExportTemplateHandler::save()
{
    //do nothing
}

}
}
}
