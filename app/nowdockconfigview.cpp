/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*
*  This file is part of Candil-Dock
*
*  Candil-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 3 of
*  the License, or (at your option) any later version.
*
*  Candil-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "nowdockconfigview.h"
#include "nowdockview.h"
#include "nowdockcorona.h"

#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

#include <KLocalizedContext>
#include <KWindowSystem>
#include <KWindowEffects>

#include <plasma/package.h>

NowDockConfigView::NowDockConfigView(Plasma::Containment *containment, NowDockView *dockView, QWindow *parent)
    : PlasmaQuick::ConfigView(containment, parent), m_containment(containment), m_dockView(dockView)
{
    m_deleterTimer.setSingleShot(true);
    m_deleterTimer.setInterval(10 * 1000);
    connect(&m_deleterTimer, &QTimer::timeout, this, &QObject::deleteLater);
    connect(dockView, &QObject::destroyed, this, &QObject::deleteLater);
    
    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(100);
    
    connect(dockView, SIGNAL(screenChanged(QScreen *)), &m_screenSyncTimer, SLOT(start()));
    
    connect(&m_screenSyncTimer, &QTimer::timeout, this, [this]() {
        setScreen(m_dockView->screen());
        setFlags(wFlags());
        syncGeometry();
        syncSlideEffect();
    });
    
    connect(containment, &Plasma::Containment::immutabilityChanged, this, &NowDockConfigView::immutabilityChanged);

    NowDockCorona *corona = dynamic_cast<NowDockCorona *>(m_containment->corona());

    if (corona) {
        connect(corona, &NowDockCorona::configurationShown, this, &NowDockConfigView::configurationShown);
    }

    /*   connect(containment, &Plasma::Containment::immutabilityChanged
       , [&](Plasma::Types::ImmutabilityType type) {
           if (type != Plasma::Types::Mutable && this && isVisible())
               hide();
       });*/
}

NowDockConfigView::~NowDockConfigView()
{
}

void NowDockConfigView::init()
{
    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    rootContext()->setContextProperty(QStringLiteral("dock"), m_dockView);
    engine()->rootContext()->setContextObject(new KLocalizedContext(this));
    auto source = QUrl::fromLocalFile(m_containment->corona()->kPackage().filePath("nowdockconfigurationui"));
    setSource(source);
    syncSlideEffect();
}

inline Qt::WindowFlags NowDockConfigView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

void NowDockConfigView::syncGeometry()
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
            //<< qEnumToStr(m_containment->formFactor());
            break;
    }
}

void NowDockConfigView::syncSlideEffect()
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

void NowDockConfigView::showEvent(QShowEvent *ev)
{
    KWindowSystem::setType(winId(), NET::Dock);
    setFlags(wFlags());
    KWindowSystem::setState(winId(), NET::KeepAbove | NET::SkipPager | NET::SkipTaskbar);
    KWindowSystem::forceActiveWindow(winId());
    KWindowEffects::enableBlurBehind(winId(), true);
    
    syncGeometry();
    syncSlideEffect();
    
    if (m_containment)
        m_containment->setUserConfiguring(true);
        
    //  m_dockView->visibility()->forceShow(true);
    //  m_dockView->visibility()->showImmediately();
    m_screenSyncTimer.start();
    m_deleterTimer.stop();

    ConfigView::showEvent(ev);

    //trigger showing configuration window through corona
    //in order to hide all alternative configuration windows
    NowDockCorona *corona = dynamic_cast<NowDockCorona *>(m_containment->corona());

    if (corona) {
        emit corona->configurationShown(this);
    }
}

void NowDockConfigView::hideEvent(QHideEvent *ev)
{
    //  m_dockView->visibility()->forceShow(false);
    //  m_dockView->visibility()->restore();
    m_deleterTimer.start();
    
    if (m_containment) {
        m_dockView->saveConfig();
        m_containment->setUserConfiguring(false);
    }
    
    ConfigView::hideEvent(ev);
}

void NowDockConfigView::focusOutEvent(QFocusEvent *ev)
{
    //FIXME: I can understand why we need to hide on focus out
    Q_UNUSED(ev);
    const auto *focusWindow = qGuiApp->focusWindow();
    
    if (focusWindow && focusWindow->flags().testFlag(Qt::Popup))
        return;
        
        
    // hide();
}

void NowDockConfigView::configurationShown(PlasmaQuick::ConfigView *configView)
{
    if ((configView != this) && isVisible()) {
        hide();
    }
}

void NowDockConfigView::immutabilityChanged(Plasma::Types::ImmutabilityType type)
{
    if (type != Plasma::Types::Mutable && isVisible()) {
        hide();
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
