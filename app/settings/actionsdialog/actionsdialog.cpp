/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionsdialog.h"

// local
#include "ui_actionsdialog.h"

// Qt
#include <QDebug>

namespace Latte {
namespace Settings {
namespace Dialog {

ActionsDialog::ActionsDialog(QDialog *parent)
    : GenericDialog(parent),
      m_ui(new Ui::ActionsDialog)/*this is necessary, in order to create the ui*/
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    init();
}

ActionsDialog::~ActionsDialog()
{
}

Ui::ActionsDialog *ActionsDialog::ui() const
{
    return m_ui;
}

void ActionsDialog::init()
{
    m_ui->setupUi(this);
}

void ActionsDialog::accept()
{
    qDebug() << Q_FUNC_INFO;
    //close();
}

void ActionsDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void ActionsDialog::onReset()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

}
}
}
