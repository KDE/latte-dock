/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWCONTAINMENTINTERFACE_H
#define VIEWCONTAINMENTINTERFACE_H

// local
#include "tasksmodel.h"

// Qt
#include <QHash>
#include <QMetaMethod>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QTimer>
#include <QUrl>

namespace Plasma {
class Applet;
}

namespace PlasmaQuick {
class AppletQuickItem;
}

namespace KDeclarative {
class ConfigPropertyMap;
}

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {

struct AppletInterfaceData
{
    int id{-1};
    QString plugin;
    int lastValidIndex{-1};
    Plasma::Applet *applet{nullptr};
    PlasmaQuick::AppletQuickItem *plasmoid{nullptr};
    KDeclarative::ConfigPropertyMap *configuration{nullptr};
};

class ContainmentInterface: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasExpandedApplet READ hasExpandedApplet NOTIFY hasExpandedAppletChanged)
    Q_PROPERTY(bool hasLatteTasks READ hasLatteTasks NOTIFY hasLatteTasksChanged)
    Q_PROPERTY(bool hasPlasmaTasks READ hasPlasmaTasks NOTIFY hasPlasmaTasksChanged)

    Q_PROPERTY(QObject *plasmoid READ plasmoid() WRITE setPlasmoid NOTIFY plasmoidChanged)

    Q_PROPERTY(QAbstractListModel *latteTasksModel READ latteTasksModel() NOTIFY latteTasksModelChanged)
    Q_PROPERTY(QAbstractListModel *plasmaTasksModel READ plasmaTasksModel() NOTIFY plasmaTasksModelChanged)

    //! specified from containment qml side
    Q_PROPERTY(QObject* layoutManager READ layoutManager WRITE setLayoutManager NOTIFY layoutManagerChanged)

public:
    ContainmentInterface(Latte::View *parent);
    virtual ~ContainmentInterface();

    bool hasExpandedApplet() const;
    bool hasLatteTasks() const;
    bool hasPlasmaTasks() const;

    bool applicationLauncherInPopup() const;
    bool applicationLauncherHasGlobalShortcut() const;
    bool containsApplicationLauncher() const;
    bool isCapableToShowShortcutBadges();

    bool activateEntry(const int index);
    bool newInstanceForEntry(const int index);

    bool activatePlasmaTask(const int index);
    bool newInstanceForPlasmaTask(const int index);

    bool hideShortcutBadges();
    bool showOnlyMeta();
    bool showShortcutBadges(const bool showLatteShortcuts, const bool showMeta);

    //! this is updated from external apps e.g. a thunderbird plugin
    bool updateBadgeForLatteTask(const QString identifier, const QString value);

    int applicationLauncherId() const;
    int appletIdForVisualIndex(const int index);

    int indexOfApplet(const int &id);
    QList<int> appletsOrder() const;
    ViewPart::AppletInterfaceData appletDataAtIndex(const int &index);
    ViewPart::AppletInterfaceData appletDataForId(const int &id);

    QObject *plasmoid() const;
    void setPlasmoid(QObject *plasmoid);

    QObject *layoutManager() const;
    void setLayoutManager(QObject *manager);

    QAbstractListModel *latteTasksModel() const;
    QAbstractListModel *plasmaTasksModel() const;

public slots:
    Q_INVOKABLE void deactivateApplets();
    Q_INVOKABLE void toggleAppletExpanded(const int id);

    Q_INVOKABLE bool appletIsExpandable(const int id) const;
    Q_INVOKABLE bool appletIsExpanded(const int id) const;
    Q_INVOKABLE bool appletIsActivationTogglesExpanded(const int id) const;

    Q_INVOKABLE bool isApplication(const QUrl &url) const;

    void addApplet(const QString &pluginId);
    void addApplet(QObject *metadata, int x, int y);
    void removeApplet(const int &id);
    void setAppletsOrder(const QList<int> &order);
    void setAppletsInLockedZoom(const QList<int> &applets);
    void setAppletsDisabledColoring(const QList<int> &applets);
    void setAppletInScheduledDestruction(const int &id, const bool &enabled);
    void updateContainmentConfigProperty(const QString &key, const QVariant &value);
    void updateAppletConfigProperty(const int &id, const QString &key, const QVariant &value);    

