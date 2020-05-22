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
#include "../layout/genericlayout.h"
#include "../settings/universalsettings.h"

// Qt
#include <QDebug>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>
#include <PlasmaQuick/AppletQuickItem>

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

    m_latteTasksModel = new TasksModel(this);
    m_plasmaTasksModel = new TasksModel(this);

    m_appletsExpandedConnectionsTimer.setInterval(2000);
    m_appletsExpandedConnectionsTimer.setSingleShot(true);

    connect(&m_appletsExpandedConnectionsTimer, &QTimer::timeout, this, &ContainmentInterface::updateAppletsTracking);

    connect(m_view, &View::containmentChanged
            , this, [&]() {
        if (m_view->containment()) {
            connect(m_view->containment(), &Plasma::Containment::appletAdded, this, &ContainmentInterface::on_appletAdded);

            m_appletsExpandedConnectionsTimer.start();
        }
    });

    connect(m_latteTasksModel, &TasksModel::countChanged, this, &ContainmentInterface::onLatteTasksCountChanged);
    connect(m_plasmaTasksModel, &TasksModel::countChanged, this, &ContainmentInterface::onPlasmaTasksCountChanged);
}

ContainmentInterface::~ContainmentInterface()
{
}

void ContainmentInterface::identifyShortcutsHost()
{
    if (m_shortcutsHost) {
        return;
    }

    if (QQuickItem *graphicItem = m_view->containment()->property("_plasma_graphicObject").value<QQuickItem *>()) {
        const auto &childItems = graphicItem->childItems();

        for (QQuickItem *item : childItems) {
            if (item->objectName() == "containmentViewLayout" ) {
                for (QQuickItem *subitem : item->childItems()) {
                    if (subitem->objectName() == "PositionShortcutsAbilityHost") {
                        m_shortcutsHost = subitem;
                        identifyMethods();
                        return;
                    }
                }
            }
        }
    }
}

void ContainmentInterface::identifyMethods()
{
    int aeIndex = m_shortcutsHost->metaObject()->indexOfMethod("activateEntryAtIndex(QVariant)");
    int niIndex = m_shortcutsHost->metaObject()->indexOfMethod("newInstanceForEntryAtIndex(QVariant)");
    int sbIndex = m_shortcutsHost->metaObject()->indexOfMethod("setShowAppletShortcutBadges(QVariant,QVariant,QVariant,QVariant)");
    int afiIndex = m_shortcutsHost->metaObject()->indexOfMethod("appletIdForIndex(QVariant)");

    m_activateEntryMethod = m_shortcutsHost->metaObject()->method(aeIndex);
    m_appletIdForIndexMethod = m_shortcutsHost->metaObject()->method(afiIndex);
    m_newInstanceMethod = m_shortcutsHost->metaObject()->method(niIndex);
    m_showShortcutsMethod = m_shortcutsHost->metaObject()->method(sbIndex);
}

