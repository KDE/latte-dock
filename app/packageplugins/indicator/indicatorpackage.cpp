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

#include "indicatorpackage.h"

// Qt
#include <QDebug>

// KDE
#include <KPackage/PackageLoader>
#include <KI18n/KLocalizedString>

namespace Latte {

IndicatorPackage::IndicatorPackage(QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
{
}

IndicatorPackage::~IndicatorPackage()
{
}

void IndicatorPackage::initPackage(KPackage::Package *package)
{
    package->setDefaultPackageRoot(QStringLiteral("latte/indicators"));

    package->addDirectoryDefinition("config", QStringLiteral("config"), i18n("Configuration Definitions"));
    package->addDirectoryDefinition("ui", QStringLiteral("ui"), i18n("User Interface"));
    package->addDirectoryDefinition("data", QStringLiteral("data"), i18n("Data Files"));
    package->addDirectoryDefinition("scripts", QStringLiteral("code"), i18n("Executable Scripts"));
    package->addDirectoryDefinition("translations", QStringLiteral("locale"), i18n("Translations"));
}

}

K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(Latte::IndicatorPackage, "latte-packagestructure-indicator.json")

#include "indicatorpackage.moc"
