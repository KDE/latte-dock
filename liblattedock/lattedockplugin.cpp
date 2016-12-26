#include "lattedockplugin.h"
#include "windowsystem.h"
#include "dock.h"

#include <qqml.h>

void LatteDockPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.latte"));
    
    qmlRegisterUncreatableType<Latte::Dock>(uri, 0, 1, "Dock", "Latte Dock Types uncreatable");
    
    qmlRegisterType<Latte::WindowSystem>(uri, 0, 1, "WindowSystem");
}
