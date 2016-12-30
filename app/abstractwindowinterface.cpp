#include "abstractwindowinterface.h"
#include "xwindowinterface.h"

#include <QObject>
#include <QQuickWindow>

#include <KWindowSystem>

namespace Latte {

AbstractWindowInterface::AbstractWindowInterface(QQuickWindow *const view, QObject *parent)
    : QObject(parent), m_view(view)
{

}

AbstractWindowInterface::~AbstractWindowInterface()
{
    
}

AbstractWindowInterface *AbstractWindowInterface::getInstance(QQuickWindow * const view, QObject *parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: WaylandWindowInterface
        return nullptr;
    }
    
    /* if(KWindowSystem::isPlatformX11) */
    return new XWindowInterface(view, parent);
}

}
