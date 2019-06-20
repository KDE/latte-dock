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
#include "indicatorinfo.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../indicator/factory.h"
#include "../../../liblatte2/types.h"

// Qt
#include <QFileDialog>

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
    connect(this, &Indicator::enabledForAppletsChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::paddingChanged, this, &Indicator::saveConfig);
    connect(this, &Indicator::pluginChanged, this, &Indicator::saveConfig);

    connect(m_view, &Latte::View::latteTasksArePresentChanged, this, &Indicator::latteTasksArePresentChanged);

    connect(m_corona->indicatorFactory(), &Latte::Indicator::Factory::customPluginsChanged, [this]() {
        if (!m_corona->indicatorFactory()->pluginExists(m_type)) {
            setType("org.kde.latte.default");
        }

        emit customPluginsChanged();
    });

    connect(m_corona->indicatorFactory(), &Latte::Indicator::Factory::pluginsUpdated, [this]() {
        if (m_view && m_view->layout()) {
            m_view->layout()->recreateView(m_view->containment());
        }

    });

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

        QString path = m_metadata.fileName();
        m_pluginPath = path.remove("metadata.desktop");

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
            m_lastCreatedConfigUi->setTranslationDomain(QLatin1String("latte_indicator_") + m_metadata.pluginId());
            m_lastCreatedConfigUi->setInitializationDelayed(true);
            uiPath = m_pluginPath + "package/" + uiPath;
            m_lastCreatedConfigUi->setSource(QUrl::fromLocalFile(uiPath));
            m_lastCreatedConfigUi->rootContext()->setContextProperty(QStringLiteral("dialog"), parent);
            m_lastCreatedConfigUi->rootContext()->setContextProperty(QStringLiteral("indicator"), this);
            m_lastCreatedConfigUi->completeInitialization();

            QQuickItem *qmlItem = qobject_cast<QQuickItem*>(m_lastCreatedConfigUi->rootObject());
            qmlItem->setParentItem(parent);

            setProvidesConfigUi(true);
        } else {
            setProvidesConfigUi(false);
        }
    }
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

void Indicator::addIndicator()
{
    QFileDialog *fileDialog = new QFileDialog(nullptr
                                              , i18nc("add indicator", "Add Indicator")
                                              , QDir::homePath()
                                              , QStringLiteral("indicator.latte"));

    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setDefaultSuffix("indicator.latte");

    QStringList filters;
    filters << QString(i18nc("add indicator file", "Latte Indicator") + "(*.indicator.latte)");
    fileDialog->setNameFilters(filters);

    connect(fileDialog, &QFileDialog::finished, fileDialog, &QFileDialog::deleteLater);

    connect(fileDialog, &QFileDialog::fileSelected, this, [&](const QString & file) {
        qDebug() << "Trying to import indicator file ::: " << file;
        m_corona->indicatorFactory()->importIndicatorFile(file);
    });

    fileDialog->open();
}

void Indicator::downloadIndicator()
{
    //! call asynchronously in order to not crash when view settings window
    //! loses focus and it closes
    QTimer::singleShot(0, [this]() {
        m_corona->indicatorFactory()->downloadIndicator();
    });
}

void Indicator::removeIndicator(QString pluginId)
{    //! call asynchronously in order to not crash when view settings window
    //! loses focus and it closes
    QTimer::singleShot(0, [this, pluginId]() {
        m_corona->indicatorFactory()->removeIndicator(pluginId);
    });
}

void Indicator::loadConfig()
{
    auto config = m_view->containment()->config().group("Indicator");
    m_customType = config.readEntry("customType", QString());
    m_enabled = config.readEntry("enabled", true);
    m_enabledForApplets = config.readEntry("enabledForApplets", true);
    m_padding = config.readEntry("padding", (float)0.08);
    m_type = config.readEntry("type", "org.kde.latte.default");
}

void Indicator::saveConfig()
{
    auto config = m_view->containment()->config().group("Indicator");
    config.writeEntry("customType", m_customType);
    config.writeEntry("enabled", m_enabled);
    config.writeEntry("enabledForApplets", m_enabledForApplets);
    config.writeEntry("padding", m_padding);
    config.writeEntry("type", m_type);

    config.sync();
}

}
}
