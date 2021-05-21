/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "menu.h"

// local
#include "layoutmenuitemwidget.h"

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

const int MEMORYINDEX = 0;
const int ACTIVELAYOUTSINDEX = 1;
const int CURRENTLAYOUTSINDEX = 2;
const int LAYOUTMENUINDEX = 3;
const int VIEWTYPEINDEX = 4;
const int VIEWLAYOUTINDEX = 5;

const char ADDVIEWNAME[] = "add view";
const char LAYOUTSNAME[] = "layouts";
const char PREFERENCESNAME[] = "preferences";
const char QUITLATTENAME[] = "quit latte";
const char ADDWIDGETSNAME[] = "add latte widgets";
const char DUPLICATEVIEWNAME[] = "duplicate view";
const char EDITVIEWNAME[] = "edit view";
const char EXPORTVIEWTEMPLATENAME[] = "export view";
const char REMOVEVIEWNAME[] = "remove view";
const char MOVEVIEWNAME[] = "move view";

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
    m_sectionAction->deleteLater();
    m_separator->deleteLater();

    //! sub-menus
    m_addViewMenu->deleteLater();
    m_switchLayoutsMenu->deleteLater();
    m_moveToLayoutMenu->deleteLater();

    //! actions
    m_addWidgetsAction->deleteLater();
    m_configureAction->deleteLater();
    m_duplicateAction->deleteLater();
    m_exportViewAction->deleteLater();
    m_preferenceAction->deleteLater();
    m_printAction->deleteLater();    
    m_removeAction->deleteLater();
    m_quitApplication->deleteLater();
}

void Menu::makeActions()
{
    m_sectionAction = new QAction(this);
    m_sectionAction->setSeparator(true);
    m_sectionAction->setText("Latte");

    m_separator = new QAction(this);
    m_separator->setSeparator(true);

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

    //! Add View submenu
    m_addViewMenu = new QMenu;
    m_addViewAction = m_addViewMenu->menuAction();
    m_addViewAction->setText(i18n("&Add Dock/Panel"));
    m_addViewAction->setIcon(QIcon::fromTheme("list-add"));
    m_addViewAction->setStatusTip(i18n("Add dock or panel based on specific template"));
    this->containment()->actions()->addAction(ADDVIEWNAME, m_addViewAction);

    connect(m_addViewMenu, &QMenu::aboutToShow, this, &Menu::populateViewTemplates);
    connect(m_addViewMenu, &QMenu::triggered, this, &Menu::addView);

    //! Move submenu
    m_moveToLayoutMenu = new QMenu;
    m_moveAction = m_moveToLayoutMenu->menuAction();
    m_moveAction->setVisible(containment()->isUserConfiguring());
    m_moveAction->setText("Move To Layout");
    m_moveAction->setIcon(QIcon::fromTheme("transform-move-horizontal"));
    m_moveAction->setStatusTip(i18n("Move dock or panel to different layout"));
    this->containment()->actions()->addAction(MOVEVIEWNAME, m_moveAction);

    connect(m_moveToLayoutMenu, &QMenu::aboutToShow, this, &Menu::populateMoveToLayouts);
    connect(m_moveToLayoutMenu, &QMenu::triggered, this, &Menu::moveToLayout);

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

    actions << m_sectionAction;
    //actions << m_printAction;
    actions << m_layoutsAction;
    actions << m_preferenceAction;
    actions << m_quitApplication;

    actions << m_separator;
    actions << m_addWidgetsAction;
    actions << m_addViewAction;
    actions << m_moveAction;
    actions << m_exportViewAction;
    actions << m_configureAction;
    actions << m_removeAction;

    m_data.clear();
    m_viewTemplates.clear();
    QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        QDBusReply<QStringList> contextData = iface.call("contextMenuData", containment()->id());
        m_data = contextData.value();

        QDBusReply<QStringList> templatesData = iface.call("viewTemplatesData");
        m_viewTemplates = templatesData.value();
    }

    ViewType viewType{static_cast<ViewType>((m_data[VIEWTYPEINDEX]).toInt())};

    const QString configureActionText = (viewType == DockView) ? i18n("&Edit Dock...") : i18n("&Edit Panel...");
    m_configureAction->setText(configureActionText);

    const QString duplicateActionText = (viewType == DockView) ? i18n("&Duplicate Dock") : i18n("&Duplicate Panel");
    m_duplicateAction->setText(duplicateActionText);

    const QString exportTemplateText = (viewType == DockView) ? i18n("E&xport Dock as Template") : i18n("E&xport Panel as Template");
    m_exportViewAction->setText(exportTemplateText);

    QStringList activeNames = m_data[ACTIVELAYOUTSINDEX].split(";;");
    if (activeNames.count() > 1 && containment()->isUserConfiguring()) {
        m_moveAction->setVisible(true);
        const QString moveText = (viewType == DockView) ? i18n("&Move Dock To Layout") : i18n("&Move Panel To Layout");
        m_moveAction->setText(moveText);
    } else {
        m_moveAction->setVisible(false);
    }

    const QString removeActionText = (viewType == DockView) ? i18n("&Remove Dock") : i18n("&Remove Panel");
    m_removeAction->setText(removeActionText);

    return actions;
}

