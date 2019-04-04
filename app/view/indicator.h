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

#ifndef VIEWINDICATOR_H
#define VIEWINDICATOR_H

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
    Q_PROPERTY(bool providesConfigUi READ providesConfigUi NOTIFY providesConfigUiChanged)
    Q_PROPERTY(bool reversed READ reversed WRITE setReversed NOTIFY reversedChanged)

    Q_PROPERTY(float padding READ padding WRITE setPadding NOTIFY paddingChanged)

    Q_PROPERTY(QString type READ type WRITE setType NOTIFY pluginChanged)
    Q_PROPERTY(QString customType READ customType NOTIFY customPluginChanged)

    /* Custom plugins */
    Q_PROPERTY(int customPluginsCount READ customPluginsCount NOTIFY customPluginsChanged)
    Q_PROPERTY(QStringList customPluginIds READ customPluginIds NOTIFY customPluginsChanged)
    Q_PROPERTY(QStringList customPluginNames READ customPluginNames NOTIFY customPluginsChanged)
    Q_PROPERTY(QStringList customLocalPluginIds READ customLocalPluginIds NOTIFY customPluginsChanged)

    Q_PROPERTY(QQuickItem *info READ info NOTIFY infoChanged)

    /**
     * Configuration object: each config key will be a writable property of this object. property bindings work.
     */
    Q_PROPERTY(QObject *configuration READ configuration NOTIFY pluginChanged)

    Q_PROPERTY(QQmlComponent *component READ component NOTIFY pluginChanged)
    Q_PROPERTY(QQmlComponent *plasmaComponent READ plasmaComponent NOTIFY plasmaComponentChanged)

public:
    Indicator(Latte::View *parent);
    virtual ~Indicator();

    bool enabled() const;
    void setEnabled(bool enabled);

    bool enabledForApplets() const;
    void setEnabledForApplets(bool enabled);

    bool latteTasksArePresent();
    bool providesConfigUi() const;

    bool pluginIsReady();

    bool reversed() const;
    void setReversed(bool reversed);

    float padding() const;
    void setPadding(float padding);

    QString type() const;
    void setType(QString type);

    QString customType() const;

    int customPluginsCount() const;
    QStringList customPluginIds() const;
    QStringList customPluginNames() const;
    QStringList customLocalPluginIds() const;

    QQuickItem *info() const;

    QObject *configuration() const;
    QQmlComponent *component() const;
    QQmlComponent *plasmaComponent() const;

    void load(QString type);

public slots:
    Q_INVOKABLE void configUiFor(QString type, QQuickItem *parent);
    Q_INVOKABLE void addIndicator();
    Q_INVOKABLE void downloadIndicator();
    Q_INVOKABLE void removeIndicator(QString pluginId);
    Q_INVOKABLE void setIndicatorInfo(QQuickItem *info);

signals:
    void customPluginsChanged();
    void enabledChanged();
    void enabledForAppletsChanged();
    void customPluginChanged();
    void infoChanged();
    void latteTasksArePresentChanged();
    void paddingChanged();
    void plasmaComponentChanged();
    void pluginChanged();
    void pluginIsReadyChanged();
    void providesConfigUiChanged();
    void reversedChanged();

private:
    void loadConfig();
    void saveConfig();

    void setPluginIsReady(bool ready);
    void setProvidesConfigUi(bool provides);

    void setCustomType(QString type);

    void loadPlasmaComponent();
    void updateComponent();
    void updateScheme();

private:
    bool m_enabled{true};
    bool m_enabledForApplets{true};
    bool m_pluginIsReady{false};
    bool m_providesConfigUi{true};
    bool m_reversed{false};

    float m_padding{0.08};

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

    QPointer<QQuickItem> m_info;

    QPointer<KDeclarative::ConfigPropertyMap> m_configuration;
    QPointer<KDeclarative::QmlObjectSharedEngine> m_lastCreatedConfigUi;
};

}
}

#endif
