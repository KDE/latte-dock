/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indicatorpackage.h"

// Qt
#include <QDebug>

// KDE
#include <KPackage/PackageLoader>

namespace Latte {

IndicatorPackage::IndicatorPackage(QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
{
}

void IndicatorPackage::initPackage(KPackage::Package *package)
{
    package->setDefaultPackageRoot(QStringLiteral("latte/indicators"));

    package->addDirectoryDefinition("config", QStringLiteral("config"));
    package->addDirectoryDefinition("ui", QStringLiteral("ui"));
    package->addDirectoryDefinition("data", QStringLiteral("data"));
    package->addDirectoryDefinition("scripts", QStringLiteral("code"));
    package->addDirectoryDefinition("translations", QStringLiteral("locale"));
}

}

K_PLUGIN_CLASS_WITH_JSON(Latte::IndicatorPackage, "latte-packagestructure-indicator.json")

#include "indicatorpackage.moc"
