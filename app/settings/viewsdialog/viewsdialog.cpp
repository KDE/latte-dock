/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    setAttribute(Qt::WA_DeleteOnClose, true);
    loadConfig();

    //! first we need to setup the ui
    m_ui->setupUi(this);
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ViewsHandler(this);

    //! Button Group
    m_applyNowBtn = new QPushButton(QIcon::fromTheme("dialog-yes"), i18n("Apply Now"), m_ui->buttonBox);
    m_applyNowBtn->setToolTip(i18n("Apply all dock, panels changes now"));
    m_ui->buttonBox->addButton(m_applyNowBtn, QDialogButtonBox::ApplyRole);

    //! Signals/Slots
    connect(m_handler, &Handler::ViewsHandler::currentLayoutChanged, this, &ViewsDialog::updateApplyButtonsState);
    connect(m_handler, &Handler::ViewsHandler::dataChanged, this, &ViewsDialog::updateApplyButtonsState);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &ViewsDialog::onCancel);

    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &ViewsDialog::onReset);

    connect(m_applyNowBtn, &QPushButton::clicked, this, &ViewsDialog::onApply);

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
    bool changed = m_handler->hasChangedData();

    m_applyNowBtn->setEnabled(changed);
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(changed);
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

void ViewsDialog::onApply()
{
    qDebug() << Q_FUNC_INFO;
    m_handler->save();
}

void ViewsDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void ViewsDialog::onReset()
{
    qDebug() << Q_FUNC_INFO;
    m_handler->reset();
}

void ViewsDialog::loadConfig()
{
    m_windowSize = m_storage.readEntry("windowSize", QSize(980, 630));
}

void ViewsDialog::saveConfig()
{
    m_storage.writeEntry("windowSize", size());
}

}
}
}