bool ContainmentInterface::applicationLauncherHasGlobalShortcut() const
{
    if (!containsApplicationLauncher()) {
        return false;
    }

    uint launcherAppletId = applicationLauncherId();

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

    uint launcherAppletId = applicationLauncherId();
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
    identifyShortcutsHost();

    if (!hasLatteTasks() && hasPlasmaTasks()) {
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
    if (!hasLatteTasks()) {
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
    bool containsPlasmaTaskManager{hasPlasmaTasks() && !hasLatteTasks()};

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
    bool containsPlasmaTaskManager{hasPlasmaTasks() && !hasLatteTasks()};

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
                        int methodIndex{metaObject->indexOfMethod("newInstanceForTaskAtIndex(QVariant)")};

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
    identifyShortcutsHost();

    if (!m_activateEntryMethod.isValid()) {
        return false;
    }

    return m_activateEntryMethod.invoke(m_shortcutsHost, Q_ARG(QVariant, index));
}

bool ContainmentInterface::newInstanceForEntry(const int index)
{
    identifyShortcutsHost();

    if (!m_newInstanceMethod.isValid()) {
        return false;
    }

    return m_newInstanceMethod.invoke(m_shortcutsHost, Q_ARG(QVariant, index));
}

bool ContainmentInterface::hideShortcutBadges()
{
    identifyShortcutsHost();

    if (!m_showShortcutsMethod.isValid()) {
        return false;
    }

    return m_showShortcutsMethod.invoke(m_shortcutsHost, Q_ARG(QVariant, false), Q_ARG(QVariant, false), Q_ARG(QVariant, false), Q_ARG(QVariant, -1));
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
    identifyShortcutsHost();

    if (!m_showShortcutsMethod.isValid() || !isCapableToShowShortcutBadges()) {
        return false;
    }

    int appLauncherId = m_corona->universalSettings()->kwin_metaForwardedToLatte() && showMeta ? applicationLauncherId() : -1;

    return m_showShortcutsMethod.invoke(m_shortcutsHost, Q_ARG(QVariant, showLatteShortcuts), Q_ARG(QVariant, true), Q_ARG(QVariant, showMeta), Q_ARG(QVariant, appLauncherId));
}

int ContainmentInterface::appletIdForVisualIndex(const int index)
{
    identifyShortcutsHost();

    if (!m_appletIdForIndexMethod.isValid()) {
        return false;
    }

    QVariant appletId{-1};

    m_appletIdForIndexMethod.invoke(m_shortcutsHost, Q_RETURN_ARG(QVariant, appletId), Q_ARG(QVariant, index));

    return appletId.toInt();
}


void ContainmentInterface::deactivateApplets()
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return;
    }

    for (const auto applet : m_view->containment()->applets()) {
        PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

        if (ai) {
            ai->setExpanded(false);
        }
    }
}

bool ContainmentInterface::appletIsExpandable(const int id)
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return false;
    }

    for (const auto applet : m_view->containment()->applets()) {
        if (applet && applet->id() == (uint)id) {
            if (m_view->layout() && m_view->layout()->isInternalContainment(applet)) {
                return true;
            }

            PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai) {
                return appletIsExpandable(ai);
            }
        }
    }

    return false;
}

bool ContainmentInterface::appletIsExpandable(PlasmaQuick::AppletQuickItem *appletQuickItem)
{
    if (!appletQuickItem || !m_view->inReadyState()) {
        return false;
    }

    return (appletQuickItem->fullRepresentation() != nullptr
            && appletQuickItem->preferredRepresentation() != appletQuickItem->fullRepresentation());
}

bool ContainmentInterface::hasExpandedApplet() const
{
    return m_expandedAppletIds.count() > 0;
}

bool ContainmentInterface::hasLatteTasks() const
{
    return (m_latteTasksModel->count() > 0);
}

bool ContainmentInterface::hasPlasmaTasks() const
{
    return (m_plasmaTasksModel->count() > 0);
}

void ContainmentInterface::addExpandedApplet(const int &id)
{
    if (m_expandedAppletIds.contains(id) && appletIsExpandable(id)) {
        return;
    }

    bool isExpanded = hasExpandedApplet();

    m_expandedAppletIds << id;

    if (isExpanded != hasExpandedApplet()) {
        emit hasExpandedAppletChanged();
    }

    emit expandedAppletStateChanged();
}

void ContainmentInterface::removeExpandedApplet(const int &id)
{
    if (!m_expandedAppletIds.contains(id)) {
        return;
    }

    bool isExpanded = hasExpandedApplet();

    m_expandedAppletIds.removeAll(id);

    if (isExpanded != hasExpandedApplet()) {
        emit hasExpandedAppletChanged();
    }

    emit expandedAppletStateChanged();
}

QAbstractListModel *ContainmentInterface::latteTasksModel() const
{
    return m_latteTasksModel;
}

QAbstractListModel *ContainmentInterface::plasmaTasksModel() const
{
    return m_plasmaTasksModel;
}

void ContainmentInterface::on_appletExpandedChanged()
{
    PlasmaQuick::AppletQuickItem *appletItem = static_cast<PlasmaQuick::AppletQuickItem *>(QObject::sender());

    if (appletItem) {
        if (appletItem->isExpanded()) {
            addExpandedApplet(appletItem->applet()->id());
        } else {
            removeExpandedApplet(appletItem->applet()->id());
        }
    }
}

