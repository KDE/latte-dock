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

#include "viewsdialog.h"

// local
#include "ui_viewsdialog.h"
#include "viewshandler.h"
#include "../settingsdialog/layoutscontroller.h"

namespace Latte {
namespace Settings {
namespace Dialog {

ViewsDialog::ViewsDialog(SettingsDialog *parent, Controller::Layouts *controller)
    : GenericDialog(parent),
      m_parentDlg(parent),
      m_ui(new Ui::ViewsDialog),
      m_layoutsController(controller),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog").group("ViewsDialog"))
{
    loadConfig();

    //! first we need to setup the ui
    m_ui->setupUi(this);
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ViewsHandler(this);

    connect(m_handler, &Handler::ViewsHandler::currentLayoutChanged, this, &ViewsDialog::updateApplyButtonsState);
    connect(m_handler, &Handler::ViewsHandler::dataChanged, this, &ViewsDialog::updateApplyButtonsState);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &ViewsDialog::onOk);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &ViewsDialog::onCancel);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &ViewsDialog::onReset);

    resize(m_windowSize);
    updateApplyButtonsState();    
}

ViewsDialog::~ViewsDialog()
{
    saveConfig();
}

Controller::Layouts *ViewsDialog::layoutsController() const
{
    return m_layoutsController;
}

Ui::ViewsDialog *ViewsDialog::ui() const
{
    return m_ui;
}

Latte::Corona *ViewsDialog::corona() const
{
    return m_parentDlg->corona();
}

void ViewsDialog::updateApplyButtonsState()
{
  /*  if (m_handler->hasChangedData()) {
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
    } else {
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    }*/
}

void ViewsDialog::accept()
{
    qDebug() << Q_FUNC_INFO;
}

void ViewsDialog::onOk()
{
    qDebug() << Q_FUNC_INFO;
//    m_handler->save();
    close();
}

void ViewsDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void ViewsDialog::onReset()
{
    qDebug() << Q_FUNC_INFO;
   // m_handler->reset();
}

void ViewsDialog::loadConfig()
{
    m_windowSize = m_storage.readEntry("windowSize", QSize(775, 500));
}

void ViewsDialog::saveConfig()
{
    m_storage.writeEntry("windowSize", size());
}

}
}
}
