#include "nowdockpackage.h"

#include <KPackage/PackageLoader>
#include <KI18n/KLocalizedString>
#include <QDebug>

NowDockPackage::NowDockPackage(QObject *parent, const QVariantList &args)
    : KPackage::PackageStructure(parent, args)
{
}

NowDockPackage::~NowDockPackage()
{
}

void NowDockPackage::initPackage(KPackage::Package *package)
{
    auto fallback = KPackage::PackageLoader::self()->loadPackage("Plasma/Shell", "org.kde.plasma.desktop");
    
    package->setDefaultPackageRoot(QStringLiteral("plasma/shells/"));
    package->setPath("org.kde.latte.shell");
    package->addFileDefinition("nowdockui", QStringLiteral("views/Panel.qml"), i18n("Now Dock panel"));
    //Configuration
    package->addFileDefinition("nowdockconfigurationui", QStringLiteral("configuration/LatteDockConfiguration.qml"), i18n("Dock configuration UI"));
    package->addFileDefinition("configmodel", QStringLiteral("configuration/config.qml"), i18n("Config model"));
    package->setFallbackPackage(fallback);
    qDebug() << "package is valid" << package->isValid();
}

void NowDockPackage::pathChanged(KPackage::Package *package)
{
    if (!package->metadata().isValid())
        return;
        
    const QString pluginName = package->metadata().pluginId();
    
    if (!pluginName.isEmpty() && pluginName != "org.kde.latte.shell") {
        auto fallback = KPackage::PackageLoader::self()->loadPackage("LatteDock/Shell", "org.kde.latte.shell");
        package->setFallbackPackage(fallback);
    } else if (pluginName.isEmpty() || pluginName == "org.kde.latte.shell") {
        package->setFallbackPackage(KPackage::Package());
    }
}