signals:
    void expandedAppletStateChanged();
    void hasExpandedAppletChanged();
    void hasLatteTasksChanged();
    void hasPlasmaTasksChanged();
    void initializationCompleted();
    void latteTasksModelChanged();
    void layoutManagerChanged();
    void plasmaTasksModelChanged();
    void plasmoidChanged();

    //! syncing signals
    void appletRemoved(const int &id);

    void appletConfigPropertyChanged(const int &id, const QString &key, const QVariant &value);
    void appletCreated(const QString &pluginId);
    void appletDataCreated(const int &id);
    void appletDropped(QObject *data, int x, int y);
    void containmentConfigPropertyChanged(const QString &key, const QVariant &value);
    void appletsOrderChanged();
    void appletsInLockedZoomChanged(const QList<int> &applets);
    void appletsDisabledColoringChanged(const QList<int> &applets);
    void appletInScheduledDestructionChanged(const int &id, const bool &enabled);

    void appletRequestedVisualIndicator(const int &plasmoidId);

private slots:
    void identifyShortcutsHost();
    void identifyMethods();

    void updateAppletsOrder();
    void updateAppletsInLockedZoom();
    void updateAppletsDisabledColoring();
    void updateAppletsTracking();
    void updateAppletDelayedConfiguration();

    void onAppletAdded(Plasma::Applet *applet);
    void onAppletExpandedChanged();
    void onLatteTasksCountChanged();
    void onPlasmaTasksCountChanged();

private:
    void addExpandedApplet(PlasmaQuick::AppletQuickItem * appletQuickItem);
    void removeExpandedApplet(PlasmaQuick::AppletQuickItem *appletQuickItem);
    void initAppletConfigurationSignals(const int &id, KDeclarative::ConfigPropertyMap *configuration);

    bool appletIsExpandable(PlasmaQuick::AppletQuickItem *appletQuickItem) const;

    KDeclarative::ConfigPropertyMap *appletConfiguration(const Plasma::Applet *applet);

    QList<int> toIntList(const QVariantList &list);

private:
    bool m_hasLatteTasks{false};
    bool m_hasPlasmaTasks{false};

    QMetaMethod m_activateEntryMethod;
    QMetaMethod m_appletIdForIndexMethod;
    QMetaMethod m_newInstanceMethod;
    QMetaMethod m_showShortcutsMethod;

    QPointer<Latte::Corona> m_corona;
    QPointer<Latte::View> m_view;
    QPointer<QQuickItem> m_shortcutsHost;

    //! startup timer to initialize
    //! applets tracking
    QTimer m_appletsExpandedConnectionsTimer;

    TasksModel *m_latteTasksModel;
    TasksModel *m_plasmaTasksModel;

    //!follow containment plasmoid
    QPointer<QObject> m_plasmoid;
    QPointer<QObject> m_layoutManager;
    QPointer<KDeclarative::ConfigPropertyMap> m_configuration;

    //!keep record of applet ids and avoid crashes when trying to access ids for already destroyed applets
    QHash<PlasmaQuick::AppletQuickItem *, int> m_expandedAppletIds;
    QHash<PlasmaQuick::AppletQuickItem *, QMetaObject::Connection> m_appletsExpandedConnections;

    //!all applet data
    QList<int> m_appletOrder; //includes justify splitters
    QList<int> m_appletsInLockedZoom;
    QList<int> m_appletsDisabledColoring;
    QHash<int, ViewPart::AppletInterfaceData> m_appletData;
    QTimer m_appletDelayedConfigurationTimer;
};

}
}

#endif
