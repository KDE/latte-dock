#include "windowsystem.h"

#include <KWindowSystem>

namespace NowDock {

WindowSystem::WindowSystem(QObject *parent) :
    QObject(parent)
{
    connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), this, SLOT(compositingChanged(bool)));
}

WindowSystem::~WindowSystem()
{
}

bool WindowSystem::compositingActive() const
{
    return KWindowSystem::compositingActive();
}

void WindowSystem::compositingChanged(bool state)
{
    emit compositingChanged();
}

}
