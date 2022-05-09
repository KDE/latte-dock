/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "menu.h"

// local
#include "contextmenudata.h"
#include "layoutmenuitemwidget.h"

// Qt
#include <QAction>
#include <QDebug>
#include <QFont>
#include <QMenu>
#include <QtDBus>
#include <QTimer>
#include <QLatin1String>

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
const int ACTIONSALWAYSSHOWN = 3;
const int LAYOUTMENUINDEX = 4;
const int VIEWLAYOUTINDEX = 5;
const int VIEWTYPEINDEX = 6;

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
}

Menu::~Menu()
{
    //! sub-menus
    m_addViewMenu->deleteLater();
    m_switchLayoutsMenu->deleteLater();
    m_moveToLayoutMenu->deleteLater();

    //! clear menu actions that have been created from submenus
    m_actions.remove(Latte::Data::ContextMenu::ADDVIEWACTION);
    m_actions.remove(Latte::Data::ContextMenu::LAYOUTSACTION);

    //! actions
    qDeleteAll(m_actions.values());
    m_actions.clear();
}

void Menu::restore(const KConfigGroup &config)
{
    if (!m_actions.isEmpty()) {
        return;
    }

    m_actions[Latte::Data::ContextMenu::SECTIONACTION] = new QAction(this);
    m_actions[Latte::Data::ContextMenu::SECTIONACTION]->setSeparator(true);
    m_actions[Latte::Data::ContextMenu::SECTIONACTION]->setText("Latte");

    m_actions[Latte::Data::ContextMenu::SEPARATOR1ACTION] = new QAction(this);
    m_actions[Latte::Data::ContextMenu::SEPARATOR1ACTION]->setSeparator(true);

    //! Print Message...
    m_actions[Latte::Data::ContextMenu::PRINTACTION] = new QAction(QIcon::fromTheme("edit"), "Print Message...", this);
    connect(m_actions[Latte::Data::ContextMenu::PRINTACTION], &QAction::triggered, [ = ]() {
        qDebug() << "Action Trigerred !!!";
    });

    //! Add Widgets...
    m_actions[Latte::Data::ContextMenu::ADDWIDGETSACTION] = new QAction(QIcon::fromTheme("list-add"), i18n("&Add Widgets..."), this);
    m_actions[Latte::Data::ContextMenu::ADDWIDGETSACTION]->setStatusTip(i18n("Show Widget Explorer"));
    connect(m_actions[Latte::Data::ContextMenu::ADDWIDGETSACTION], &QAction::triggered, this, &Menu::requestWidgetExplorer);
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::ADDWIDGETSACTION, m_actions[Latte::Data::ContextMenu::ADDWIDGETSACTION]);

    /*connect(m_addWidgetsAction, &QAction::triggered, [ = ]() {
        QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("toggleWidgetExplorer");
        }
    });*/

    //! Edit Dock/Panel...
    m_actions[Latte::Data::ContextMenu::EDITVIEWACTION] = new QAction(QIcon::fromTheme("document-edit"), "Edit Dock...", this);
    connect(m_actions[Latte::Data::ContextMenu::EDITVIEWACTION], &QAction::triggered, this, &Menu::requestConfiguration);
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::EDITVIEWACTION, m_actions[Latte::Data::ContextMenu::EDITVIEWACTION]);


    //! Quit Application
    m_actions[Latte::Data::ContextMenu::QUITLATTEACTION] = new QAction(QIcon::fromTheme("application-exit"), i18nc("quit application", "Quit &Latte"));
    connect(m_actions[Latte::Data::ContextMenu::QUITLATTEACTION], &QAction::triggered, this, &Menu::quitApplication);
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::QUITLATTEACTION, m_actions[Latte::Data::ContextMenu::QUITLATTEACTION]);

    //! Layouts submenu
    m_switchLayoutsMenu = new QMenu;
    m_actions[Latte::Data::ContextMenu::LAYOUTSACTION] = m_switchLayoutsMenu->menuAction();
    m_actions[Latte::Data::ContextMenu::LAYOUTSACTION]->setText(i18n("&Layouts"));
    m_actions[Latte::Data::ContextMenu::LAYOUTSACTION]->setIcon(QIcon::fromTheme("user-identity"));
    m_actions[Latte::Data::ContextMenu::LAYOUTSACTION]->setStatusTip(i18n("Switch to another layout"));
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::LAYOUTSACTION, m_actions[Latte::Data::ContextMenu::LAYOUTSACTION]);

    connect(m_switchLayoutsMenu, &QMenu::aboutToShow, this, &Menu::populateLayouts);
    connect(m_switchLayoutsMenu, &QMenu::triggered, this, &Menu::switchToLayout);

    //! Add View submenu
    m_addViewMenu = new QMenu;
    m_actions[Latte::Data::ContextMenu::ADDVIEWACTION] = m_addViewMenu->menuAction();
    m_actions[Latte::Data::ContextMenu::ADDVIEWACTION]->setText(i18n("&Add Dock/Panel"));
    m_actions[Latte::Data::ContextMenu::ADDVIEWACTION]->setIcon(QIcon::fromTheme("list-add"));
    m_actions[Latte::Data::ContextMenu::ADDVIEWACTION]->setStatusTip(i18n("Add dock or panel based on specific template"));
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::ADDVIEWACTION, m_actions[Latte::Data::ContextMenu::ADDVIEWACTION]);

    connect(m_addViewMenu, &QMenu::aboutToShow, this, &Menu::populateViewTemplates);
    connect(m_addViewMenu, &QMenu::triggered, this, &Menu::addView);

    //! Move submenu
    m_moveToLayoutMenu = new QMenu;
    m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION] = m_moveToLayoutMenu->menuAction();
    m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->setText("Move To Layout");
    m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->setIcon(QIcon::fromTheme("transform-move-horizontal"));
    m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->setStatusTip(i18n("Move dock or panel to different layout"));
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::MOVEVIEWACTION, m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]);

    connect(m_moveToLayoutMenu, &QMenu::aboutToShow, this, &Menu::populateMoveToLayouts);
    connect(m_moveToLayoutMenu, &QMenu::triggered, this, &Menu::moveToLayout);

    //! Configure Latte
    m_actions[Latte::Data::ContextMenu::PREFERENCESACTION] = new QAction(QIcon::fromTheme("configure"), i18nc("global settings window", "&Configure Latte..."), this);
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::PREFERENCESACTION, m_actions[Latte::Data::ContextMenu::PREFERENCESACTION]);
    connect(m_actions[Latte::Data::ContextMenu::PREFERENCESACTION], &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("showSettingsWindow", (int)PreferencesPage);
        }
    });

    //! Duplicate Action
    m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION] = new QAction(QIcon::fromTheme("edit-copy"), "Duplicate Dock as Template", this);
    connect(m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION], &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("duplicateView", containment()->id());
        }
    });
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::DUPLICATEVIEWACTION, m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION]);

    //! Export View Template Action
    m_actions[Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION] = new QAction(QIcon::fromTheme("document-export"), "Export as Template...", this);
    connect(m_actions[Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION], &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("exportViewTemplate", containment()->id());
        }
    });
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION, m_actions[Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION]);

    //! Remove Action
    m_actions[Latte::Data::ContextMenu::REMOVEVIEWACTION] = new QAction(QIcon::fromTheme("delete"), "Remove Dock", this);
    connect(m_actions[Latte::Data::ContextMenu::REMOVEVIEWACTION], &QAction::triggered, [=](){
        QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            iface.call("removeView", containment()->id());
        }
    });
    this->containment()->actions()->addAction(Latte::Data::ContextMenu::REMOVEVIEWACTION, m_actions[Latte::Data::ContextMenu::REMOVEVIEWACTION]);

    //! Signals
    connect(this->containment(), &Plasma::Containment::userConfiguringChanged, [=](){
        updateVisibleActions();
    });
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

    actions << m_actions[Latte::Data::ContextMenu::SECTIONACTION];
    actions << m_actions[Latte::Data::ContextMenu::PRINTACTION];
    for(int i=0; i<Latte::Data::ContextMenu::ACTIONSEDITORDER.count(); ++i) {
        actions << m_actions[Latte::Data::ContextMenu::ACTIONSEDITORDER[i]];
    }
    actions << m_actions[Latte::Data::ContextMenu::EDITVIEWACTION];

    m_data.clear();
    m_viewTemplates.clear();
    QDBusInterface iface("org.kde.lattedock", "/Latte", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        QDBusReply<QStringList> contextData = iface.call("contextMenuData", containment()->id());
        m_data = contextData.value();

        QDBusReply<QStringList> templatesData = iface.call("viewTemplatesData");
        m_viewTemplates = templatesData.value();
    }

    m_actionsAlwaysShown = m_data[ACTIONSALWAYSSHOWN].split(";;");

    updateViewData();

    QString configureActionText = (m_view.type == DockView) ? i18n("&Edit Dock...") : i18n("&Edit Panel...");
    if (m_view.isCloned) {
        configureActionText = (m_view.type == DockView) ? i18n("&Edit Original Dock...") : i18n("&Edit Original Panel...");
    }
    m_actions[Latte::Data::ContextMenu::EDITVIEWACTION]->setText(configureActionText);

    const QString duplicateActionText = (m_view.type == DockView) ? i18n("&Duplicate Dock") : i18n("&Duplicate Panel");
    m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION]->setText(duplicateActionText);

    const QString exportTemplateText = (m_view.type == DockView) ? i18n("E&xport Dock as Template") : i18n("E&xport Panel as Template");
    m_actions[Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION]->setText(exportTemplateText);

    m_activeLayoutNames = m_data[ACTIVELAYOUTSINDEX].split(";;");
    const QString moveText = (m_view.type == DockView) ? i18n("&Move Dock To Layout") : i18n("&Move Panel To Layout");
    m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->setText(moveText);

    const QString removeActionText = (m_view.type == DockView) ? i18n("&Remove Dock") : i18n("&Remove Panel");
    m_actions[Latte::Data::ContextMenu::REMOVEVIEWACTION]->setText(removeActionText);

    updateVisibleActions();

    return actions;
}

