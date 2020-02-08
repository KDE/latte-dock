/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "globalshortcuts.h"

// local
#include "modifiertracker.h"
#include "shortcutstracker.h"
#include "../lattecorona.h"
#include "../layout/centrallayout.h"
#include "../layouts/manager.h"
#include "../layouts/synchronizer.h"
#include "../settings/universalsettings.h"
#include "../view/containmentinterface.h"
#include "../view/view.h"

// C++
#include <array>

// Qt
#include <QAction>
#include <QDebug>
#include <QQuickItem>
#include <QMetaMethod>
#include <QX11Info>

// KDE
#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginMetaData>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>

namespace Latte {

const int APPLETEXECUTIONDELAY = 400;

GlobalShortcuts::GlobalShortcuts(QObject *parent)
    : QObject(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(parent);
    m_modifierTracker = new ShortcutsPart::ModifierTracker(this);
    m_shortcutsTracker = new ShortcutsPart::ShortcutsTracker(this);

    if (m_corona) {
        init();
    }

    m_hideViewsTimer.setSingleShot(true);

    if (QX11Info::isPlatformX11()) {
        //in X11 the timer is a poller that checks to see if the modifier keys
        //from user global shortcut have been released
        m_hideViewsTimer.setInterval(300);
    } else {
        //on wayland in acting just as simple timer that hides the view afterwards
        m_hideViewsTimer.setInterval(2500);
    }

    connect(&m_hideViewsTimer, &QTimer::timeout, this, &GlobalShortcuts::hideViewsTimerSlot);
}

GlobalShortcuts::~GlobalShortcuts()
{
    if (m_modifierTracker) {
        m_modifierTracker->deleteLater();
    }

    if (m_shortcutsTracker) {
        m_shortcutsTracker->deleteLater();
    }
}

void GlobalShortcuts::init()
{
    KActionCollection *generalActions = new KActionCollection(m_corona);

    //show-hide the main view in the primary screen
    QAction *showAction = generalActions->addAction(QStringLiteral("show latte view"));
    showAction->setText(i18n("Show Latte Dock/Panel"));
    showAction->setShortcut(QKeySequence(Qt::META + '`'));
    KGlobalAccel::setGlobalShortcut(showAction, QKeySequence(Qt::META + '`'));
    connect(showAction, &QAction::triggered, this, [this]() {
        showViews();
    });

    //show-cycle between Latte settings windows
    QAction *settingsAction = generalActions->addAction(QStringLiteral("show view settings"));
    settingsAction->setText(i18n("Cycle Through Dock/Panel Settings Windows"));
    KGlobalAccel::setGlobalShortcut(settingsAction, QKeySequence(Qt::META + Qt::Key_A));
    connect(settingsAction, &QAction::triggered, this, [this] {
        m_modifierTracker->cancelMetaPressed();
        showSettings();
    });

    //show the layouts editor
    QAction *layoutsAction = generalActions->addAction(QStringLiteral("show latte global settings"));
    layoutsAction->setText(i18n("Show Latte Global Settings"));
    layoutsAction->setShortcut(QKeySequence(Qt::META + Qt::Key_W));
    KGlobalAccel::setGlobalShortcut(layoutsAction, QKeySequence(Qt::META + Qt::Key_W));
    connect(layoutsAction, &QAction::triggered, this, [this]() {
        m_modifierTracker->cancelMetaPressed();
        m_corona->layoutsManager()->showLatteSettingsDialog(Types::LayoutPage);
    });

    KActionCollection *taskbarActions = new KActionCollection(m_corona);

    //activate actions [1-9]
    for (int i = 1; i < 10; ++i) {
        const int entryNumber = i;
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_0 + i);

        QAction *action = taskbarActions->addAction(QStringLiteral("activate entry %1").arg(QString::number(entryNumber)));
        action->setText(i18n("Activate Entry %1", entryNumber));
        action->setShortcut(QKeySequence(Qt::META + key));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + key));
        connect(action, &QAction::triggered, this, [this, i] {
            // qDebug() << "meta action...";
            m_modifierTracker->cancelMetaPressed();
            activateEntry(i, static_cast<Qt::Key>(Qt::META));
        });
    }

    //! Array that is used to register correctly actions for task index>=10 and <19
    std::array<Qt::Key, 10> keysAboveTen{  Qt::Key_0,  Qt::Key_Z,  Qt::Key_X, Qt::Key_C, Qt::Key_V, Qt::Key_B, Qt::Key_N, Qt::Key_M, Qt::Key_Comma, Qt::Key_Period };

    //activate actions [10-19]
    for (int i = 10; i < 20; ++i) {
        QAction *action = taskbarActions->addAction(QStringLiteral("activate entry %1").arg(QString::number(i)));
        action->setText(i18n("Activate Entry %1", i));
        action->setShortcut(QKeySequence(Qt::META + keysAboveTen[i - 10]));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + keysAboveTen[i - 10]));
        connect(action, &QAction::triggered, this, [this, i] {
            m_modifierTracker->cancelMetaPressed();
            activateEntry(i, static_cast<Qt::Key>(Qt::META));
        });
    }

    //new instance actions [1-9]
    for (int i = 1; i < 10; ++i) {
        const int entryNumber = i;
        const Qt::Key key = static_cast<Qt::Key>(Qt::Key_0 + i);

        QAction *action = taskbarActions->addAction(QStringLiteral("new instance for entry %1").arg(QString::number(entryNumber)));
        action->setText(i18n("New Instance for Entry %1", entryNumber));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + Qt::CTRL + key));
        connect(action, &QAction::triggered, this, [this, i] {
            // qDebug() << "meta + ctrl + action...";
            m_modifierTracker->cancelMetaPressed();
            activateEntry(i, static_cast<Qt::Key>(Qt::CTRL));
        });
    }

    //new instance actions [10-19]
    for (int i = 10; i < 20; ++i) {
        QAction *action = taskbarActions->addAction(QStringLiteral("new instance for entry %1").arg(QString::number(i)));
        action->setText(i18n("New Instance for Entry %1", i));
        KGlobalAccel::setGlobalShortcut(action, QKeySequence(Qt::META + Qt::CTRL + keysAboveTen[i - 10]));
        connect(action, &QAction::triggered, this, [this, i] {
            m_modifierTracker->cancelMetaPressed();
            activateEntry(i, static_cast<Qt::Key>(Qt::CTRL));
        });
    }

    m_singleMetaAction = new QAction(this);
    m_singleMetaAction->setShortcut(QKeySequence(Qt::META));

    connect(m_corona->universalSettings(), &UniversalSettings::metaPressAndHoldEnabledChanged , this, [&]() {
        if (!m_corona->universalSettings()->metaPressAndHoldEnabled()) {
            m_modifierTracker->blockModifierTracking(Qt::Key_Super_L);
            m_modifierTracker->blockModifierTracking(Qt::Key_Super_R);
        } else {
            m_modifierTracker->unblockModifierTracking(Qt::Key_Super_L);
            m_modifierTracker->unblockModifierTracking(Qt::Key_Super_R);
        }
    });

    //display shortcut badges while holding Meta
    connect(m_modifierTracker, &ShortcutsPart::ModifierTracker::metaModifierPressed, this, [&]() {
        m_metaShowedViews = true;
        showViews();
    });
}

