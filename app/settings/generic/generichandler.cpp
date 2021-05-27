/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "generichandler.h"

// local
#include "genericdialog.h"

namespace Latte {
namespace Settings {
namespace Handler {

Generic::Generic(Dialog::GenericDialog *parent)
    : QObject(parent),
      m_dialog(parent)
{
}

Generic::Generic(Dialog::GenericDialog *parentDialog, QObject *parent)
    : QObject(parent),
      m_dialog(parentDialog)
{
}


void Generic::setTwinProperty(QAction *action, const QString &property, QVariant value)
{
    if (!m_twinActions.contains(action)) {
        return;
    }

    if (property == TWINVISIBLE) {
        action->setVisible(value.toBool());
        m_twinActions[action]->setVisible(value.toBool());
    } else if (property == TWINENABLED) {
        action->setEnabled(value.toBool());
        m_twinActions[action]->setEnabled(value.toBool());
    } else if (property == TWINCHECKED) {
        action->setChecked(value.toBool());
        m_twinActions[action]->setChecked(value.toBool());
    }
}

void Generic::connectActionWithButton(QPushButton *button, QAction *action)
{
    button->setText(action->text());
    button->setToolTip(action->toolTip());
    button->setWhatsThis(action->whatsThis());
    button->setIcon(action->icon());
    button->setCheckable(action->isCheckable());
    button->setChecked(action->isChecked());

    m_twinActions[action] = button;

    connect(button, &QPushButton::clicked, action, &QAction::trigger);
}

void Generic::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const bool &isPersistent, QList<QAction *> actions)
{
    m_dialog->showInlineMessage(msg, type, isPersistent, actions);
}

}
}
}
