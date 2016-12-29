#include "abstractwindowinterface.h"

#include <QObject>
#include <QQuickWindow>

namespace Latte {

AbstractWindowInterface::AbstractWindowInterface(QQuickWindow *const view, QObject *parent)
    : QObject(parent), m_view(view)
{

}

AbstractWindowInterface::~AbstractWindowInterface()
{

}



}
