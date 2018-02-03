/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "menu.h"

#include <QAction>
#include <QDebug>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ServiceJob>

Menu::Menu(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

Menu::~Menu()
{
    qDeleteAll(m_actions);
}

void Menu::makeMenu()
{
    qDeleteAll(m_actions);
    m_actions.clear();

    QAction *action = new QAction(QIcon::fromTheme("edit"), "Print Message...", this);
    connect(action, &QAction::triggered, [ = ]() {
        qDebug() << "Action Trigerred !!!";
    });

    m_actions << action;
    /*foreach (const QString &id, m_consumer.activities(KActivities::Info::Running)) {
        KActivities::Info info(id);
        QAction *action = new QAction(QIcon::fromTheme(info.icon()), info.name(), this);
        action->setData(id);

        if (id == m_consumer.currentActivity()) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }

        connect(action, &QAction::triggered, [ = ]() {
            switchTo(action);
        });

        m_actions << action;
    }*/
}



QList<QAction *> Menu::contextualActions()
{
    makeMenu();

    return m_actions;
}

K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON(lattecontextmenu, Menu, "plasma-containmentactions-lattecontextmenu.json")

#include "menu.moc"
