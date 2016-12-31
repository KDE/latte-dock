#include "windowsystem.h"

#include <KWindowSystem>

namespace Latte {

WindowSystem::WindowSystem(QObject *parent) :
    QObject(parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        
    } else {
        compositingChangedProxy(KWindowSystem::self()->compositingActive());
        connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), this, SLOT(compositingChanged(bool)));
    }
}

WindowSystem::~WindowSystem()
{
}

WindowSystem &WindowSystem::self()
{
    static WindowSystem ws;
    
    return ws;
}

bool WindowSystem::compositingActive() const
{
    return m_compositing;
}

void WindowSystem::compositingChangedProxy(bool enable)
{
    if (m_compositing == enable)
        return;
    
    m_compositing = enable;
    emit compositingChanged();
}

} //end of namespace