ShortcutsPart::ShortcutsTracker *GlobalShortcuts::shortcutsTracker() const
{
    return m_shortcutsTracker;
}

Latte::View *GlobalShortcuts::highestApplicationLauncherView(const QList<Latte::View *> &views) const
{
    if (views.isEmpty()) {
        return nullptr;
    }

    Latte::View *highestPriorityView{nullptr};

    for (const auto view : views) {
        if (view->interface()->applicationLauncherHasGlobalShortcut()) {
            highestPriorityView = view;
            break;
        }
    }

    if (!highestPriorityView) {
        for (const auto view : views) {
            if (view->interface()->containsApplicationLauncher()) {
                highestPriorityView = view;
                break;
            }
        }
    }

    return highestPriorityView;
}

//! Activate launcher menu through dbus interface
void GlobalShortcuts::activateLauncherMenu()
{
    if (m_metaShowedViews) {
        return;
    }

    QList<Latte::View *> sortedViews;
    CentralLayout *currentLayout = m_corona->layoutsManager()->currentLayout();

    if (currentLayout) {
        sortedViews = currentLayout->sortedLatteViews();
    }

    Latte::View *highestPriorityView = highestApplicationLauncherView(sortedViews);

    if (highestPriorityView) {
        if (highestPriorityView->visibility()->isHidden() && highestPriorityView->interface()->applicationLauncherInPopup()) {
            m_lastInvokedAction = m_singleMetaAction;

            highestPriorityView->visibility()->setBlockHiding(true);

            //! delay the execution in order to show first the view
            QTimer::singleShot(APPLETEXECUTIONDELAY, [this, highestPriorityView]() {
                highestPriorityView->toggleAppletExpanded(highestPriorityView->interface()->applicationLauncherId());
            });
        } else {
            highestPriorityView->toggleAppletExpanded(highestPriorityView->interface()->applicationLauncherId());
        }
    }
}

