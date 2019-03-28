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

#include "indicator.h"

// local
#include "view.h"
#include "../lattecorona.h"
#include "../indicator/factory.h"

// KDE
#include <KPluginMetaData>
#include <KDeclarative/ConfigPropertyMap>
#include <KDeclarative/QmlObjectSharedEngine>


namespace Latte {
namespace ViewPart {

Indicator::Indicator(Latte::View *parent)
    : QObject(parent),
      m_view(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(m_view->corona());
    loadConfig();

    connect(this, &Indicator::enabledChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::enabledForAppletsChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::paddingChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::reversedChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::pluginChanged, this, &Indicator::saveConfig);

    connect(m_view, &Latte::View::latteTasksArePresentChanged, this, &Indicator::latteTasksArePresentChanged);

    connect(m_corona->indicatorFactory(), &Latte::Indicator::Factory::customPluginsChanged, this, &Indicator::customPluginsChanged);

    connect(this, &Indicator::pluginChanged, [this]() {
        if ((m_type != "org.kde.latte.default") && m_type != "org.kde.latte.plasma") {
            setCustomType(m_type);
        }
    });

    load(m_type);

    loadPlasmaComponent();
}

Indicator::~Indicator()
{
    if (m_component) {
        m_component->deleteLater();
    }

    if (m_configLoader) {
        m_configLoader->deleteLater();
    }

    if (m_configuration) {
        m_configuration->deleteLater();
    }
}

bool Indicator::enabled() const
{
    return m_enabled;
}

void Indicator::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit enabledChanged();
}

bool Indicator::enabledForApplets() const
{
    return m_enabledForApplets;
}

void Indicator::setEnabledForApplets(bool enabled)
{
    if (m_enabledForApplets == enabled) {
        return;
    }

    m_enabledForApplets = enabled;
    emit enabledForAppletsChanged();
}

bool Indicator::latteTasksArePresent()
{
    return m_view->latteTasksArePresent();
}

bool Indicator::providesConfigUi() const
{
    return m_providesConfigUi;
}

void Indicator::setProvidesConfigUi(bool provides)
{
    if (m_providesConfigUi == provides) {
        return;
    }

    m_providesConfigUi = provides;
    emit providesConfigUiChanged();
}

bool Indicator::reversed() const
{
    return m_reversed;
}

void Indicator::setReversed(bool reversed)
{
    if (m_reversed == reversed) {
        return;
    }

    m_reversed = reversed;
    emit reversedChanged();
}

float Indicator::padding() const
{
    return m_padding;
}

void Indicator::setPadding(float padding)
{
    if (m_padding == padding) {
        return;
    }

    m_padding = padding;
    emit paddingChanged();
}

bool Indicator::pluginIsReady()
{
    return m_pluginIsReady;
}

void Indicator::setPluginIsReady(bool ready)
{
    if (m_pluginIsReady == ready) {
        return;
    }

    m_pluginIsReady = ready;
    emit pluginIsReadyChanged();
}

QString Indicator::type() const
{
    return m_type;
}

void Indicator::setType(QString type)
{
    if (m_type == type) {
        return;
    }

    load(type);
}

QString Indicator::customType() const
{
    return m_customType;
}

void Indicator::setCustomType(QString type)
{
    if (m_customType == type) {
        return;
    }

    m_customType = type;
    emit customPluginChanged();
}

int Indicator::customPluginsCount() const
{
    return m_corona->indicatorFactory()->customPluginsCount();
}

QStringList Indicator::customPluginIds() const
{
    return m_corona->indicatorFactory()->customPluginIds();
}

QStringList Indicator::customPluginNames() const
{
    return m_corona->indicatorFactory()->customPluginNames();
}

QQmlComponent *Indicator::component() const
{
    return m_component;
}

QQmlComponent *Indicator::plasmaComponent() const
{
    return m_plasmaComponent;
}

QObject *Indicator::configuration() const
{
    return m_configuration;
}

void Indicator::load(QString type)
{
    KPluginMetaData metadata = m_corona->indicatorFactory()->metadata(type);

    if (metadata.isValid()) {
        bool state{m_enabled};
        //! remove all previous indicators
        setPluginIsReady(false);

        m_metadata = metadata;
        m_type = type;

        QString path = m_metadata.fileName();
        m_pluginPath = path.remove("metadata.desktop");

        updateScheme();
        updateComponent();

        emit pluginChanged();

        //! create all indicators with the new type
        setPluginIsReady(true);
    } else if (type!="org.kde.latte.default") {
        setType("org.kde.latte.default");
    }
}

void Indicator::updateComponent()
{
    auto prevComponent = m_component;

    QString uiPath = m_metadata.value("X-Latte-MainScript");

    if (!uiPath.isEmpty()) {
        uiPath = m_pluginPath + "package/" + uiPath;
        m_component = new QQmlComponent(m_view->engine(), uiPath);
    }

    if (prevComponent) {
        prevComponent->deleteLater();
    }
}

void Indicator::loadPlasmaComponent()
{
    auto prevComponent = m_plasmaComponent;

    KPluginMetaData metadata = m_corona->indicatorFactory()->metadata("org.kde.latte.plasma");
    QString uiPath = metadata.value("X-Latte-MainScript");

    if (!uiPath.isEmpty()) {
        QString path = metadata.fileName();
        path = path.remove("metadata.desktop");

        uiPath = path + "package/" + uiPath;
        m_plasmaComponent = new QQmlComponent(m_view->engine(), uiPath);
    }

    if (prevComponent) {
        prevComponent->deleteLater();
    }

    emit plasmaComponentChanged();
}

void Indicator::configUiFor(QString type, QQuickItem *parent)
{
    if (m_lastCreatedConfigUi) {
        delete m_lastCreatedConfigUi;
        m_lastCreatedConfigUi = nullptr;
    }
    auto prevConfigUi = m_lastCreatedConfigUi;

    KPluginMetaData metadata;

    if (m_metadata.pluginId() == type) {
        metadata = m_metadata;
    } else {
        metadata = m_corona->indicatorFactory()->metadata(type);
    }

    if (metadata.isValid()) {
        QString uiPath = metadata.value("X-Latte-ConfigUi");

        if (!uiPath.isEmpty()) {
            m_lastCreatedConfigUi = new KDeclarative::QmlObjectSharedEngine(parent);
            m_lastCreatedConfigUi->setInitializationDelayed(true);
            uiPath = m_pluginPath + "package/" + uiPath;
            m_lastCreatedConfigUi->setSource(QUrl::fromLocalFile(uiPath));
            m_lastCreatedConfigUi->rootContext()->setContextProperty(QStringLiteral("indicator"), this);
            m_lastCreatedConfigUi->completeInitialization();

            m_lastCreatedConfigUi->setTranslationDomain(QLatin1String("latte_indicator_") + m_metadata.pluginId());

            QQuickItem *qmlItem = qobject_cast<QQuickItem*>(m_lastCreatedConfigUi->rootObject());
            qmlItem->setParentItem(parent);

            setProvidesConfigUi(true);
        } else {
            setProvidesConfigUi(false);
        }
    }
}

void Indicator::updateScheme()
{
    auto prevConfigLoader = m_configLoader;
    auto prevConfiguration = m_configuration;

    QString xmlPath = m_metadata.value("X-Latte-ConfigXml");

    if (!xmlPath.isEmpty()) {
        QFile file(m_pluginPath + "package/" + xmlPath);
        m_configLoader = new KConfigLoader(m_view->containment()->config().group("Indicator").group(m_metadata.pluginId()), &file);
        m_configuration = new KDeclarative::ConfigPropertyMap(m_configLoader, this);
    } else {
        m_configLoader = nullptr;
        m_configuration = nullptr;
    }

    if (prevConfigLoader) {
        prevConfigLoader->deleteLater();
    }

    if (prevConfiguration) {
        prevConfiguration->deleteLater();
    }
}

void Indicator::loadConfig()
{
    auto config = m_view->containment()->config().group("Indicator");
    m_customType = config.readEntry("customType", QString());
    m_enabled = config.readEntry("enabled", true);
    m_enabledForApplets = config.readEntry("enabledForApplets", true);
    m_padding = config.readEntry("padding", (float)0.08);
    m_reversed = config.readEntry("reversed", false);
    m_type = config.readEntry("type", "org.kde.latte.default");
}

void Indicator::saveConfig()
{
    auto config = m_view->containment()->config().group("Indicator");
    config.writeEntry("customType", m_customType);
    config.writeEntry("enabled", m_enabled);
    config.writeEntry("enabledForApplets", m_enabledForApplets);
    config.writeEntry("padding", m_padding);
    config.writeEntry("reversed", m_reversed);
    config.writeEntry("type", m_type);

    config.sync();
}

}
}
