#include "windowsystem.h"

#include <KWindowSystem>

namespace Latte {

WindowSystem::WindowSystem(QObject *parent) :
    QObject(parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: Wayland compositing
    } else {
        compositingChangedProxy(KWindowSystem::self()->compositingActive());
        connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool))
                , this, SLOT(compositingChangedProxy(bool)));
    }
}

WindowSystem &WindowSystem::self()
{
    static WindowSystem wm;
    return wm;
}

WindowSystem::~WindowSystem()
{
}

bool WindowSystem::compositingActive() const
{
    return m_enabled;
}

void WindowSystem::compositingChangedProxy(bool enabled)
{
    m_enabled = enabled;
    emit compositingChanged();
}

} //end of namespace
