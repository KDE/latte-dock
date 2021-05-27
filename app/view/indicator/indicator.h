/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWINDICATOR_H
#define VIEWINDICATOR_H

// local
#include "indicatorinfo.h"
#include "indicatorresources.h"

// Qt
#include <QObject>
#include <QPointer>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>

// KDE
#include <KConfigLoader>
#include <KPluginMetaData>

namespace KDeclarative
{
class ConfigPropertyMap;
class QmlObjectSharedEngine;
}

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {

class Indicator: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool enabledForApplets READ enabledForApplets WRITE setEnabledForApplets NOTIFY enabledForAppletsChanged)
    Q_PROPERTY(bool latteTasksArePresent READ latteTasksArePresent NOTIFY latteTasksArePresentChanged)
    Q_PROPERTY(bool pluginIsReady READ pluginIsReady NOTIFY pluginIsReadyChanged)

    Q_PROPERTY(QString type READ type WRITE setType NOTIFY pluginChanged)
    Q_PROPERTY(QString customType READ customType NOTIFY customPluginChanged)

    /* Custom plugins */
    Q_PROPERTY(int customPluginsCount READ customPluginsCount NOTIFY customPluginsChanged)
    Q_PROPERTY(QStringList customPluginIds READ customPluginIds NOTIFY customPluginsChanged)
    Q_PROPERTY(QStringList customPluginNames READ customPluginNames NOTIFY customPluginsChanged)
    Q_PROPERTY(QStringList customLocalPluginIds READ customLocalPluginIds NOTIFY customPluginsChanged)

    /**
     * Configuration object: each config key will be a writable property of this object. property bindings work.
     */
    Q_PROPERTY(QObject *configuration READ configuration NOTIFY configurationChanged)

    Q_PROPERTY(QQmlComponent *component READ component NOTIFY pluginChanged)
    Q_PROPERTY(QQmlComponent *plasmaComponent READ plasmaComponent NOTIFY plasmaComponentChanged)

    /**
      * Information provided from the indicator itself
      */
    Q_PROPERTY(Latte::ViewPart::IndicatorPart::Info *info READ info NOTIFY infoChanged)

    /**
      * Resources provided from the indicator itself
      */
    Q_PROPERTY(Latte::ViewPart::IndicatorPart::Resources *resources READ resources NOTIFY resourcesChanged)


public:
    Indicator(Latte::View *parent);
    virtual ~Indicator();

    bool enabled() const;
    void setEnabled(bool enabled);

    bool enabledForApplets() const;
    void setEnabledForApplets(bool enabled);

    bool isCustomIndicator() const;

    bool latteTasksArePresent();

    bool pluginIsReady();

    int index(const QString &type);

    QString type() const;
    void setType(QString type);

    QString uiPath() const;

    QString customType() const;

    int customPluginsCount() const;
    QStringList customPluginIds() const;
    QStringList customPluginNames() const;
    QStringList customLocalPluginIds() const;

    IndicatorPart::Info *info() const;
    IndicatorPart::Resources *resources() const;

    QObject *configuration() const;
    QQmlComponent *component() const;
    QQmlComponent *plasmaComponent() const;

    void load(QString type);
    void unloadIndicators();

signals:
    void customPluginsChanged();
    void enabledChanged();
    void enabledForAppletsChanged();
    void configurationChanged();
    void customPluginChanged();
    void infoChanged();
    void latteTasksArePresentChanged();
    void plasmaComponentChanged();
    void pluginChanged();
    void pluginIsReadyChanged();
    void resourcesChanged();

private:
    void loadConfig();
    void saveConfig();

    void setPluginIsReady(bool ready);

    void setCustomType(QString type);

    void loadPlasmaComponent();
    void updateComponent();
    void updateScheme();

private:
    bool m_enabled{true};
    bool m_enabledForApplets{true};
    bool m_pluginIsReady{false};

    QString m_pluginPath;
    QString m_type{"org.kde.latte.default"};
    QString m_customType;

    QPointer<QQmlComponent> m_component;
    QPointer<QQmlComponent> m_plasmaComponent;
    QPointer<QQmlComponent> m_configUi;
    QPointer<KConfigLoader> m_configLoader;
    QPointer<Latte::Corona> m_corona;
    QPointer<Latte::View> m_view;

    KPluginMetaData m_metadata;

    QPointer<IndicatorPart::Info> m_info;
    QPointer<IndicatorPart::Resources> m_resources;

    QPointer<KDeclarative::ConfigPropertyMap> m_configuration;
};

}
}

#endif
