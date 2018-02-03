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
    qDeleteAll(m_actions);
}

void Menu::makeActions()
{
    qDeleteAll(m_actions);
    m_actions.clear();

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
    actions << m_printAction;
    actions << m_addWidgetsAction;
    actions << m_configureAction;

    populateLayoutsMenu();

    return actions;
}

QAction *Menu::action(const QString &name)
{
    if (name == "add widgets") {
        return m_addWidgetsAction;
    } else if (name == "configure") {
        return m_configureAction;
    }

    return nullptr;
}

void Menu::populateLayoutsMenu()
{
    auto *dockCorona = qobject_cast<Latte::DockCorona *>(containment()->corona());

    if (dockCorona && dockCorona->layoutManager()->menuLayouts().count() > 1) {
        const QIcon identityIcon = QIcon::fromTheme("user-identity");
        QMenu *layoutsMenu = new QMenu; //(desktopMenu);

        QAction *layoutsAction = new QAction(identityIcon, i18n("Layouts"), this);
        layoutsAction->setIcon(identityIcon);
        layoutsAction->setCheckable(false);
        layoutsAction->setText(i18n("Layouts"));
        layoutsAction->setStatusTip(i18n("Switch to another layout"));

        QStringList activeLayouts = dockCorona->layoutManager()->activeLayoutsNames();
        Latte::Dock::LayoutsMemoryUsage memoryUsage = dockCorona->layoutManager()->memoryUsage();
        QString currentName = dockCorona->layoutManager()->currentLayoutName();

        foreach (auto layout, dockCorona->layoutManager()->menuLayouts()) {
            QString currentText = (memoryUsage == Latte::Dock::MultipleLayouts && layout == currentName) ?
                                  (" " + i18nc("current layout", "(Current)")) : "";
            QString layoutName = layout + currentText;

            QAction *layoutAction = new QAction(layoutName, layoutsMenu);

            layoutAction->setCheckable(true);

            if (activeLayouts.contains(layout)) {
                layoutAction->setChecked(true);
            } else {
                layoutAction->setChecked(false);
            }

            connect(layoutAction, &QAction::triggered, this, [this, dockCorona, layout] {
                dockCorona->layoutManager()->switchToLayout(layout);
            });

            layoutsMenu->addAction(layoutAction);

            qDebug() << layout;
        }

        layoutsMenu->addSeparator();
    }
}

K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON(lattecontextmenu, Menu, "plasma-containmentactions-lattecontextmenu.json")

#include "menu.moc"
