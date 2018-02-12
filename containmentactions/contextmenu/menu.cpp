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

#include "../../liblattedock/dock.h"

#include <QAction>
#include <QDebug>
#include <QFont>
#include <QMenu>
#include <QtDBus/QtDBus>
#include <QTimer>

#include <KActionCollection>
#include <KLocalizedString>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ServiceJob>

Menu::Menu(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
    makeActions();
}

Menu::~Menu()
{
    m_separator1->deleteLater();
    m_addWidgetsAction->deleteLater();
    m_configureAction->deleteLater();
    m_printAction->deleteLater();
    m_switchLayoutsMenu->deleteLater();
    m_layoutsAction->deleteLater();
}

void Menu::makeActions()
{
    m_separator1 = new QAction(this);
    m_separator1->setSeparator(true);

    m_printAction = new QAction(QIcon::fromTheme("edit"), "Print Message...", this);
    connect(m_printAction, &QAction::triggered, [ = ]() {
        qDebug() << "Action Trigerred !!!";
    });

    m_addWidgetsAction = new QAction(QIcon::fromTheme("add"), i18n("Add Widgets..."), this);
    m_addWidgetsAction->setStatusTip(i18n("Show Plasma Widget Explorer"));
    connect(m_addWidgetsAction, &QAction::triggered, [ = ]() {
        QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("toggleWidgetExplorer");
        }
    });

    m_configureAction = new QAction(QIcon::fromTheme("configure"), i18nc("dock/panel settings window", "Dock/Panel Settings"), this);
    m_configureAction->setShortcut(QKeySequence());
    connect(m_configureAction, &QAction::triggered, this, &Menu::requestConfiguration);

    m_switchLayoutsMenu = new QMenu;
    m_layoutsAction = m_switchLayoutsMenu->menuAction();
    m_layoutsAction->setText(i18n("Layouts"));
    m_layoutsAction->setIcon(QIcon::fromTheme("user-identity"));
    m_layoutsAction->setStatusTip(i18n("Switch to another layout"));

    connect(m_switchLayoutsMenu, &QMenu::aboutToShow, this, &Menu::populateLayouts);
    connect(m_switchLayoutsMenu, &QMenu::triggered, this, &Menu::switchToLayout);
}


void Menu::requestConfiguration()
{
    if (this->containment()) {
        emit this->containment()->configureRequested(containment());
    }
}


QList<QAction *> Menu::contextualActions()
{
    QList<QAction *> actions;
    actions << m_separator1;
    //actions << m_printAction;
    actions << m_layoutsAction;
    actions << m_addWidgetsAction;
    actions << m_configureAction;

    m_layoutsData.clear();
    QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        QDBusReply<QStringList> replyData = iface.call("contextMenuData");

        m_layoutsData = replyData.value();
    }

    if (m_layoutsData.size() > 3) {
        m_layoutsAction->setEnabled(true);
        m_layoutsAction->setVisible(true);
    } else {
        m_layoutsAction->setVisible(false);
    }

    return actions;
}

QAction *Menu::action(const QString &name)
{
    if (name == "add widgets") {
        return m_addWidgetsAction;
    } else if (name == "configure") {
        return m_configureAction;
    } else if (name == "layouts") {
        return m_layoutsAction;
    }

    return nullptr;
}

void Menu::populateLayouts()
{
    m_switchLayoutsMenu->clear();

    if (m_layoutsData.size() > 3) {
        //when there are more than 1 layouts present
        Latte::Dock::LayoutsMemoryUsage memoryUsage = static_cast<Latte::Dock::LayoutsMemoryUsage>((m_layoutsData[0]).toInt());
        QString currentName = m_layoutsData[1];

        for (int i = 2; i < m_layoutsData.size(); ++i) {
            bool isActive = m_layoutsData[i].startsWith("0") ? false : true;

            QString layout = m_layoutsData[i].right(m_layoutsData[i].length() - 2);

            QString currentText = (memoryUsage == Latte::Dock::MultipleLayouts && layout == currentName) ?
                                  (" " + i18nc("current layout", "(Current)")) : "";
            QString layoutName = layout + currentText;

            QAction *layoutAction = new QAction(layoutName, m_switchLayoutsMenu);

            layoutAction->setCheckable(true);

            if (isActive) {
                layoutAction->setChecked(true);
            } else {
                layoutAction->setChecked(false);
            }

            layoutAction->setData(layout);

            if (isActive) {
                QFont font = layoutAction->font();
                font.setBold(true);
                layoutAction->setFont(font);
            }

            m_switchLayoutsMenu->addAction(layoutAction);
        }

        m_switchLayoutsMenu->addSeparator();

        QAction *editLayoutsAction = new QAction(i18n("Configure..."), m_switchLayoutsMenu);
        editLayoutsAction->setData(QStringLiteral(" _show_latte_settings_dialog_"));
        m_switchLayoutsMenu->addAction(editLayoutsAction);
    }
}

void Menu::switchToLayout(QAction *action)
{
    const QString layout = action->data().toString();

    if (layout == " _show_latte_settings_dialog_") {
        QTimer::singleShot(400, [this]() {
            QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

            if (iface.isValid()) {
                iface.call("showSettingsWindow", (int)Latte::Dock::LayoutPage);
            }
        });
    } else {
        QTimer::singleShot(400, [this, layout]() {
            QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

            if (iface.isValid()) {
                iface.call("switchToLayout", layout);
            }
        });
    }
}

K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON(lattecontextmenu, Menu, "plasma-containmentactions-lattecontextmenu.json")

#include "menu.moc"