QAction *Menu::action(const QString &name)
{
    if (m_actions.contains(name)) {
        return m_actions[name];
    }

    return nullptr;
}

void Menu::updateVisibleActions()
{
    if (!m_actions.contains(Latte::Data::ContextMenu::EDITVIEWACTION)
            || !m_actions.contains(Latte::Data::ContextMenu::REMOVEVIEWACTION)) {
        return;
    }

    bool configuring = containment()->isUserConfiguring();

    // normal actions that the user can specify their visibility
    for(auto actionName: m_actions.keys()) {
        if (Latte::Data::ContextMenu::ACTIONSSPECIAL.contains(actionName)) {
            continue;
        } else if (Latte::Data::ContextMenu::ACTIONSALWAYSHIDDEN.contains(actionName)) {
            m_actions[actionName]->setVisible(false);
            continue;
        }

        bool isvisible = m_actionsAlwaysShown.contains(actionName) || configuring;
        m_actions[actionName]->setVisible(isvisible);
    }

    // normal actions with more criteria
    bool isshown = (m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->isVisible() && m_activeLayoutNames.count()>1);
    m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->setVisible(isshown);

    // special actions
    m_actions[Latte::Data::ContextMenu::EDITVIEWACTION]->setVisible(!configuring);
    m_actions[Latte::Data::ContextMenu::SECTIONACTION]->setVisible(true);

    if (m_view.isCloned) {
        m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION]->setVisible(false);
        m_actions[Latte::Data::ContextMenu::EXPORTVIEWTEMPLATEACTION]->setVisible(false);
        m_actions[Latte::Data::ContextMenu::MOVEVIEWACTION]->setVisible(false);
        m_actions[Latte::Data::ContextMenu::REMOVEVIEWACTION]->setVisible(false);
    }

    // because sometimes they are disabled unexpectedly, we should reenable them
    for(auto actionName: m_actions.keys()) {
        m_actions[actionName]->setEnabled(true);
    }
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

void Menu::updateViewData()
{
    QStringList vdata = m_data[VIEWTYPEINDEX].split(";;");
    m_view.type = static_cast<ViewType>((vdata[0]).toInt());
    m_view.isCloned = vdata[1].toInt();
    m_view.clonesCount = vdata[2].toInt();
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
    QAction *duplicateAction = m_addViewMenu->addAction(m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION]->text());
    duplicateAction->setToolTip(m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION]->toolTip());
    duplicateAction->setIcon(m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION]->icon());
    connect(duplicateAction, &QAction::triggered, m_actions[Latte::Data::ContextMenu::DUPLICATEVIEWACTION], &QAction::triggered);
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

    if (layout == QLatin1String(" _show_latte_settings_dialog_")) {
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

K_PLUGIN_CLASS_WITH_JSON(Menu, "plasma-containmentactions-lattecontextmenu.json")

#include "menu.moc"
