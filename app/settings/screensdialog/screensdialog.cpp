/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "screensdialog.h"

// local
#include "ui_screensdialog.h"
#include "screenshandler.h"
#include "../settingsdialog/layoutscontroller.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"

namespace Latte {
namespace Settings {
namespace Dialog {

ScreensDialog::ScreensDialog(SettingsDialog *parent, Controller::Layouts *controller)
    : GenericDialog(parent),
      m_ui(new Ui::ScreensDialog),
      m_layoutsController(controller)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_corona = parent->corona();

    init();
    initRemoveNowButton();
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ScreensHandler(this);
    initSignals();
}

ScreensDialog::~ScreensDialog()
{
}

Latte::Corona *ScreensDialog::corona() const
{
    return m_corona;
}

Ui::ScreensDialog *ScreensDialog::ui() const
{
    return m_ui;
}

Controller::Layouts *ScreensDialog::layoutsController() const
{
    return m_layoutsController;
}

void ScreensDialog::init()
{
    //! first we need to setup the ui
    m_ui->setupUi(this);
    initButtons();
}

void ScreensDialog::initButtons()
{
    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &ScreensDialog::onCancel);
}

void ScreensDialog::initRemoveNowButton()
{
    m_removeNowButton = new QPushButton(this);
    m_removeNowButton->setText(i18nc("remove selected screens", "Remove Now"));
    m_removeNowButton->setIcon(QIcon::fromTheme("edit-delete"));
    m_removeNowButton->setToolTip(i18n("Remove selected screen references"));

    m_ui->buttonBox->addButton(m_removeNowButton, QDialogButtonBox::AcceptRole);
}

void ScreensDialog::initSignals()
{
    //connect(m_handler, &Handler::ExportTemplateHandler::dataChanged, this, &ScreensDialog::onDataChanged);
    //connect(m_handler, &Handler::ExportTemplateHandler::exportSucceeded, this, &ScreensDialog::onExportSucceeded);
}

QPushButton *ScreensDialog::removeNowButton() const
{
    return m_removeNowButton;
}

void ScreensDialog::onDataChanged()
{
}


void ScreensDialog::accept()
{
    qDebug() << Q_FUNC_INFO;
    //close();
}

void ScreensDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void ScreensDialog::onReset()
{
    qDebug() << Q_FUNC_INFO;
}

}
}
}
