/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "dockpackage.h"

#include <QDebug>

#include <KPackage/PackageLoader>
#include <KI18n/KLocalizedString>

namespace Latte {

DockPackage::DockPackage(QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
{
}

DockPackage::~DockPackage()
{
}

void DockPackage::initPackage(KPackage::Package *package)
{
    auto fallback = KPackage::PackageLoader::self()->loadPackage("Plasma/Shell", "org.kde.plasma.desktop");
    package->setDefaultPackageRoot(QStringLiteral("plasma/shells/"));
    package->setPath("org.kde.latte.shell");
    package->addFileDefinition("defaults", QStringLiteral("defaults"), i18n("Latte Dock defaults"));
    package->addFileDefinition("lattedockui", QStringLiteral("views/Panel.qml"), i18n("Latte Dock panel"));
    //Configuration
    package->addFileDefinition("lattedockconfigurationui", QStringLiteral("configuration/LatteDockConfiguration.qml"), i18n("Dock configuration UI"));
    package->addFileDefinition("lattedocksecondaryconfigurationui", QStringLiteral("configuration/LatteDockSecondaryConfiguration.qml"), i18n("Dock secondary configuration UI"));
    package->addFileDefinition("configmodel", QStringLiteral("configuration/config.qml"), i18n("Config model"));
    package->addFileDefinition("splitter", QStringLiteral("images/splitter.svgz"), i18n("Splitter"));
    package->addFileDefinition("trademark", QStringLiteral("images/trademark.svgz"), i18n("Latte Trademark"));
    package->addFileDefinition("infoviewui", QStringLiteral("views/InfoView.qml"), i18n("Info View Window"));

    package->addFileDefinition("layout1", QStringLiteral("layouts/Default.latterc"), i18n("default layout file"));
    package->addFileDefinition("layout2", QStringLiteral("layouts/Plasma.latterc"), i18n("plasma layout file"));
    package->addFileDefinition("layout3", QStringLiteral("layouts/Unity.latterc"), i18n("unity layout file"));
    package->addFileDefinition("layout4", QStringLiteral("layouts/Extended.latterc"), i18n("extended layout file"));

    package->addFileDefinition("preset1", QStringLiteral("presets/Default.layout.latte"), i18n("default preset file"));
    package->addFileDefinition("preset2", QStringLiteral("presets/Plasma.layout.latte"), i18n("plasma preset file"));
    package->addFileDefinition("preset3", QStringLiteral("presets/Unity.layout.latte"), i18n("unity preset file"));
    package->addFileDefinition("preset4", QStringLiteral("presets/Extended.layout.latte"), i18n("extended preset file"));
    package->addFileDefinition("preset10", QStringLiteral("presets/multiple-layouts_hidden.layout.latte"), i18n("multiple layouts hidden file"));

    package->setFallbackPackage(fallback);
    qDebug() << "package is valid" << package->isValid();
}

void DockPackage::pathChanged(KPackage::Package *package)
{
    if (!package->metadata().isValid())
        return;

    const QString pluginName = package->metadata().pluginId();

    if (!pluginName.isEmpty() && pluginName != "org.kde.latte.shell") {
        auto fallback = KPackage::PackageLoader::self()->loadPackage("Plasma/Shell", "org.kde.latte.shell");
        package->setFallbackPackage(fallback);
    } else if (pluginName.isEmpty() || pluginName == "org.kde.latte.shell") {
        package->setFallbackPackage(KPackage::Package());
    }
}

}
