/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "detailsdialog.h"

// local
#include "ui_detailsdialog.h"
#include "detailshandler.h"
#include "../settingsdialog/layoutscontroller.h"

namespace Latte {
namespace Settings {
namespace Dialog {

DetailsDialog::DetailsDialog(SettingsDialog *parent, Controller::Layouts *controller)
    : GenericDialog(parent),
      m_parentDlg(parent),
      m_ui(new Ui::DetailsDialog),
      m_layoutsController(controller),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog").group("DetailsDialog"))
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    loadConfig();

    //! first we need to setup the ui
    m_ui->setupUi(this);
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::DetailsHandler(this);

    connect(m_handler, &Handler::DetailsHandler::currentLayoutChanged, this, &DetailsDialog::updateApplyButtonsState);
    connect(m_handler, &Handler::DetailsHandler::dataChanged, this, &DetailsDialog::updateApplyButtonsState);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &DetailsDialog::onOk);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &DetailsDialog::onCancel);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &DetailsDialog::onReset);

    resize(m_windowSize);
    updateApplyButtonsState();
}

DetailsDialog::~DetailsDialog()
{
    saveConfig();
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
    if (m_handler->hasChangedData()) {
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

void DetailsDialog::onOk()
{
    qDebug() << Q_FUNC_INFO;
    m_handler->save();
    close();
}

void DetailsDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void DetailsDialog::onReset()
{
    qDebug() << Q_FUNC_INFO;
    m_handler->reset();
}

void DetailsDialog::loadConfig()
{
    m_windowSize = m_storage.readEntry("windowSize", QSize(560, 640));
}

void DetailsDialog::saveConfig()
{
    m_storage.writeEntry("windowSize", size());
}

}
}
}
