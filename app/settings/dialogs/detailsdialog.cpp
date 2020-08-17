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

#include "detailsdialog.h"

// local
#include "ui_detailsdialog.h"
#include "../controllers/layoutscontroller.h"
#include "../handlers/detailshandler.h"


namespace Latte {
namespace Settings {
namespace Dialog {

DetailsDialog::DetailsDialog(SettingsDialog *parent, Controller::Layouts *controller)
    : GenericDialog(parent),
      m_parentDlg(parent),
      m_ui(new Ui::DetailsDialog),
      m_layoutsController(controller)
{
    //! first we need to setup the ui
    m_ui->setupUi(this);
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::DetailsHandler(this);

    connect(m_handler, &Handler::DetailsHandler::currentLayoutChanged, this, &DetailsDialog::updateApplyButtonsState);
    connect(m_handler, &Handler::DetailsHandler::dataChanged, this, &DetailsDialog::updateApplyButtonsState);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &DetailsDialog::on_ok);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &DetailsDialog::on_cancel);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &DetailsDialog::on_reset);

    updateApplyButtonsState();
}

DetailsDialog::~DetailsDialog()
{
}

Controller::Layouts *DetailsDialog::layoutsController() const
{
    return m_layoutsController;
}

Ui::DetailsDialog *DetailsDialog::ui() const
{
    return m_ui;
}

Latte::Corona *DetailsDialog::corona() const
{
    return m_parentDlg->corona();
}

void DetailsDialog::updateApplyButtonsState()
{
    if (m_handler->dataAreChanged()) {
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
    } else {
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    }
}

void DetailsDialog::accept()
{
    qDebug() << Q_FUNC_INFO;
}

void DetailsDialog::on_ok()
{
    qDebug() << Q_FUNC_INFO;
    m_layoutsController->setLayoutProperties(m_handler->currentData());
    close();
}

void DetailsDialog::on_cancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void DetailsDialog::on_reset()
{
    qDebug() << Q_FUNC_INFO;
    m_handler->reset();
}

}
}
}
