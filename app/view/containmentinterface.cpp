/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "containmentinterface.h"

// local
#include "view.h"
#include "../lattecorona.h"
#include "../settings/universalsettings.h"

// Qt
#include <QDebug>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>

// KDE
#include <KLocalizedString>
#include <KPluginMetaData>

namespace Latte {
namespace ViewPart {

ContainmentInterface::ContainmentInterface(Latte::View *parent)
    : QObject(parent),
      m_view(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(m_view->corona());
}

ContainmentInterface::~ContainmentInterface()
{
}

void ContainmentInterface::identifyMainItem()
{
    if (m_mainItem) {
        return;
    }

    if (QQuickItem *graphicItem = m_view->containment()->property("_plasma_graphicObject").value<QQuickItem *>()) {
        const auto &childItems = graphicItem->childItems();

        for (QQuickItem *item : childItems) {
            if (item->objectName() == "containmentViewLayout" ) {
                m_mainItem = item;
                identifyMethods();
                return;
            }
        }
    }
}

void ContainmentInterface::identifyMethods()
{
    int aeIndex = m_mainItem->metaObject()->indexOfMethod("activateEntryAtIndex(QVariant)");
    int niIndex = m_mainItem->metaObject()->indexOfMethod("newInstanceForEntryAtIndex(QVariant)");
    int sbIndex = m_mainItem->metaObject()->indexOfMethod("setShowAppletShortcutBadges(QVariant,QVariant,QVariant,QVariant)");
    int afiIndex = m_mainItem->metaObject()->indexOfMethod("appletIdForIndex(QVariant)");

    m_activateEntryMethod = m_mainItem->metaObject()->method(aeIndex);
    m_appletIdForIndexMethod = m_mainItem->metaObject()->method(afiIndex);
    m_newInstanceMethod = m_mainItem->metaObject()->method(niIndex);
    m_showShortcutsMethod = m_mainItem->metaObject()->method(sbIndex);
}

bool ContainmentInterface::applicationLauncherHasGlobalShortcut() const
{
    if (!containsApplicationLauncher()) {
        return false;
    }

    int launcherAppletId = applicationLauncherId();

    const auto applets = m_view->containment()->applets();

    for (auto applet : applets) {
        if (applet->id() == launcherAppletId) {
            return !applet->globalShortcut().isEmpty();
        }
    }

    return false;
}

bool ContainmentInterface::applicationLauncherInPopup() const
{
    if (!containsApplicationLauncher()) {
        return false;
    }

    int launcherAppletId = applicationLauncherId();
    QString launcherPluginId;

    const auto applets = m_view->containment()->applets();

    for (auto applet : applets) {
        if (applet->id() == launcherAppletId) {
            launcherPluginId = applet->kPackage().metadata().pluginId();
        }
    }

    return launcherPluginId != "org.kde.plasma.kickerdash";
}

bool ContainmentInterface::containsApplicationLauncher() const
{
    return (applicationLauncherId() >= 0);
}

bool ContainmentInterface::isCapableToShowShortcutBadges()
{
    identifyMainItem();

    if (!m_view->latteTasksArePresent() && m_view->tasksPresent()) {
        return false;
    }

    return m_showShortcutsMethod.isValid();
}

int ContainmentInterface::applicationLauncherId() const
{
    const auto applets = m_view->containment()->applets();

    auto launcherId{-1};

    for (auto applet : applets) {
        const auto provides = applet->kPackage().metadata().value(QStringLiteral("X-Plasma-Provides"));

        if (provides.contains(QLatin1String("org.kde.plasma.launchermenu"))) {
            if (!applet->globalShortcut().isEmpty()) {
                return applet->id();
            } else if (launcherId == -1) {
                launcherId = applet->id();
            }
        }
    }

    return launcherId;
}

bool ContainmentInterface::updateBadgeForLatteTask(const QString identifier, const QString value)
{
    if (!m_view->latteTasksArePresent()) {
        return false;
    }

    const auto &applets = m_view->containment()->applets();

    for (auto *applet : applets) {
        KPluginMetaData meta = applet->kPackage().metadata();

        if (meta.pluginId() == "org.kde.latte.plasmoid") {

            if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
                const auto &childItems = appletInterface->childItems();

                if (childItems.isEmpty()) {
                    continue;
                }

                for (QQuickItem *item : childItems) {
                    if (auto *metaObject = item->metaObject()) {
                        // not using QMetaObject::invokeMethod to avoid warnings when calling
                        // this on applets that don't have it or other child items since this
                        // is pretty much trial and error.
                        // Also, "var" arguments are treated as QVariant in QMetaObject

                        int methodIndex = metaObject->indexOfMethod("updateBadge(QVariant,QVariant)");

                        if (methodIndex == -1) {
                            continue;
                        }

                        QMetaMethod method = metaObject->method(methodIndex);

                        if (method.invoke(item, Q_ARG(QVariant, identifier), Q_ARG(QVariant, value))) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool ContainmentInterface::activatePlasmaTask(const int index)
{
    bool containsPlasmaTaskManager{m_view->tasksPresent() && !m_view->latteTasksArePresent()};

    if (!containsPlasmaTaskManager) {
        return false;
    }

    const auto &applets = m_view->containment()->applets();

    for (auto *applet : applets) {
        const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

        if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
            if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
                const auto &childItems = appletInterface->childItems();

                if (childItems.isEmpty()) {
                    continue;
                }

                KPluginMetaData meta = applet->kPackage().metadata();

                for (QQuickItem *item : childItems) {
                    if (auto *metaObject = item->metaObject()) {
                        int methodIndex{metaObject->indexOfMethod("activateTaskAtIndex(QVariant)")};

                        if (methodIndex == -1) {
                            continue;
                        }

                        QMetaMethod method = metaObject->method(methodIndex);

                        if (method.invoke(item, Q_ARG(QVariant, index - 1))) {
                            showShortcutBadges(false, true);

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool ContainmentInterface::newInstanceForPlasmaTask(const int index)
{
    bool containsPlasmaTaskManager{m_view->tasksPresent() && !m_view->latteTasksArePresent()};

    if (!containsPlasmaTaskManager) {
        return false;
    }

    const auto &applets = m_view->containment()->applets();

    for (auto *applet : applets) {
        const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

        if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
            if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
                const auto &childItems = appletInterface->childItems();

                if (childItems.isEmpty()) {
                    continue;
                }

                KPluginMetaData meta = applet->kPackage().metadata();

                for (QQuickItem *item : childItems) {
                    if (auto *metaObject = item->metaObject()) {
                        int methodIndex{metaObject->indexOfMethod("ewInstanceForTaskAtIndex(QVariant)")};

                        if (methodIndex == -1) {
                            continue;
                        }

                        QMetaMethod method = metaObject->method(methodIndex);

                        if (method.invoke(item, Q_ARG(QVariant, index - 1))) {
                            showShortcutBadges(false, true);

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool ContainmentInterface::activateEntry(const int index)
{
    identifyMainItem();

    if (!m_activateEntryMethod.isValid()) {
        return false;
    }

    return m_activateEntryMethod.invoke(m_mainItem, Q_ARG(QVariant, index));
}

bool ContainmentInterface::newInstanceForEntry(const int index)
{
    identifyMainItem();

    if (!m_newInstanceMethod.isValid()) {
        return false;
    }

    return m_newInstanceMethod.invoke(m_mainItem, Q_ARG(QVariant, index));
}

bool ContainmentInterface::hideShortcutBadges()
{
    identifyMainItem();

    if (!m_showShortcutsMethod.isValid()) {
        return false;
    }

    return m_showShortcutsMethod.invoke(m_mainItem, Q_ARG(QVariant, false), Q_ARG(QVariant, false), Q_ARG(QVariant, false), Q_ARG(QVariant, -1));
}

bool ContainmentInterface::showOnlyMeta()
{
    if (!m_corona->universalSettings()->kwin_metaForwardedToLatte()) {
        return false;
    }

    return showShortcutBadges(false, true);
}

bool ContainmentInterface::showShortcutBadges(const bool showLatteShortcuts, const bool showMeta)
{
    identifyMainItem();

    if (!m_showShortcutsMethod.isValid()) {
        return false;
    }

    int appLauncherId = m_corona->universalSettings()->kwin_metaForwardedToLatte() && showMeta ? applicationLauncherId() : -1;

    return m_showShortcutsMethod.invoke(m_mainItem, Q_ARG(QVariant, showLatteShortcuts), Q_ARG(QVariant, true), Q_ARG(QVariant, showMeta), Q_ARG(QVariant, appLauncherId));
}

int ContainmentInterface::appletIdForIndex(const int index)
{
    identifyMainItem();

    if (!m_appletIdForIndexMethod.isValid()) {
        return false;
    }

    QVariant appletId{-1};

    m_appletIdForIndexMethod.invoke(m_mainItem, Q_RETURN_ARG(QVariant, appletId), Q_ARG(QVariant, index));

    return appletId.toInt();
}


}
}
