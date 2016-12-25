#include "nowdockplugin.h"
#include "panelwindow.h"
#include "windowsystem.h"
#include "types.h"

#include <qqml.h>

void NowDockPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.nowdock"));
    
    qmlRegisterUncreatableType<NowDock::Types>(uri, 0, 1, "Types", "NowDock Types uncreatable");
    
    qmlRegisterType<NowDock::PanelWindow>(uri, 0, 1, "PanelWindow");
    qmlRegisterType<NowDock::WindowSystem>(uri, 0, 1, "WindowSystem");
}

