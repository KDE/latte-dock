/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lattepackage.h"

// Qt
#include <QDebug>
#include <QLatin1String>

// KDE
#include <KPackage/PackageLoader>
#include <KI18n/KLocalizedString>

namespace Latte {

Package::Package(QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
{
}

Package::~Package()
{
}

void Package::initPackage(KPackage::Package *package)
{
    auto fallback = KPackage::PackageLoader::self()->loadPackage("Plasma/Shell", "org.kde.plasma.desktop");
    package->setDefaultPackageRoot(QStringLiteral("plasma/shells/"));
    package->setPath("org.kde.latte.shell");
    package->addFileDefinition("defaults", QStringLiteral("defaults"), i18n("Latte Dock defaults"));
    package->addFileDefinition("lattedockui", QStringLiteral("views/Panel.qml"), i18n("Latte Dock panel"));
    package->addFileDefinition("widgetexplorerui", QStringLiteral("views/WidgetExplorer.qml"), i18n("Widget Explorer"));
    //Configuration
    package->addFileDefinition("lattedockconfigurationui", QStringLiteral("configuration/LatteDockConfiguration.qml"), i18n("Dock configuration UI"));
    package->addFileDefinition("lattedocksecondaryconfigurationui", QStringLiteral("configuration/LatteDockSecondaryConfiguration.qml"), i18n("Dock secondary configuration UI"));
    package->addFileDefinition("canvasconfigurationui", QStringLiteral("configuration/CanvasConfiguration.qml"), i18n("Dock canvas configuration UI"));
    package->addFileDefinition("configmodel", QStringLiteral("configuration/config.qml"), i18n("Config model"));
    package->addFileDefinition("splitter", QStringLiteral("images/splitter.svgz"), i18n("Splitter"));
    package->addFileDefinition("trademark", QStringLiteral("images/trademark.svgz"), i18n("Latte Trademark"));
    package->addFileDefinition("trademarkicon", QStringLiteral("images/trademarkicon.svgz"), i18n("Latte Trademark Icon"));
    package->addFileDefinition("infoviewui", QStringLiteral("views/InfoView.qml"), i18n("Info View Window"));

    package->addFileDefinition("layout1", QStringLiteral("layouts/Default.latterc"), i18n("default layout file"));
    package->addFileDefinition("layout2", QStringLiteral("layouts/Plasma.latterc"), i18n("plasma layout file"));
    package->addFileDefinition("layout3", QStringLiteral("layouts/Unity.latterc"), i18n("unity layout file"));
    package->addFileDefinition("layout4", QStringLiteral("layouts/Extended.latterc"), i18n("extended layout file"));

    package->addFileDefinition("templates", QStringLiteral("templates"), i18n("system templates directory"));

    package->addFileDefinition("preset1", QStringLiteral("presets/Default.layout.latte"), i18n("default preset file"));
    package->addFileDefinition("preset2", QStringLiteral("presets/Plasma.layout.latte"), i18n("plasma preset file"));
    package->addFileDefinition("preset3", QStringLiteral("presets/Unity.layout.latte"), i18n("unity preset file"));
    package->addFileDefinition("preset4", QStringLiteral("presets/Extended.layout.latte"), i18n("extended preset file"));
    package->addFileDefinition("preset10", QStringLiteral("presets/multiple-layouts_hidden.layout.latte"), i18n("multiple layouts hidden file"));

    //! applets
    package->addFileDefinition("compactapplet", QStringLiteral("applet/CompactApplet.qml"), i18n("QML component that shows an applet in a popup"));

    package->setFallbackPackage(fallback);
    qDebug() << "package is valid" << package->isValid();
}

void Package::pathChanged(KPackage::Package *package)
{
    if (!package->metadata().isValid())
        return;

    const QString pluginName = package->metadata().pluginId();

    if (!pluginName.isEmpty() && pluginName != "org.kde.latte.shell") {
        auto fallback = KPackage::PackageLoader::self()->loadPackage("Plasma/Shell", "org.kde.latte.shell");
        package->setFallbackPackage(fallback);
    } else if (pluginName.isEmpty() || pluginName == QLatin1String("org.kde.latte.shell")) {
        package->setFallbackPackage(KPackage::Package());
    }
}

}