bool GlobalShortcuts::activatePlasmaTaskManager(const Latte::View *view, int index, Qt::Key modifier, bool *delayedExecution)
{
    bool activation{modifier == static_cast<Qt::Key>(Qt::META)};
    bool newInstance{!activation};

    if (view->visibility()->isHidden()) {
        //! delay the execution in order to show first the view
        QTimer::singleShot(APPLETEXECUTIONDELAY, [this, view, index, activation]() {
            if (activation) {
                view->interface()->activatePlasmaTask(index);
            } else {
                view->interface()->newInstanceForPlasmaTask(index);
            }
        });

        *delayedExecution = true;

        return true;
    } else {
        *delayedExecution = false;

        return (activation ? view->interface()->activatePlasmaTask(index) : view->interface()->newInstanceForPlasmaTask(index));
    }
}

bool GlobalShortcuts::activateLatteEntry(Latte::View *view, int index, Qt::Key modifier, bool *delayedExecution)
{
    bool activation{modifier == static_cast<Qt::Key>(Qt::META)};
    bool newInstance{!activation};

    int appletId = view->interface()->appletIdForIndex(index);
    bool hasPopUp {(appletId>-1 && view->appletIsExpandable(appletId))};

    if (view->visibility()->isHidden() && hasPopUp) {
        //! delay the execution in order to show first the view
        QTimer::singleShot(APPLETEXECUTIONDELAY, [this, view, index, activation]() {
            if (activation) {
                view->interface()->activateEntry(index);
            } else {
                view->interface()->newInstanceForEntry(index);
            }
        });

        *delayedExecution = true;

        return true;
    } else {
        *delayedExecution = false;

        return (activation ? view->interface()->activateEntry(index) : view->interface()->newInstanceForEntry(index));
    }
}


//! Activate task manager entry
void GlobalShortcuts::activateEntry(int index, Qt::Key modifier)
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());

    QList<Latte::View *> sortedViews;
    CentralLayout *currentLayout = m_corona->layoutsManager()->currentLayout();

    if (currentLayout) {
        sortedViews = currentLayout->sortedLatteViews();
    }

    for (const auto view : sortedViews) {
        if (view->layout()->preferredForShortcutsTouched() && !view->isPreferredForShortcuts()) {
            continue;
        }

        bool delayed{false};

        if ((!view->latteTasksArePresent() && view->tasksPresent() &&
             activatePlasmaTaskManager(view, index, modifier, &delayed))
                || activateLatteEntry(view, index, modifier, &delayed)) {

            if (!m_hideViews.contains(view)) {
                m_hideViews.append(view);
            }

            if (delayed) {
                view->visibility()->setBlockHiding(true);
                m_hideViewsTimer.start();
            }

            return;
        }
    }

}

//! update badge for specific view item
void GlobalShortcuts::updateViewItemBadge(QString identifier, QString value)
{
    CentralLayout *currentLayout = m_corona->layoutsManager()->currentLayout();
    QList<Latte::View *> views;

    if (currentLayout) {
        views = currentLayout->latteViews();
    }

    // update badges in all Latte Tasks plasmoids
    for (const auto &view : views) {
        view->interface()->updateBadgeForLatteTask(identifier, value);
    }
}

