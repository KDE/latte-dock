/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

K_PLUGIN_CLASS_WITH_JSON(Latte::IndicatorPackage, "latte-packagestructure-indicator.json")

#include "indicatorpackage.moc"
