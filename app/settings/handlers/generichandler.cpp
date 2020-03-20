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

#include "generichandler.h"



namespace Latte {
namespace Settings {
namespace Handler {

Generic::Generic(QObject *parent)
    : QObject(parent)
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

}
}
}
