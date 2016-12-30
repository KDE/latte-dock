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
    package->addFileDefinition("lattedockui", QStringLiteral("views/Panel.qml"), i18n("Latte Dock panel"));
    //Configuration
    package->addFileDefinition("lattedockconfigurationui", QStringLiteral("configuration/LatteDockConfiguration.qml"), i18n("Dock configuration UI"));
    package->addFileDefinition("configmodel", QStringLiteral("configuration/config.qml"), i18n("Config model"));
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
