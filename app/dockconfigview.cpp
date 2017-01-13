/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dockconfigview.h"
#include "dockview.h"
#include "dockcorona.h"

#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

#include <KLocalizedContext>
#include <KWindowSystem>
#include <KWindowEffects>

#include <Plasma/Package>

namespace Latte {

DockConfigView::DockConfigView(Plasma::Containment *containment, DockView *dockView, QWindow *parent)
    : PlasmaQuick::ConfigView(containment, parent),
      m_blockFocusLost(false),
      m_containment(containment),
      m_dockView(dockView)
{
    connections << connect(dockView, &QObject::destroyed, this, &QObject::deleteLater);
    
    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);
    
    connections << connect(dockView, SIGNAL(screenChanged(QScreen *)), &m_screenSyncTimer, SLOT(start()));
    
    connections << connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        setScreen(m_dockView->screen());
        KWindowSystem::setType(winId(), NET::Dock);
        setFlags(wFlags());
        syncGeometry();
        syncSlideEffect();
    });
    
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setState(winId(), NET::KeepAbove);
    setFlags(wFlags());
    KWindowSystem::forceActiveWindow(winId());
    
    connections << connect(dockView->visibility(), &VisibilityManager::modeChanged, this, &DockConfigView::syncGeometry);
    connections << connect(containment, &Plasma::Containment::immutabilityChanged, this, &DockConfigView::immutabilityChanged);
    connections << connect(containment, &Plasma::Containment::locationChanged, [&]() {
        syncSlideEffect();
        QTimer::singleShot(200, this, &DockConfigView::syncGeometry);
    });
}

DockConfigView::~DockConfigView()
{
    foreach (auto var, connections) {
        QObject::disconnect(var);
    }
}

void DockConfigView::init()
{
    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    rootContext()->setContextProperty(QStringLiteral("dock"), m_dockView);
    rootContext()->setContextProperty(QStringLiteral("dockConfig"), this);
    rootContext()->setContextObject(new KLocalizedContext(this));
    auto source = QUrl::fromLocalFile(m_containment->corona()->kPackage().filePath("lattedockconfigurationui"));
    setSource(source);
    syncSlideEffect();
}

inline Qt::WindowFlags DockConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

void DockConfigView::syncGeometry()
{
    if (!m_containment || !rootObject())
        return;
        
    const auto location = m_containment->location();
    const auto sGeometry = screen()->geometry();
    
    switch (m_containment->formFactor()) {
        case Plasma::Types::Horizontal: {
            const QSize size(rootObject()->width(), rootObject()->height());
            setMaximumSize(size);
            setMinimumSize(size);
            resize(size);
            
            if (location == Plasma::Types::TopEdge) {
                setPosition(sGeometry.center().x() - size.width() / 2
                            , m_dockView->currentThickness());
                            
            } else if (location == Plasma::Types::BottomEdge) {
                setPosition(sGeometry.center().x() - size.width() / 2
                            , sGeometry.height() - m_dockView->currentThickness() - size.height());
                            
            }
        }
        break;
        
        case Plasma::Types::Vertical: {
            const QSize size(rootObject()->width(), rootObject()->height());
            setMaximumSize(size);
            setMinimumSize(size);
            resize(size);
            
            if (location == Plasma::Types::LeftEdge) {
                setPosition(m_dockView->currentThickness()
                            , sGeometry.center().y() - size.height() / 2);
                            
            } else if (location == Plasma::Types::RightEdge) {
                setPosition(sGeometry.width() - m_dockView->currentThickness() - size.width()
                            , sGeometry.center().y() - size.height() / 2);
                            
            }
        }
        break;
        
        default:
            qWarning() << "no sync geometry, wrong formFactor";
            break;
    }
}

void DockConfigView::syncSlideEffect()
{
    if (!m_containment)
        return;
        
    KWindowEffects::SlideFromLocation slideLocation{KWindowEffects::NoEdge};
    
    switch (m_containment->location()) {
        case Plasma::Types::TopEdge:
            slideLocation = KWindowEffects::TopEdge;
            break;
            
        case Plasma::Types::RightEdge:
            slideLocation = KWindowEffects::RightEdge;
            break;
            
        case Plasma::Types::BottomEdge:
            slideLocation = KWindowEffects::BottomEdge;
            break;
            
        case Plasma::Types::LeftEdge:
            slideLocation = KWindowEffects::LeftEdge;
            break;
            
        default:
            qDebug() << staticMetaObject.className() << "wrong location";// << qEnumToStr(m_containment->location());
            break;
    }
    
    KWindowEffects::slideWindow(winId(), slideLocation, -1);
}

void DockConfigView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);
    
    KWindowSystem::setType(winId(), NET::Dock);
    setFlags(wFlags());
    KWindowSystem::setState(winId(), NET::KeepAbove | NET::SkipPager | NET::SkipTaskbar);
    KWindowSystem::forceActiveWindow(winId());
    KWindowEffects::enableBlurBehind(winId(), true);
    
    syncGeometry();
    syncSlideEffect();
    
    if (m_containment)
        m_containment->setUserConfiguring(true);
        
    m_screenSyncTimer.start();
    
    QTimer::singleShot(400, this, &DockConfigView::syncGeometry);
}

void DockConfigView::hideEvent(QHideEvent *ev)
{
    if (m_containment && m_dockView) {
        m_containment->setUserConfiguring(false);
    }
    
    QQuickWindow::hideEvent(ev);
}

void DockConfigView::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev);
    const auto *focusWindow = qGuiApp->focusWindow();
    
    if (focusWindow && focusWindow->flags().testFlag(Qt::Popup))
        return;
        
    if (!m_blockFocusLost) {
        hide();
    }
}

void DockConfigView::immutabilityChanged(Plasma::Types::ImmutabilityType type)
{
    if (type != Plasma::Types::Mutable && isVisible()) {
        hide();
    }
}

void DockConfigView::setSticker(bool blockFocusLost)
{
    if (m_blockFocusLost == blockFocusLost) {
        return;
    }
    
    m_blockFocusLost = blockFocusLost;
}

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