void GlobalShortcuts::showViews()
{
    m_lastInvokedAction = dynamic_cast<QAction *>(sender());
    if (!m_lastInvokedAction) {
        // when holding Meta
        m_lastInvokedAction = m_singleMetaAction;
    }

    QList<Latte::View *> sortedViews;
    CentralLayout *currentLayout = m_corona->layoutsManager()->currentLayout();

    if (currentLayout) {
        sortedViews = currentLayout->sortedLatteViews();
    }

    Latte::View *viewWithTasks{nullptr};
    Latte::View *viewWithMeta{nullptr};

    for(const auto view : sortedViews) {
        if (!viewWithTasks
                && view->interface()->isCapableToShowShortcutBadges()
                && (!view->layout()->preferredForShortcutsTouched() || view->isPreferredForShortcuts())) {
            viewWithTasks = view;
            break;
        }
    }

    if (m_corona->universalSettings()->kwin_metaForwardedToLatte()) {
        viewWithMeta = highestApplicationLauncherView(sortedViews);
    }

    bool viewFound{false};

    if (!m_hideViewsTimer.isActive()) {
        m_hideViews.clear();
    }

    //! show view that contains tasks plasmoid
    if (viewWithTasks) {
        viewFound = true;

        bool showMeta = (viewWithMeta && (viewWithMeta == viewWithTasks));

        viewWithTasks->interface()->showShortcutBadges(true, showMeta);

        if (!m_hideViewsTimer.isActive()) {
            m_hideViews.append(viewWithTasks);
            viewWithTasks->visibility()->setBlockHiding(true);
        }
    }

    //! show view that contains only meta
    if (viewWithMeta && viewWithMeta != viewWithTasks && viewWithMeta->interface()->showOnlyMeta()) {
        viewFound = true;

        if (!m_hideViewsTimer.isActive()) {
            m_hideViews.append(viewWithMeta);
            viewWithMeta->visibility()->setBlockHiding(true);
        }
    }

    //! show all the rest views that contain plasma shortcuts
    QList<Latte::View *> viewsWithShortcuts;

    if (currentLayout) {
        viewsWithShortcuts = currentLayout->viewsWithPlasmaShortcuts();
    }

    if (viewsWithShortcuts.count() > 0) {
        viewFound = true;

        if (!m_hideViewsTimer.isActive()) {
            for(const auto view : viewsWithShortcuts) {
                if (view != viewWithTasks && view != viewWithMeta) {
                    if (view->interface()->showShortcutBadges(false, false)) {
                        m_hideViews.append(view);
                        view->visibility()->setBlockHiding(true);
                    }
                }
            }
        }
    }

    if (viewFound) {
        if (!m_hideViewsTimer.isActive()) {
            m_hideViewsTimer.start();
        } else {
            m_hideViewsTimer.stop();
            hideViewsTimerSlot();
        }
    }
}

bool GlobalShortcuts::viewsToHideAreValid()
{
    for(const auto view : m_hideViews) {
        if (!m_corona->layoutsManager()->synchronizer()->latteViewExists(view)) {
            return false;
        }

    }

    return true;
}

void GlobalShortcuts::showSettings()
{
    QList<Latte::View *> sortedViews;
    CentralLayout *currentLayout = m_corona->layoutsManager()->currentLayout();

    if (currentLayout) {
        sortedViews = currentLayout->sortedLatteViews();
    }

    //! find which is the next view to show its settings
    if (sortedViews.count() > 0) {
        int openSettings = -1;

        //! check if there is a view with opened settings window
        for (int i = 0; i < sortedViews.size(); ++i) {
            if (sortedViews[i] == currentLayout->lastConfigViewFor()) {
                openSettings = i;
                break;
            }
        }

        if (openSettings >= 0 && sortedViews.count() > 1) {
            openSettings = currentLayout->configViewIsShown() ? openSettings + 1 : openSettings;

            if (openSettings >= sortedViews.size()) {
                openSettings = 0;
            }

            sortedViews[openSettings]->showSettingsWindow();
        } else {
            sortedViews[0]->showSettingsWindow();
        }
    }
}

void GlobalShortcuts::hideViewsTimerSlot()
{
    if (!m_lastInvokedAction || m_hideViews.count() == 0) {
        return;
    }

    auto initParameters = [this]() {
        m_lastInvokedAction = Q_NULLPTR;

        if (viewsToHideAreValid()) {
            for(const auto latteView : m_hideViews) {
                latteView->visibility()->setBlockHiding(false);
                latteView->interface()->hideShortcutBadges();
            }
        }

        m_hideViews.clear();
        m_metaShowedViews = false;
    };

    // qDebug() << "MEMORY ::: " << m_hideViews.count() << " _ " << m_viewItemsCalled.count() << " _ " << m_showShortcutBadgesMethods.count();

    if (QX11Info::isPlatformX11()) {
        if (!m_modifierTracker->sequenceModifierPressed(m_lastInvokedAction->shortcut())) {
            initParameters();

            return;
        } else {
            m_hideViewsTimer.start();
        }
    } else {
        // TODO: This is needs to be fixed in wayland
        initParameters();
    }
}

}

#include "moc_globalshortcuts.cpp"
