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
const char LAYOUTSNAME[] = "layouts";
const char PREFERENCESNAME[] = "preferences";
const char QUITLATTENAME[] = "quit latte";
const char ADDWIDGETSNAME[] = "add latte widgets";
const char DUPLICATEVIEWNAME[] = "duplicate view";
const char EDITVIEWNAME[] = "edit view";
const char EXPORTVIEWTEMPLATENAME[] = "export view";
const char REMOVEVIEWNAME[] = "remove view";

enum ViewType
{
    DockView = 0,
    PanelView
};

enum LayoutsMemoryUsage
{
    SingleLayout = 0,
    MultipleLayouts
};

enum LatteConfigPage
{
    LayoutPage = 0,
    PreferencesPage
};

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
    m_removeAction->deleteLater();
    m_quitApplication->deleteLater();
}

void Menu::makeActions()
{
    m_separator1 = new QAction(this);
    m_separator1->setSeparator(true);
    m_separator2 = new QAction(this);
    m_separator2->setSeparator(true);

    //! Print Message...
    m_printAction = new QAction(QIcon::fromTheme("edit"), "Print Message...", this);
    connect(m_printAction, &QAction::triggered, [ = ]() {
        qDebug() << "Action Trigerred !!!";
    });

    //! Add Widgets...
    m_addWidgetsAction = new QAction(QIcon::fromTheme("list-add"), i18n("&Add Widgets..."), this);
    m_addWidgetsAction->setStatusTip(i18n("Show Widget Explorer"));
    connect(m_addWidgetsAction, &QAction::triggered, this, &Menu::requestWidgetExplorer);
    this->containment()->actions()->addAction(ADDWIDGETSNAME, m_addWidgetsAction);

    /*connect(m_addWidgetsAction, &QAction::triggered, [ = ]() {
        QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("toggleWidgetExplorer");
        }
    });*/

    //! Edit Dock/Panel...
    m_configureAction = new QAction(QIcon::fromTheme("document-edit"), "Edit Dock...", this);
    connect(m_configureAction, &QAction::triggered, this, &Menu::requestConfiguration);
    this->containment()->actions()->addAction(EDITVIEWNAME, m_configureAction);


    //! Quit Application
    m_quitApplication = new QAction(QIcon::fromTheme("application-exit"), i18nc("quit application", "Quit &Latte"));
    connect(m_quitApplication, &QAction::triggered, this, &Menu::quitApplication);
    this->containment()->actions()->addAction(QUITLATTENAME, m_quitApplication);

    //! Layouts submenu
    m_switchLayoutsMenu = new QMenu;
    m_layoutsAction = m_switchLayoutsMenu->menuAction();
    m_layoutsAction->setText(i18n("&Layouts"));
    m_layoutsAction->setIcon(QIcon::fromTheme("user-identity"));
    m_layoutsAction->setStatusTip(i18n("Switch to another layout"));
    this->containment()->actions()->addAction(LAYOUTSNAME, m_layoutsAction);

    connect(m_switchLayoutsMenu, &QMenu::aboutToShow, this, &Menu::populateLayouts);
    connect(m_switchLayoutsMenu, &QMenu::triggered, this, &Menu::switchToLayout);

    //! Configure Latte
    m_preferenceAction = new QAction(QIcon::fromTheme("configure"), i18nc("global settings window", "&Configure Latte..."), this);
    this->containment()->actions()->addAction(PREFERENCESNAME, m_preferenceAction);
    connect(m_preferenceAction, &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("showSettingsWindow", (int)PreferencesPage);
        }
    });

    //! Duplicate Action
    m_duplicateAction = new QAction(QIcon::fromTheme("edit-copy"), "Duplicate Dock as Template", this);
    m_duplicateAction->setVisible(containment()->isUserConfiguring());
    connect(m_duplicateAction, &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("duplicateView", containment()->id());
        }
    });
    this->containment()->actions()->addAction(DUPLICATEVIEWNAME, m_duplicateAction);

    //! Duplicate Action
    m_exportViewAction = new QAction(QIcon::fromTheme("document-export"), "Export as Template...", this);
    m_exportViewAction->setVisible(containment()->isUserConfiguring());
    connect(m_exportViewAction, &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("exportViewTemplate", containment()->id());
        }
    });
    this->containment()->actions()->addAction(EXPORTVIEWTEMPLATENAME, m_exportViewAction);

    //! Remove Action
    m_removeAction = new QAction(QIcon::fromTheme("delete"), "Remove Dock", this);
    m_removeAction->setVisible(containment()->isUserConfiguring());
    connect(m_removeAction, &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("removeView", containment()->id());
        }
    });
    this->containment()->actions()->addAction(REMOVEVIEWNAME, m_removeAction);

    //! Signals
    connect(this->containment(), &Plasma::Containment::userConfiguringChanged, this, &Menu::onUserConfiguringChanged);
}

void Menu::requestConfiguration()
{
    if (this->containment()) {
        emit this->containment()->configureRequested(containment());
    }
}

void Menu::requestWidgetExplorer()
{
    if (this->containment()) {
        emit this->containment()->showAddWidgetsInterface(QPointF());
    }
}

