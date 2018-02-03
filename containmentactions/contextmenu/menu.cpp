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

#include "../../app/dockcorona.h"
#include "../../app/layoutmanager.h"
#include "../../liblattedock/dock.h"

#include <QAction>
#include <QDebug>
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

    auto *dockCorona = qobject_cast<Latte::DockCorona *>(containment()->corona());

    if (dockCorona && dockCorona->layoutManager()->menuLayouts().count() > 1) {
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

    auto *dockCorona = qobject_cast<Latte::DockCorona *>(containment()->corona());

    if (dockCorona && dockCorona->layoutManager()->menuLayouts().count() > 1) {
        QStringList activeLayouts = dockCorona->layoutManager()->activeLayoutsNames();
        Latte::Dock::LayoutsMemoryUsage memoryUsage = dockCorona->layoutManager()->memoryUsage();
        QString currentName = dockCorona->layoutManager()->currentLayoutName();

        foreach (auto layout, dockCorona->layoutManager()->menuLayouts()) {
            QString currentText = (memoryUsage == Latte::Dock::MultipleLayouts && layout == currentName) ?
                                  (" " + i18nc("current layout", "(Current)")) : "";
            QString layoutName = layout + currentText;

            QAction *layoutAction = new QAction(layoutName, m_switchLayoutsMenu);

            layoutAction->setCheckable(true);

            if (activeLayouts.contains(layout)) {
                layoutAction->setChecked(true);
            } else {
                layoutAction->setChecked(false);
            }

            layoutAction->setData(layout);

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
    auto *dockCorona = qobject_cast<Latte::DockCorona *>(containment()->corona());

    if (dockCorona && dockCorona->layoutManager()) {
        const QString layout = action->data().toString();

        if (layout == " _show_latte_settings_dialog_") {
            QTimer::singleShot(400, [this, dockCorona]() {
                dockCorona->layoutManager()->showLatteSettingsDialog(Latte::Dock::LayoutPage);
            });
        } else {
            QTimer::singleShot(400, [this, dockCorona, layout]() {
                dockCorona->layoutManager()->switchToLayout(layout);
            });
        }
    }
}

K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON(lattecontextmenu, Menu, "plasma-containmentactions-lattecontextmenu.json")

#include "menu.moc"