void ContainmentInterface::onLatteTasksCountChanged()
{
    if ((m_hasLatteTasks && m_latteTasksModel->count()>0)
            || (!m_hasLatteTasks && m_latteTasksModel->count() == 0)) {
        return;
    }

    m_hasLatteTasks = (m_latteTasksModel->count() > 0);
    emit hasLatteTasksChanged();
}

void ContainmentInterface::onPlasmaTasksCountChanged()
{
    if ((m_hasPlasmaTasks && m_plasmaTasksModel->count()>0)
            || (!m_hasPlasmaTasks && m_plasmaTasksModel->count() == 0)) {
        return;
    }

    m_hasPlasmaTasks = (m_plasmaTasksModel->count() > 0);
    emit hasPlasmaTasksChanged();
}

bool ContainmentInterface::appletIsExpanded(const int id)
{
    return m_expandedAppletIds.contains(id);
}

void ContainmentInterface::toggleAppletExpanded(const int id)
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return;
    }

    for (const auto applet : m_view->containment()->applets()) {
        if (applet->id() == (uint)id && !m_view->layout()->isInternalContainment(applet)/*block for internal containments*/) {
            PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai) {
                if (appletIsExpandable(ai)) {
                    ai->setExpanded(!ai->isExpanded());
                } else {
                    emit applet->activated();
                }
            }
        }
    }
}

void ContainmentInterface::updateAppletsTracking()
{
    if (!m_view->containment()) {
        return;
    }

    for (const auto applet : m_view->containment()->applets()) {
        on_appletAdded(applet);
    }
}

void ContainmentInterface::on_appletAdded(Plasma::Applet *applet)
{
    if (!m_view->containment() || !applet) {
        return;
    }

    if (m_view->layout() && m_view->layout()->isInternalContainment(applet)) {
        //! internal containment case
        Plasma::Containment *internalC = m_view->layout()->internalContainmentOf(applet);
        PlasmaQuick::AppletQuickItem *contAi = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

        if (contAi && !m_appletsExpandedConnections.contains(contAi)) {
            m_appletsExpandedConnections[contAi] = connect(contAi, &PlasmaQuick::AppletQuickItem::expandedChanged, this, &ContainmentInterface::on_appletExpandedChanged);

            connect(contAi, &QObject::destroyed, this, [&, contAi](){
                m_appletsExpandedConnections.remove(contAi);
                removeExpandedApplet(contAi->applet()->id());
            });
        }

        for (const auto internalApplet : internalC->applets()) {
            PlasmaQuick::AppletQuickItem *ai = internalApplet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai && !m_appletsExpandedConnections.contains(ai) ){
                m_appletsExpandedConnections[ai] = connect(ai, &PlasmaQuick::AppletQuickItem::expandedChanged, this, &ContainmentInterface::on_appletExpandedChanged);

                connect(ai, &QObject::destroyed, this, [&, ai](){
                    m_appletsExpandedConnections.remove(ai);
                    removeExpandedApplet(ai->applet()->id());
                });
            }
        }
    } else {
        PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

        if (!ai) {
            return;
        }

        KPluginMetaData meta = applet->kPackage().metadata();
        const auto &provides = KPluginMetaData::readStringList(meta.rawData(), QStringLiteral("X-Plasma-Provides"));

        if (meta.pluginId() == "org.kde.latte.plasmoid") {
            //! populate latte tasks applet
            m_latteTasksModel->addTask(ai);
        } else if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
            //! populate plasma tasks applet
            m_plasmaTasksModel->addTask(ai);
        } else if (!m_appletsExpandedConnections.contains(ai)) {
            m_appletsExpandedConnections[ai] = connect(ai, &PlasmaQuick::AppletQuickItem::expandedChanged, this, &ContainmentInterface::on_appletExpandedChanged);

            connect(ai, &QObject::destroyed, this, [&, ai](){
                m_appletsExpandedConnections.remove(ai);
                removeExpandedApplet(ai->applet()->id());
            });
        }
    }

}


}
}
