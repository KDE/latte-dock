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
#include "../layouts/storage.h"
#include "../settings/universalsettings.h"

// Qt
#include <QDebug>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>
#include <PlasmaQuick/AppletQuickItem>

// KDE
#include <KDesktopFile>
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
            connect(m_view->containment(), &Plasma::Containment::appletAdded, this, &ContainmentInterface::onAppletAdded);

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

bool ContainmentInterface::isApplication(const QUrl &url) const
{
    if (!url.isValid() || !url.isLocalFile()) {
        return false;
    }

    const QString &localPath = url.toLocalFile();

    if (!KDesktopFile::isDesktopFile(localPath)) {
        return false;
    }

    KDesktopFile desktopFile(localPath);
    return desktopFile.hasApplicationType();
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
            if (Layouts::Storage::self()->isSubContainment(m_view->layout(), applet)) {
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

void ContainmentInterface::addExpandedApplet(PlasmaQuick::AppletQuickItem * appletQuickItem)
{
    if (appletQuickItem && m_expandedAppletIds.contains(appletQuickItem) && appletIsExpandable(appletQuickItem)) {
        return;
    }

    bool isExpanded = hasExpandedApplet();

    m_expandedAppletIds[appletQuickItem] = appletQuickItem->applet()->id();

    if (isExpanded != hasExpandedApplet()) {
        emit hasExpandedAppletChanged();
    }

    emit expandedAppletStateChanged();
}

void ContainmentInterface::removeExpandedApplet(PlasmaQuick::AppletQuickItem *appletQuickItem)
{
    if (!m_expandedAppletIds.contains(appletQuickItem)) {
        return;
    }

    bool isExpanded = hasExpandedApplet();

    m_expandedAppletIds.remove(appletQuickItem);

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

void ContainmentInterface::onAppletExpandedChanged()
{
    PlasmaQuick::AppletQuickItem *appletItem = static_cast<PlasmaQuick::AppletQuickItem *>(QObject::sender());

    if (appletItem) {
        if (appletItem->isExpanded()) {
            addExpandedApplet(appletItem);
        } else {
            removeExpandedApplet(appletItem);
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
    return m_expandedAppletIds.values().contains(id);
}

void ContainmentInterface::toggleAppletExpanded(const int id)
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return;
    }

    for (const auto applet : m_view->containment()->applets()) {
        if (applet->id() == (uint)id && !Layouts::Storage::self()->isSubContainment(m_view->layout(), applet)/*block for sub-containments*/) {
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
        onAppletAdded(applet);
    }
}

void ContainmentInterface::onAppletAdded(Plasma::Applet *applet)
{
    if (!m_view->containment() || !applet) {
        return;
    }

    if (Layouts::Storage::self()->isSubContainment(m_view->layout(), applet)) {
        //! internal containment case
        Plasma::Containment *subContainment = Layouts::Storage::self()->subContainmentOf(m_view->layout(), applet);
        PlasmaQuick::AppletQuickItem *contAi = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

        if (contAi && !m_appletsExpandedConnections.contains(contAi)) {
            m_appletsExpandedConnections[contAi] = connect(contAi, &PlasmaQuick::AppletQuickItem::expandedChanged, this, &ContainmentInterface::onAppletExpandedChanged);

            connect(contAi, &QObject::destroyed, this, [&, contAi](){
                m_appletsExpandedConnections.remove(contAi);
                removeExpandedApplet(contAi);
            });
        }

        for (const auto internalApplet : subContainment->applets()) {
            PlasmaQuick::AppletQuickItem *ai = internalApplet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai && !m_appletsExpandedConnections.contains(ai) ){
                m_appletsExpandedConnections[ai] = connect(ai, &PlasmaQuick::AppletQuickItem::expandedChanged, this, &ContainmentInterface::onAppletExpandedChanged);

                connect(ai, &QObject::destroyed, this, [&, ai](){
                    m_appletsExpandedConnections.remove(ai);
                    removeExpandedApplet(ai);
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
            m_appletsExpandedConnections[ai] = connect(ai, &PlasmaQuick::AppletQuickItem::expandedChanged, this, &ContainmentInterface::onAppletExpandedChanged);

            connect(ai, &QObject::destroyed, this, [&, ai](){
                m_appletsExpandedConnections.remove(ai);
                removeExpandedApplet(ai);
            });
        }
    }

}

void ContainmentInterface::moveAppletsInJustifyAlignment(QQuickItem *start, QQuickItem *main, QQuickItem *end)
{
    if (!start || !main || !end) {
        return;
    }

    QList<QQuickItem *> appletlist;

    appletlist << start->childItems();
    appletlist << main->childItems();
    appletlist << end->childItems();

    bool firstSplitterFound{false};
    bool secondSplitterFound{false};
    int splitter1{-1};
    int splitter2{-1};

    for(int i=0; i<appletlist.count(); ++i) {
        bool issplitter = appletlist[i]->property("isInternalViewSplitter").toBool();

        if (!firstSplitterFound) {
            appletlist[i]->setParentItem(start);
            if (issplitter) {
                firstSplitterFound = true;
                splitter1 = i;
            }
        } else if (firstSplitterFound && !secondSplitterFound) {
            if (issplitter) {
                secondSplitterFound = true;
                splitter2 = i;
                appletlist[i]->setParentItem(end);
            } else {
                appletlist[i]->setParentItem(main);
            }
        } else if (firstSplitterFound && secondSplitterFound) {
            appletlist[i]->setParentItem(end);
        }
    }

    for(int i=0; i<appletlist.count()-1; ++i) {
        QQuickItem *before = appletlist[i];
        QQuickItem *after = appletlist[i+1];

        if (before->parentItem() == after->parentItem()) {
            before->stackBefore(after);
        }
    }

    //! Confirm Last item of End Layout
    if (end->childItems().count() > 0) {
        QQuickItem *lastItem = end->childItems()[end->childItems().count()-1];

        int correctpos{-1};

        for(int i=0; i<appletlist.count()-1; ++i) {
            if (lastItem == appletlist[i]) {
                correctpos = i;
                break;
            }
        }

        if (correctpos>=0) {
            lastItem->stackBefore(appletlist[correctpos+1]);
        }
    }
}

}
}
