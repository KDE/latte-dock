#include "lattedockplugin.h"
#include "windowsystem.h"
#include "types.h"

#include <qqml.h>

void LatteDockPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.latte.dock"));
    
    qmlRegisterUncreatableType<LatteDock::Types>(uri, 0, 1, "Types", "LatteDock Types uncreatable");
    
    qmlRegisterType<LatteDock::WindowSystem>(uri, 0, 1, "WindowSystem");
}