QAction *Menu::action(const QString &name)
{
    if (name == ADDVIEWNAME) {
        return m_addViewAction;
    } else if (name == ADDWIDGETSNAME) {
        return m_addWidgetsAction;
    } else if (name == DUPLICATEVIEWNAME) {
        return m_duplicateAction;
    } else if (name == EDITVIEWNAME) {
        return m_configureAction;
    } else if (name == EXPORTVIEWTEMPLATENAME) {
        return m_exportViewAction;
    } else if (name == LAYOUTSNAME) {
        return m_layoutsAction;
    } else if (name == MOVEVIEWNAME) {
        return m_moveAction;
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
    m_exportViewAction->setVisible(configuring);
    m_moveAction->setVisible(configuring);
    m_removeAction->setVisible(configuring);

    // because sometimes they are disabled unexpectedly, we should reenable them
    m_configureAction->setEnabled(true);
    m_exportViewAction->setEnabled(true);
    m_moveAction->setEnabled(true);
    m_removeAction->setEnabled(true);
}


void Menu::populateLayouts()
{
    m_switchLayoutsMenu->clear();

    LayoutsMemoryUsage memoryUsage = static_cast<LayoutsMemoryUsage>((m_data[MEMORYINDEX]).toInt());
    QStringList activeNames = m_data[ACTIVELAYOUTSINDEX].split(";;");
    QStringList currentNames = m_data[CURRENTLAYOUTSINDEX].split(";;");

    QList<LayoutInfo> layoutsmenulist;

    QStringList layoutsdata = m_data[LAYOUTMENUINDEX].split(";;");

    for (int i=0; i<layoutsdata.count(); ++i) {
        QStringList cdata = layoutsdata[i].split("**");

        LayoutInfo info;
        info.layoutName = cdata[0];
        info.isBackgroundFileIcon = cdata[1].toInt();
        info.iconName = cdata[2];

        layoutsmenulist << info;
    }

    for (int i = 0; i < layoutsmenulist.count(); ++i) {
        bool isActive = activeNames.contains(layoutsmenulist[i].layoutName);

        QString layoutText = layoutsmenulist[i].layoutName;

        bool isCurrent = ((memoryUsage == SingleLayout && isActive)
                          || (memoryUsage == MultipleLayouts && currentNames.contains(layoutsmenulist[i].layoutName)));


        QWidgetAction *action = new QWidgetAction(m_switchLayoutsMenu);
        action->setText(layoutsmenulist[i].layoutName);
        action->setCheckable(true);
        action->setChecked(isCurrent);
        action->setData(layoutsmenulist[i].layoutName);

        LayoutMenuItemWidget *menuitem = new LayoutMenuItemWidget(action, m_switchLayoutsMenu);
        menuitem->setIcon(layoutsmenulist[i].isBackgroundFileIcon, layoutsmenulist[i].iconName);
        action->setDefaultWidget(menuitem);
        m_switchLayoutsMenu->addAction(action);
    }

    m_switchLayoutsMenu->addSeparator();

    QWidgetAction *editaction = new QWidgetAction(m_switchLayoutsMenu);
    editaction->setText(i18n("Edit &Layouts..."));
    editaction->setCheckable(false);
    editaction->setData(QStringLiteral(" _show_latte_settings_dialog_"));
    editaction->setVisible(false);

    LayoutMenuItemWidget *editmenuitem = new LayoutMenuItemWidget(editaction, m_switchLayoutsMenu);
    editmenuitem->setIcon(false, "document-edit");
    editaction->setDefaultWidget(editmenuitem);
    m_switchLayoutsMenu->addAction(editaction);
}

void Menu::populateMoveToLayouts()
{
    m_moveToLayoutMenu->clear();

    LayoutsMemoryUsage memoryUsage = static_cast<LayoutsMemoryUsage>((m_data[MEMORYINDEX]).toInt());

    if (memoryUsage == LayoutsMemoryUsage::MultipleLayouts) {
        QStringList activeNames = m_data[ACTIVELAYOUTSINDEX].split(";;");
        QStringList currentNames = m_data[CURRENTLAYOUTSINDEX].split(";;");
        QString viewLayoutName = m_data[VIEWLAYOUTINDEX];

        QList<LayoutInfo> layoutsmenulist;

        QStringList layoutsdata = m_data[LAYOUTMENUINDEX].split(";;");

        for (int i=0; i<layoutsdata.count(); ++i) {
            QStringList cdata = layoutsdata[i].split("**");

            LayoutInfo info;
            info.layoutName = cdata[0];
            info.isBackgroundFileIcon = cdata[1].toInt();
            info.iconName = cdata[2];

            layoutsmenulist << info;
        }

        for (int i = 0; i < layoutsmenulist.count(); ++i) {
            bool isCurrent = currentNames.contains(layoutsmenulist[i].layoutName) && activeNames.contains(layoutsmenulist[i].layoutName);
            bool isViewCurrentLayout = layoutsmenulist[i].layoutName == viewLayoutName;

            QWidgetAction *action = new QWidgetAction(m_moveToLayoutMenu);
            action->setText(layoutsmenulist[i].layoutName);
            action->setCheckable(true);
            action->setChecked(isViewCurrentLayout);
            action->setData(isViewCurrentLayout ? QString() : layoutsmenulist[i].layoutName);

            LayoutMenuItemWidget *menuitem = new LayoutMenuItemWidget(action, m_moveToLayoutMenu);
            menuitem->setIcon(layoutsmenulist[i].isBackgroundFileIcon, layoutsmenulist[i].iconName);
            action->setDefaultWidget(menuitem);
            m_moveToLayoutMenu->addAction(action);
        }
    }
}

void Menu::populateViewTemplates()
{
    m_addViewMenu->clear();

    for(int i=0; i<m_viewTemplates.count(); ++i) {
        if (i % 2 == 1) {
            //! even records are the templates ids and they have already been registered
            continue;
        }

        QAction *templateAction = m_addViewMenu->addAction(m_viewTemplates[i]);
        templateAction->setIcon(QIcon::fromTheme("list-add"));
        templateAction->setData(m_viewTemplates[i+1]);
    }

    QAction *templatesSeparatorAction = m_addViewMenu->addSeparator();
    QAction *duplicateAction = m_addViewMenu->addAction(m_duplicateAction->text());
    duplicateAction->setToolTip(m_duplicateAction->toolTip());
    duplicateAction->setIcon(m_duplicateAction->icon());
    connect(duplicateAction, &QAction::triggered, m_duplicateAction, &QAction::triggered);
}

void Menu::addView(QAction *action)
{
    const QString templateId = action->data().toString();

    QTimer::singleShot(400, [this, templateId]() {
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("addView", containment()->id(), templateId);
        }
    });
}

void Menu::moveToLayout(QAction *action)
{
    const QString layoutName = action->data().toString();

    QTimer::singleShot(400, [this, layoutName]() {
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("moveViewToLayout", containment()->id(), layoutName);
        }
    });
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
