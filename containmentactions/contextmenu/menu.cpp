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

// local
#include "../../liblatte2/types.h"

// Qt
#include <QAction>
#include <QDebug>
#include <QFont>
#include <QMenu>
#include <QtDBus>
#include <QTimer>

// KDE
#include <KActionCollection>
#include <KLocalizedString>

// Plasma
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ServiceJob>

const int LAYOUTSPOS = 3;

Menu::Menu(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
    makeActions();
}

Menu::~Menu()
{
    m_separator1->deleteLater();
    m_separator2->deleteLater();
    m_addWidgetsAction->deleteLater();
    m_configureAction->deleteLater();
    m_printAction->deleteLater();
    m_switchLayoutsMenu->deleteLater();
    m_layoutsAction->deleteLater();
    m_preferenceAction->deleteLater();
}

void Menu::makeActions()
{
    m_separator1 = new QAction(this);
    m_separator1->setSeparator(true);
    m_separator2 = new QAction(this);
    m_separator2->setSeparator(true);

    m_printAction = new QAction(QIcon::fromTheme("edit"), "Print Message...", this);
    connect(m_printAction, &QAction::triggered, [ = ]() {
        qDebug() << "Action Trigerred !!!";
    });

    m_addWidgetsAction = new QAction(QIcon::fromTheme("list-add"), i18n("&Add Widgets..."), this);
    m_addWidgetsAction->setStatusTip(i18n("Show Plasma Widget Explorer"));
    connect(m_addWidgetsAction, &QAction::triggered, [ = ]() {
        QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("toggleWidgetExplorer");
        }
    });

    m_configureAction = new QAction(QIcon::fromTheme("document-edit"), i18nc("view settings window", "View &Settings..."), this);
    connect(m_configureAction, &QAction::triggered, this, &Menu::requestConfiguration);

    connect(this->containment(), &Plasma::Containment::userConfiguringChanged, this, [&](bool configuring){
        m_configureAction->setVisible(!configuring);
        // because sometimes it's disabled unexpectedly
        // we should enable it
        m_configureAction->setEnabled(true);
    });

    m_switchLayoutsMenu = new QMenu;
    m_layoutsAction = m_switchLayoutsMenu->menuAction();
    m_layoutsAction->setText(i18n("&Layouts"));
    m_layoutsAction->setIcon(QIcon::fromTheme("user-identity"));
    m_layoutsAction->setStatusTip(i18n("Switch to another layout"));

    connect(m_switchLayoutsMenu, &QMenu::aboutToShow, this, &Menu::populateLayouts);
    connect(m_switchLayoutsMenu, &QMenu::triggered, this, &Menu::switchToLayout);

    m_preferenceAction = new QAction(QIcon::fromTheme("configure"), i18nc("global settings window", "&Configure Latte..."), this);
    connect(m_preferenceAction, &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("showSettingsWindow", (int)Latte::Types::PreferencesPage);
        }
    });
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
    actions << m_preferenceAction;

    actions << m_separator2;
    actions << m_addWidgetsAction;
    actions << m_configureAction;
    
    m_data.clear();
    QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("setContextMenuView", (int)containment()->id());
        QDBusReply<QStringList> replyData = iface.call("contextMenuData");

        m_data = replyData.value();
    }

    if (m_data.size() > LAYOUTSPOS + 1) {
        m_layoutsAction->setEnabled(true);
        m_layoutsAction->setVisible(true);
    } else {
        m_layoutsAction->setVisible(false);
    }

    Latte::Types::ViewType viewType{Latte::Types::DockView};

    if (m_data.size() >= LAYOUTSPOS + 1) {
        viewType = static_cast<Latte::Types::ViewType>((m_data[2]).toInt());
    }

    const QString configureActionText = (viewType == Latte::Types::DockView) ? i18nc("dock settings window", "&Edit Dock...") : i18nc("panel settings window", "&Edit Panel...");
    m_configureAction->setText(configureActionText);

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

    if (m_data.size() > LAYOUTSPOS + 1) {
        //when there are more than 1 layouts present
        Latte::Types::LayoutsMemoryUsage memoryUsage = static_cast<Latte::Types::LayoutsMemoryUsage>((m_data[0]).toInt());
        QString currentName = m_data[1];

        for (int i = LAYOUTSPOS; i < m_data.size(); ++i) {
            bool isActive = m_data[i].startsWith("0") ? false : true;

            QString layout = m_data[i].right(m_data[i].length() - 2);

            QString currentText = (memoryUsage == Latte::Types::MultipleLayouts && layout == currentName) ?
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

        QAction *editLayoutsAction = new QAction(i18n("Manage &Layouts..."), m_switchLayoutsMenu);
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
                iface.call("showSettingsWindow", (int)Latte::Types::LayoutPage);
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
