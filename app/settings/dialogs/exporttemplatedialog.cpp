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

#include "exporttemplatedialog.h"

// local
#include "ui_exporttemplatedialog.h"
#include "../controllers/layoutscontroller.h"
#include "../handlers/exporttemplatehandler.h"

namespace Latte {
namespace Settings {
namespace Dialog {

ExportTemplateDialog::ExportTemplateDialog(SettingsDialog *parent, Controller::Layouts *controller)
    : GenericDialog(parent),
      m_parentDlg(parent),
      m_ui(new Ui::ExportTemplateDialog),
      m_layoutsController(controller)
{
    //! first we need to setup the ui
    m_ui->setupUi(this);
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ExportTemplateHandler(this);

    m_ui->appletsTable->horizontalHeader()->setStretchLastSection(true);
    m_ui->appletsTable->verticalHeader()->setVisible(false);
}

ExportTemplateDialog::~ExportTemplateDialog()
{
}

Ui::ExportTemplateDialog *ExportTemplateDialog::ui() const
{
    return m_ui;
}

Latte::Corona *ExportTemplateDialog::corona() const
{
    return m_parentDlg->corona();
}

Controller::Layouts *ExportTemplateDialog::layoutsController() const
{
    return m_layoutsController;
}

void ExportTemplateDialog::accept()
{
    qDebug() << Q_FUNC_INFO;
}

void ExportTemplateDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

}
}
}