QList<QAction *> Menu::contextualActions()
{
    QList<QAction *> actions;

    actions << m_separator1;
    //actions << m_printAction;
    actions << m_layoutsAction;
    actions << m_preferenceAction;
    actions << m_quitApplication;

    actions << m_separator2;
    actions << m_addWidgetsAction;
    actions << m_duplicateAction;
    actions << m_exportViewAction;
    actions << m_configureAction;
    actions << m_removeAction;

    m_data.clear();
    QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("setContextMenuView", (int)containment()->id());
        QDBusReply<QStringList> replyData = iface.call("contextMenuData");

        m_data = replyData.value();
    }

    ViewType viewType{DockView};

    if (m_data.size() >= LAYOUTSPOS + 1) {
        viewType = static_cast<ViewType>((m_data[2]).toInt());
    }

    const QString configureActionText = (viewType == DockView) ? i18n("&Edit Dock...") : i18n("&Edit Panel...");
    m_configureAction->setText(configureActionText);

    const QString duplicateActionText = (viewType == DockView) ? i18n("&Duplicate Dock") : i18n("&Duplicate Panel");
    m_duplicateAction->setText(duplicateActionText);

    const QString exportTemplateText = (viewType == DockView) ? i18n("E&xport Dock as Template") : i18n("E&xport Panel as Template");
    m_exportViewAction->setText(exportTemplateText);

    const QString removeActionText = (viewType == DockView) ? i18n("&Remove Dock") : i18n("&Remove Panel");
    m_removeAction->setText(removeActionText);

    return actions;
}

QAction *Menu::action(const QString &name)
{
    if (name == ADDWIDGETSNAME) {
        return m_addWidgetsAction;
    } else if (name == DUPLICATEVIEWNAME) {
        return m_duplicateAction;
    } else if (name == EDITVIEWNAME) {
        return m_configureAction;
    } else if (name == EXPORTVIEWTEMPLATENAME) {
        return m_exportViewAction;
    } else if (name == LAYOUTSNAME) {
        return m_layoutsAction;
    } else if (name == PREFERENCESNAME) {
        return m_preferenceAction;
    } else if (name == QUITLATTENAME) {
        return m_quitApplication;
    } else if (name == REMOVEVIEWNAME) {
        return m_removeAction;
    }

    return nullptr;
}

void Menu::onUserConfiguringChanged(const bool &configuring)
{
    if (!m_configureAction || !m_removeAction) {
        return;
    }

    m_configureAction->setVisible(!configuring);
    m_duplicateAction->setVisible(configuring);
    m_exportViewAction->setVisible(configuring);
    m_removeAction->setVisible(configuring);

    // because sometimes they are disabled unexpectedly, we should reenable them
    m_configureAction->setEnabled(true);
    m_duplicateAction->setEnabled(true);
    m_exportViewAction->setEnabled(true);
    m_removeAction->setEnabled(true);
}


void Menu::populateLayouts()
{
    m_switchLayoutsMenu->clear();

    LayoutsMemoryUsage memoryUsage = static_cast<LayoutsMemoryUsage>((m_data[0]).toInt());
    QStringList currentNames = m_data[1].split(";;");

    bool hasActiveNoCurrentLayout{false};

    if (memoryUsage == LayoutsMemoryUsage::MultipleLayouts) {
        for (int i = LAYOUTSPOS; i < m_data.size(); ++i) {
            QString layout = m_data[i].right(m_data[i].length() - 2);
            if (!currentNames.contains(layout)) {
                hasActiveNoCurrentLayout = true;
                break;
            }
        }
    }

    for (int i = LAYOUTSPOS; i < m_data.size(); ++i) {
        bool isActive = m_data[i].startsWith("0") ? false : true;

        QString layout = m_data[i].right(m_data[i].length() - 2);
        QString layoutText = layout;

        bool isCurrent = ((memoryUsage == SingleLayout && isActive)
                          || (memoryUsage == MultipleLayouts && currentNames.contains(layout)));

        if (isCurrent && hasActiveNoCurrentLayout) {
            layoutText += QString(" " + i18nc("current layout", "[Current]"));
        }

        QAction *layoutAction = m_switchLayoutsMenu->addAction(layoutText);

        if (memoryUsage == LayoutsMemoryUsage::SingleLayout) {
            layoutAction->setCheckable(true);

            if (isActive) {
                layoutAction->setChecked(true);
            } else {
                layoutAction->setChecked(false);
            }
        }

        layoutAction->setData(layout);

        if (isCurrent) {
            QFont font = layoutAction->font();
            font.setBold(true);
            layoutAction->setFont(font);
        }
    }

    m_switchLayoutsMenu->addSeparator();

    QAction *editLayoutsAction = m_switchLayoutsMenu->addAction(i18n("Manage &Layouts..."));
    editLayoutsAction->setData(QStringLiteral(" _show_latte_settings_dialog_"));
}

void Menu::switchToLayout(QAction *action)
{
    const QString layout = action->data().toString();

    if (layout == " _show_latte_settings_dialog_") {
        QTimer::singleShot(400, [this]() {
            QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

            if (iface.isValid()) {
                iface.call("showSettingsWindow", (int)LayoutPage);
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

void Menu::quitApplication()
{
    QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("quitApplication");
    }
}

K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON(lattecontextmenu, Menu, "plasma-containmentactions-lattecontextmenu.json")

#include "menu.moc"
