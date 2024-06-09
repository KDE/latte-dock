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
#include <KLocalizedString>

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
    package->addFileDefinition("defaults", QStringLiteral("defaults"));
    package->addFileDefinition("lattedockui", QStringLiteral("views/Panel.qml"));
    package->addFileDefinition("widgetexplorerui", QStringLiteral("views/WidgetExplorer.qml"));
    //Configuration
    package->addFileDefinition("lattedockconfigurationui", QStringLiteral("configuration/LatteDockConfiguration.qml"));
    package->addFileDefinition("lattedocksecondaryconfigurationui", QStringLiteral("configuration/LatteDockSecondaryConfiguration.qml"));
    package->addFileDefinition("canvasconfigurationui", QStringLiteral("configuration/CanvasConfiguration.qml"));
    package->addFileDefinition("configmodel", QStringLiteral("configuration/config.qml"));
    package->addFileDefinition("splitter", QStringLiteral("images/splitter.svgz"));
    package->addFileDefinition("trademark", QStringLiteral("images/trademark.svgz"));
    package->addFileDefinition("trademarkicon", QStringLiteral("images/trademarkicon.svgz"));
    package->addFileDefinition("infoviewui", QStringLiteral("views/InfoView.qml"));

    package->addFileDefinition("layout1", QStringLiteral("layouts/Default.latterc"));
    package->addFileDefinition("layout2", QStringLiteral("layouts/Plasma.latterc"));
    package->addFileDefinition("layout3", QStringLiteral("layouts/Unity.latterc"));
    package->addFileDefinition("layout4", QStringLiteral("layouts/Extended.latterc"));

    package->addFileDefinition("templates", QStringLiteral("templates"));

    package->addFileDefinition("preset1", QStringLiteral("presets/Default.layout.latte"));
    package->addFileDefinition("preset2", QStringLiteral("presets/Plasma.layout.latte"));
    package->addFileDefinition("preset3", QStringLiteral("presets/Unity.layout.latte"));
    package->addFileDefinition("preset4", QStringLiteral("presets/Extended.layout.latte"));
    package->addFileDefinition("preset10", QStringLiteral("presets/multiple-layouts_hidden.layout.latte"));

    //! applets
    package->addFileDefinition("compactapplet", QStringLiteral("applet/CompactApplet.qml"));

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
