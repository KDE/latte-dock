/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indicator.h"

// local
#include <coretypes.h>
#include "indicatorinfo.h"
#include "../containmentinterface.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../indicator/factory.h"

// Qt
#include <QFileDialog>
#include <QFileInfo>
#include <QLatin1String>

// KDE
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KDeclarative/ConfigPropertyMap>
#include <KDeclarative/QmlObjectSharedEngine>


namespace Latte {
namespace ViewPart {

Indicator::Indicator(Latte::View *parent)
    : QObject(parent),
      m_view(parent),
      m_info(new IndicatorPart::Info(this)),
      m_resources(new IndicatorPart::Resources(this))
{
    m_corona = qobject_cast<Latte::Corona *>(m_view->corona());
    loadConfig();

    connect(this, &Indicator::enabledChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::pluginChanged, this, &Indicator::saveConfig);

    connect(m_view->extendedInterface(), &ContainmentInterface::hasLatteTasksChanged, this, &Indicator::latteTasksArePresentChanged);

    connect(m_view, &Latte::View::indicatorPluginChanged, [this](const QString &indicatorId) {
        if (m_corona && m_corona->indicatorFactory()->isCustomType(indicatorId)) {
            emit customPluginsChanged();
        }
    });

    connect(m_view, &Latte::View::indicatorPluginRemoved, [this](const QString &indicatorId) {
        if (m_corona && m_type == indicatorId && !m_corona->indicatorFactory()->pluginExists(indicatorId)) {
            setType("org.kde.latte.default");
        }

        if (m_corona && m_corona->indicatorFactory()->isCustomType(indicatorId)) {
            emit customPluginsChanged();
        }
    });

    load(m_type);

    loadPlasmaComponent();
}

Indicator::~Indicator()
{
    unloadIndicators();

    if (m_component) {
        m_component->deleteLater();
    }

    if (m_configLoader) {
        m_configLoader->deleteLater();
    }

    if (m_configuration) {
        m_configuration->deleteLater();
    }

    if (m_info) {
        m_info->deleteLater();
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

bool Indicator::isCustomIndicator() const
{
    return m_corona->indicatorFactory()->isCustomType(type());
}

bool Indicator::latteTasksArePresent()
{
    return m_view->extendedInterface()->hasLatteTasks();
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

int Indicator::index(const QString &type)
{
    if (type == QLatin1String("org.kde.latte.default")) {
        return 0;
    } else if (type == QLatin1String("org.kde.latte.plasma")) {
        return 1;
    } else if (type == QLatin1String("org.kde.latte.plasmatabstyle")) {
        return 2;
    } else if (customPluginIds().contains(type)){
        return 3 + customPluginIds().indexOf(type);
    }

    return -1;
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

QString Indicator::uiPath() const
{
    return m_corona->indicatorFactory()->uiPath(m_type);
}

QStringList Indicator::customPluginIds() const
{
    return m_corona->indicatorFactory()->customPluginIds();
}

QStringList Indicator::customPluginNames() const
{
    return m_corona->indicatorFactory()->customPluginNames();
}

QStringList Indicator::customLocalPluginIds() const
{
    return m_corona->indicatorFactory()->customLocalPluginIds();
}

IndicatorPart::Info *Indicator::info() const
{
    return m_info;
}

IndicatorPart::Resources *Indicator::resources() const
{
    return m_resources;
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
        m_pluginPath = QFileInfo(m_metadata.fileName()).absolutePath();

        if (m_corona && m_corona->indicatorFactory()->isCustomType(type)) {
            setCustomType(type);
        }

        updateScheme();
        updateComponent();

        emit pluginChanged();

        //! create all indicators with the new type
        setPluginIsReady(true);
    } else if (type!="org.kde.latte.default") {
        qDebug() << " Indicator metadata are not valid : " << type;
        setType("org.kde.latte.default");
    }
}

void Indicator::updateComponent()
{
    auto prevComponent = m_component;

    QString uiPath = m_metadata.value("X-Latte-MainScript");

    if (!uiPath.isEmpty()) {
        uiPath = m_pluginPath + "/package/" + uiPath;
        m_component = new QQmlComponent(m_view->engine(), uiPath);
    }

    if (prevComponent) {
        prevComponent->deleteLater();
    }
}

void Indicator::loadPlasmaComponent()
{
    auto prevComponent = m_plasmaComponent;

    KPluginMetaData metadata = m_corona->indicatorFactory()->metadata("org.kde.latte.plasmatabstyle");
    QString uiPath = metadata.value("X-Latte-MainScript");

    if (!uiPath.isEmpty()) {
        uiPath = QFileInfo(metadata.fileName()).absolutePath() + "/package/" + uiPath;
        m_plasmaComponent = new QQmlComponent(m_view->engine(), uiPath);
    }

    if (prevComponent) {
        prevComponent->deleteLater();
    }

    emit plasmaComponentChanged();
}

void Indicator::unloadIndicators()
{
    setPluginIsReady(false);
}

void Indicator::updateScheme()
{
    auto prevConfigLoader = m_configLoader;
    auto prevConfiguration = m_configuration;

    QString xmlPath = m_metadata.value("X-Latte-ConfigXml");

    if (!xmlPath.isEmpty()) {
        QFile file(m_pluginPath + "/package/" + xmlPath);
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

    emit configurationChanged();
}

void Indicator::loadConfig()
{
    auto config = m_view->containment()->config().group("Indicator");
    m_customType = config.readEntry("customType", QString());
    m_enabled = config.readEntry("enabled", true);
    m_type = config.readEntry("type", "org.kde.latte.default");
}

void Indicator::saveConfig()
{
    auto config = m_view->containment()->config().group("Indicator");
    config.writeEntry("customType", m_customType);
    config.writeEntry("enabled", m_enabled);
    config.writeEntry("type", m_type);
}

}
}
