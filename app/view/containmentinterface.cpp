/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "containmentinterface.h"

// local
#include "view.h"
#include "../lattecorona.h"
#include "../layout/genericlayout.h"
#include "../layouts/importer.h"
#include "../layouts/storage.h"
#include "../settings/universalsettings.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QLatin1String>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>
#include <PlasmaQuick/AppletQuickItem>

// KDE
#include <KDesktopFile>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KDeclarative/ConfigPropertyMap>

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

    m_appletDelayedConfigurationTimer.setInterval(1000);
    m_appletDelayedConfigurationTimer.setSingleShot(true);
    connect(&m_appletDelayedConfigurationTimer, &QTimer::timeout, this, &ContainmentInterface::updateAppletDelayedConfiguration);

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
            if (item->objectName() == QLatin1String("containmentViewLayout")) {
                for (QQuickItem *subitem : item->childItems()) {
                    if (subitem->objectName() == QLatin1String("PositionShortcutsAbilityHost")) {
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
    const auto applets = m_view->containment()->applets();

    PlasmaQuick::AppletQuickItem *appLauncherItem{nullptr};

    for (auto applet : applets) {
        if (applet->id() == launcherAppletId) {
            appLauncherItem = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();
        }
    }

    return appLauncherItem && appletIsExpandable(appLauncherItem);
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

        if (meta.pluginId() == QLatin1String("org.kde.latte.plasmoid")) {

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
        return -1;
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

bool ContainmentInterface::appletIsExpandable(const int id) const
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return false;
    }

    for (const auto applet : m_view->containment()->applets()) {
        if (applet && applet->id() == (uint)id) {
            if (Layouts::Storage::self()->isSubContainment(m_view->corona(), applet)) {
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

bool ContainmentInterface::appletIsExpandable(PlasmaQuick::AppletQuickItem *appletQuickItem) const
{
    if (!appletQuickItem || !m_view->inReadyState()) {
        return false;
    }

    return ((appletQuickItem->fullRepresentation() != nullptr
            && appletQuickItem->preferredRepresentation() != appletQuickItem->fullRepresentation())
            || Latte::Layouts::Storage::self()->isSubContainment(m_view->corona(), appletQuickItem->applet()));
}

bool ContainmentInterface::appletIsActivationTogglesExpanded(const int id) const
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return false;
    }

    for (const auto applet : m_view->containment()->applets()) {
        if (applet && applet->id() == (uint)id) {
            if (Layouts::Storage::self()->isSubContainment(m_view->corona(), applet)) {
                return true;
            }

            PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai) {
                return ai->isActivationTogglesExpanded();
            }
        }
    }

    return false;
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

int ContainmentInterface::indexOfApplet(const int &id)
{
    if (m_appletOrder.contains(id)) {
        return m_appletOrder.indexOf(id);
    } else if (m_appletData.contains(id)) {
        return m_appletData[id].lastValidIndex;
    }

    return -1;
}

ViewPart::AppletInterfaceData ContainmentInterface::appletDataAtIndex(const int &index)
{
    ViewPart::AppletInterfaceData data;

    if (index<0 || (index > (m_appletOrder.count()-1))) {
        return data;
    }

    return m_appletData[m_appletOrder[index]];
}

ViewPart::AppletInterfaceData ContainmentInterface::appletDataForId(const int &id)
{
    ViewPart::AppletInterfaceData data;

    if (!m_appletData.contains(id)) {
        return data;
    }

    return m_appletData[id];
}

QObject *ContainmentInterface::plasmoid() const
{
    return m_plasmoid;
}

void ContainmentInterface::setPlasmoid(QObject *plasmoid)
{
    if (m_plasmoid == plasmoid) {
        return;
    }

    m_plasmoid = plasmoid;

    if (m_plasmoid) {
        m_configuration = qobject_cast<KDeclarative::ConfigPropertyMap *>(m_plasmoid->property("configuration").value<QObject *>());

        if (m_configuration) {
            connect(m_configuration, &QQmlPropertyMap::valueChanged, this, &ContainmentInterface::containmentConfigPropertyChanged);
        }
    }

    emit plasmoidChanged();
}

QObject *ContainmentInterface::layoutManager() const
{
    return m_layoutManager;
}

void ContainmentInterface::setLayoutManager(QObject *manager)
{
    if (m_layoutManager == manager) {
        return;
    }

    m_layoutManager = manager;

    // applets order
    int metaorderindex = m_layoutManager->metaObject()->indexOfProperty("order");
    if (metaorderindex >= 0) {
        QMetaProperty metaorder = m_layoutManager->metaObject()->property(metaorderindex);
        if (metaorder.hasNotifySignal()) {
            QMetaMethod metaorderchanged = metaorder.notifySignal();
            QMetaMethod metaupdateappletorder = this->metaObject()->method(this->metaObject()->indexOfSlot("updateAppletsOrder()"));
            connect(m_layoutManager, metaorderchanged, this, metaupdateappletorder);
            updateAppletsOrder();
        }
    }

    // applets in locked zoom
    metaorderindex = m_layoutManager->metaObject()->indexOfProperty("lockedZoomApplets");
    if (metaorderindex >= 0) {
        QMetaProperty metaorder = m_layoutManager->metaObject()->property(metaorderindex);
        if (metaorder.hasNotifySignal()) {
            QMetaMethod metaorderchanged = metaorder.notifySignal();
            QMetaMethod metaupdateapplets = this->metaObject()->method(this->metaObject()->indexOfSlot("updateAppletsInLockedZoom()"));
            connect(m_layoutManager, metaorderchanged, this, metaupdateapplets);
            updateAppletsInLockedZoom();
        }
    }

    // applets disabled their autocoloring
    metaorderindex = m_layoutManager->metaObject()->indexOfProperty("userBlocksColorizingApplets");
    if (metaorderindex >= 0) {
        QMetaProperty metaorder = m_layoutManager->metaObject()->property(metaorderindex);
        if (metaorder.hasNotifySignal()) {
            QMetaMethod metaorderchanged = metaorder.notifySignal();
            QMetaMethod metaupdateapplets = this->metaObject()->method(this->metaObject()->indexOfSlot("updateAppletsDisabledColoring()"));
            connect(m_layoutManager, metaorderchanged, this, metaupdateapplets);
            updateAppletsDisabledColoring();
        }
    }

    emit layoutManagerChanged();
}

void ContainmentInterface::addApplet(const QString &pluginId)
{
    if (pluginId.isEmpty()) {
        return;
    }

    QStringList paths = Latte::Layouts::Importer::standardPaths();
    QString pluginpath;

    for(int i=0; i<paths.count(); ++i) {
        QString cpath = paths[i] + "/plasma/plasmoids/" + pluginId;

        if (QDir(cpath).exists()) {
            pluginpath = cpath;
            break;
        }
    }

    if (!pluginpath.isEmpty()) {
        m_view->containment()->createApplet(pluginId);
    }
}

void ContainmentInterface::addApplet(QObject *metadata, int x, int y)
{
    int processmimedataindex = m_plasmoid->metaObject()->indexOfMethod("processMimeData(QObject*,int,int)");
    QMetaMethod processmethod = m_plasmoid->metaObject()->method(processmimedataindex);
    processmethod.invoke(m_plasmoid,
                         Q_ARG(QObject *, metadata),
                         Q_ARG(int, x),
                         Q_ARG(int, y));
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
        bool added{false};

        if (appletItem->isExpanded()) {
            if (appletItem->switchWidth()>0 && appletItem->switchHeight()>0) {
                added = ((appletItem->width()<=appletItem->switchWidth())
                         && (appletItem->height()<=appletItem->switchHeight()));
            } else {
                added = true;
            }
        }

        if (added && appletIsExpandable(appletItem)) {
            addExpandedApplet(appletItem);
        } else {
            removeExpandedApplet(appletItem);
        }
    }
}

QList<int> ContainmentInterface::appletsOrder() const
{
    return m_appletOrder;
}

void ContainmentInterface::updateAppletsOrder()
{
    if (!m_layoutManager) {
        return;
    }

    QList<int> neworder = m_layoutManager->property("order").value<QList<int>>();

    if (m_appletOrder == neworder) {
        return;
    }

    m_appletOrder = neworder;

    //! update applets last recorded index, this is needed for example when an applet is removed
    //! to know in which index was located before the removal
    for(const auto &id: m_appletOrder) {
        if (m_appletData.contains(id)) {
            m_appletData[id].lastValidIndex = m_appletOrder.indexOf(id);
        }
    }

    emit appletsOrderChanged();
}

void ContainmentInterface::updateAppletsInLockedZoom()
{
    if (!m_layoutManager) {
        return;
    }

    QList<int> appletslockedzoom = m_layoutManager->property("lockedZoomApplets").value<QList<int>>();

    if (m_appletsInLockedZoom == appletslockedzoom) {
        return;
    }

    m_appletsInLockedZoom = appletslockedzoom;
    emit appletsInLockedZoomChanged(m_appletsInLockedZoom);
}

void ContainmentInterface::updateAppletsDisabledColoring()
{
    if (!m_layoutManager) {
        return;
    }

    QList<int> appletsdisabledcoloring = m_layoutManager->property("userBlocksColorizingApplets").value<QList<int>>();

    if (m_appletsDisabledColoring == appletsdisabledcoloring) {
        return;
    }

    m_appletsDisabledColoring = appletsdisabledcoloring;
    emit appletsDisabledColoringChanged(appletsdisabledcoloring);
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

bool ContainmentInterface::appletIsExpanded(const int id) const
{
    return m_expandedAppletIds.values().contains(id);
}

void ContainmentInterface::toggleAppletExpanded(const int id)
{
    if (!m_view->containment() || !m_view->inReadyState()) {
        return;
    }

    for (const auto applet : m_view->containment()->applets()) {
        if (applet->id() == (uint)id && !Layouts::Storage::self()->isSubContainment(m_view->corona(), applet)/*block for sub-containments*/) {
            PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (ai) {
                emit applet->activated();
            }
        }
    }
}

void ContainmentInterface::removeApplet(const int &id)
{
    if (!m_appletData.contains(id)) {
        return;
    }

    auto applet = m_appletData[id].applet;
    emit applet->appletDeleted(applet); //! this signal should be part of Plasma Frameworks AppletPrivate::destroy() function...
    applet->destroy();
}


void ContainmentInterface::setAppletsOrder(const QList<int> &order)
{
    QMetaObject::invokeMethod(m_layoutManager,
                              "requestAppletsOrder",
                              Qt::DirectConnection,
                              Q_ARG(QList<int>, order));

}

void ContainmentInterface::setAppletsInLockedZoom(const QList<int> &applets)
{
    QMetaObject::invokeMethod(m_layoutManager,
                              "requestAppletsInLockedZoom",
                              Qt::DirectConnection,
                              Q_ARG(QList<int>, applets));
}

void ContainmentInterface::setAppletsDisabledColoring(const QList<int> &applets)
{
    QMetaObject::invokeMethod(m_layoutManager,
                              "requestAppletsDisabledColoring",
                              Qt::DirectConnection,
                              Q_ARG(QList<int>, applets));
}

void ContainmentInterface::setAppletInScheduledDestruction(const int &id, const bool &enabled)
{
    QMetaObject::invokeMethod(m_layoutManager,
                              "setAppletInScheduledDestruction",
                              Qt::DirectConnection,
                              Q_ARG(int, id),
                              Q_ARG(bool, enabled));
}

void ContainmentInterface::updateContainmentConfigProperty(const QString &key, const QVariant &value)
{
    if (!m_configuration || !m_configuration->keys().contains(key)) {

    }

    if (m_configuration->keys().contains(key)
            && (*m_configuration)[key] != value) {
        m_configuration->insert(key, value);
        emit m_configuration->valueChanged(key, value);
    }
}

void ContainmentInterface::updateAppletConfigProperty(const int &id, const QString &key, const QVariant &value)
{
    if (!m_appletData.contains(id) || !m_appletData[id].configuration || !m_appletData[id].configuration->keys().contains(key)) {
        return;
    }

    if (m_appletData[id].configuration->keys().contains(key)
            && (*m_appletData[id].configuration)[key] != value) {
        m_appletData[id].configuration->insert(key, value);
        emit m_appletData[id].configuration->valueChanged(key, value);
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

    emit initializationCompleted();
}

void ContainmentInterface::updateAppletDelayedConfiguration()
{
    for (const auto id : m_appletData.keys()) {
        if (!m_appletData[id].configuration) {
            m_appletData[id].configuration = appletConfiguration(m_appletData[id].applet);

            if (m_appletData[id].configuration) {
                qDebug() << "org.kde.sync delayed applet configuration was successful for : " << id;
                initAppletConfigurationSignals(id, m_appletData[id].configuration);
            }
        }
    }
}

void ContainmentInterface::initAppletConfigurationSignals(const int &id, KDeclarative::ConfigPropertyMap *configuration)
{
    if (!configuration) {
        return;
    }

    connect(configuration, &QQmlPropertyMap::valueChanged,
            this, [&, id](const QString &key, const QVariant &value) {
        //qDebug() << "org.kde.sync applet property changed : " << currentAppletId << " __ " << m_appletData[currentAppletId].plugin << " __ " << key << " __ " << value;
        emit appletConfigPropertyChanged(id, key, value);
    });
}

KDeclarative::ConfigPropertyMap *ContainmentInterface::appletConfiguration(const Plasma::Applet *applet)
{
    if (!m_view->containment() || !applet) {
        return nullptr;
    }

    PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();
    bool isSubContainment = Layouts::Storage::self()->isSubContainment(m_view->corona(), applet); //we use corona() to make sure that returns true even when it is first created from user
    int currentAppletId = applet->id();
    KDeclarative::ConfigPropertyMap *configuration{nullptr};

    //! set configuration object properly for applets and subcontainments
    if (!isSubContainment) {
        int metaconfigindex = ai->metaObject()->indexOfProperty("configuration");
        if (metaconfigindex >=0 ){
            configuration = qobject_cast<KDeclarative::ConfigPropertyMap *>((ai->property("configuration")).value<QObject *>());
        }
    } else {
        Plasma::Containment *subcontainment = Layouts::Storage::self()->subContainmentOf(m_view->corona(), applet);
        if (subcontainment) {
            PlasmaQuick::AppletQuickItem *subcai = subcontainment->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();

            if (subcai) {
                int metaconfigindex = subcai->metaObject()->indexOfProperty("configuration");
                if (metaconfigindex >=0 ){
                    configuration = qobject_cast<KDeclarative::ConfigPropertyMap *>((subcai->property("configuration")).value<QObject *>());
                }
            }
        }
    }

    return configuration;
}

void ContainmentInterface::onAppletAdded(Plasma::Applet *applet)
{
    if (!m_view->containment() || !applet) {
        return;
    }

    PlasmaQuick::AppletQuickItem *ai = applet->property("_plasma_graphicObject").value<PlasmaQuick::AppletQuickItem *>();
    bool isSubContainment = Layouts::Storage::self()->isSubContainment(m_view->corona(), applet); //we use corona() to make sure that returns true even when it is first created from user
    int currentAppletId = applet->id();

    //! Track expanded/able applets and Tasks applets
    if (isSubContainment) {
        //! internal containment case
        Plasma::Containment *subContainment = Layouts::Storage::self()->subContainmentOf(m_view->corona(), applet);
        PlasmaQuick::AppletQuickItem *contAi = ai;

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
    } else if (ai) {
        KPluginMetaData meta = applet->kPackage().metadata();
        const auto &provides = KPluginMetaData::readStringList(meta.rawData(), QStringLiteral("X-Plasma-Provides"));

        if (meta.pluginId() == QLatin1String("org.kde.latte.plasmoid")) {
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

    //! Track All Applets, for example to support syncing between different docks and panels
    if (ai) {
        bool initializing{!m_appletData.contains(currentAppletId)};

        KPluginMetaData meta = applet->kPackage().metadata();
        ViewPart::AppletInterfaceData data;
        data.id = currentAppletId;
        data.plugin = meta.pluginId();
        data.applet = applet;
        data.plasmoid = ai;
        data.lastValidIndex = m_appletOrder.indexOf(data.id);
        //! set configuration object properly for applets and subcontainments
        data.configuration = appletConfiguration(applet);

        //! track property changes in applets
        if (data.configuration) {
            initAppletConfigurationSignals(data.id, data.configuration);
        } else {
            qDebug() << "org.kde.sync Unfortunately configuration syncing for :: " << currentAppletId << " was not established, configuration object was missing!";
            m_appletDelayedConfigurationTimer.start();
        }

        if (initializing) {
            //! track applet destroyed flag
            connect(applet, &Plasma::Applet::destroyedChanged, this, [&, currentAppletId](bool destroyed) {
                emit appletInScheduledDestructionChanged(currentAppletId, destroyed);
            });

            //! remove on applet destruction
            connect(applet, &QObject::destroyed, this, [&, data](){
                emit appletRemoved(data.id);
                //qDebug() << "org.kde.sync: removing applet ::: " << data.id << " __ " << data.plugin << " remained : " << m_appletData.keys();
                m_appletData.remove(data.id);
            });
        }

        m_appletData[data.id] = data;
        emit appletDataCreated(data.id);
    }
}

QList<int> ContainmentInterface::toIntList(const QVariantList &list)
{
    QList<int> converted;

    for(const QVariant &item: list) {
        converted << item.toInt();
    }

    return converted;
}

}
}
