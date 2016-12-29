/*
 * Copyright 2014  Bhushan Shah <bhush94@gmail.com>
 * Copyright 2014 Marco Martin <notmart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "nowdockview.h"
#include "nowdockconfigview.h"
#include "visibilitymanager.h"

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QMetaEnum>
//#include <QtX11Extras/QX11Info>

#include <NETWM>
#include <KWindowSystem>
#include <Plasma/Containment>
#include <KActionCollection>
#include <KLocalizedContext>

#include "nowdockcorona.h"


NowDockView::NowDockView(Plasma::Corona *corona, QScreen *targetScreen)
    : PlasmaQuick::ContainmentView(corona),
      m_corona(corona)
{
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    
    setVisible(false);
    setTitle(corona->kPackage().metadata().name());
    setIcon(QIcon::fromTheme(corona->kPackage().metadata().iconName()));
    
    setResizeMode(QuickViewSharedEngine::SizeRootObjectToView);
    setClearBeforeRendering(true);
    /* setFlags(Qt::FramelessWindowHint
               | Qt::WindowStaysOnTopHint
               | Qt::NoDropShadowWindowHint
               | Qt::WindowDoesNotAcceptFocus);*/
    
    //    NETWinInfo winfo(QX11Info::connection(), winId(), winId(), 0, 0);
    //   winfo.setAllowedActions(NET::ActionChangeDesktop);
    
    if (targetScreen)
        adaptToScreen(targetScreen);
    else
        adaptToScreen(qGuiApp->primaryScreen());
        
    m_timerGeometry.setSingleShot(true);
    m_timerGeometry.setInterval(400);
    
    m_lockGeometry.setSingleShot(true);
    m_lockGeometry.setInterval(700);
    
    connect(this, &NowDockView::containmentChanged
    , this, [&]() {
        if (!containment())
            return;
            
        if (!m_visibility) {
            m_visibility = new VisibilityManager(this);
        }
        
        m_visibility->setContainment(containment());
    }, Qt::DirectConnection);
}

NowDockView::~NowDockView()
{
}

void NowDockView::init()
{
    connect(this, &NowDockView::screenChanged
            , this, &NowDockView::adaptToScreen
            , Qt::QueuedConnection);
            
            
    connect(&m_timerGeometry, &QTimer::timeout, [&]() {
        initWindow();
    });
    
    connect(this, &NowDockView::locationChanged, [&]() {
        //! avoid glitches
        m_timerGeometry.start();
    });
    
    connect(KWindowSystem::self(), &KWindowSystem::compositingChanged
    , this, [&]() {
        emit compositingChanged();
    } , Qt::QueuedConnection);
    
    connect(this, &NowDockView::screenGeometryChanged
            , this, &NowDockView::updateDockPosition
            , Qt::QueuedConnection);
            
    connect(this, SIGNAL(widthChanged(int)), this, SIGNAL(widthChanged()));
    connect(this, SIGNAL(heightChanged(int)), this, SIGNAL(heightChanged()));
    
    rootContext()->setContextProperty(QStringLiteral("dock"), this);
    engine()->rootContext()->setContextObject(new KLocalizedContext(this));
    
    // engine()->rootContext()->setContextProperty(QStringLiteral("dock"), this);
    setSource(corona()->kPackage().filePath("nowdockui"));
    
    
    connect(this, SIGNAL(xChanged(int)), this, SLOT(updateDockPositionSlot()));
    connect(this, SIGNAL(yChanged(int)), this, SLOT(updateDockPositionSlot()));
    
    connect(&m_lockGeometry, &QTimer::timeout, [&]() {
        updateDockPosition();
    });
    
    qDebug() << "SOURCE:" << source();
    
    initialize();
}


void NowDockView::initialize()
{
    m_secondInitPass = true;
    m_timerGeometry.start();
}

void NowDockView::initWindow()
{
    //  m_visibility->updateVisibilityFlags();
    
    updateDockPosition();
    resizeWindow();
    
    // The initialization phase makes two passes because
    // changing the window style and type wants a small delay
    // and afterwards the second pass positions them correctly
    if (m_secondInitPass) {
        m_timerGeometry.start();
        m_secondInitPass = false;
        setVisible(true);
    }
}

void NowDockView::updateDockPositionSlot()
{
    if (!m_lockGeometry.isActive()) {
        m_lockGeometry.start();
    }
}

//!BEGIN SLOTS
void NowDockView::adaptToScreen(QScreen *screen)
{
    setScreen(screen);
    
    if (formFactor() == Plasma::Types::Vertical)
        m_maxLength = screen->size().height();
    else
        m_maxLength = screen->size().width();
        
//   KWindowSystem::setOnAllDesktops(winId(), true);
//   KWindowSystem::setType(winId(), NET::Dock);

    if (containment())
        containment()->reactToScreenChange();
        
    m_timerGeometry.start();
}

void NowDockView::addNewDock()
{
    NowDockCorona *corona = dynamic_cast<NowDockCorona *>(m_corona);
    
    if (corona) {
        corona->loadDefaultLayout();
    }
}

void NowDockView::removeDock()
{
    NowDockCorona *corona = dynamic_cast<NowDockCorona *>(m_corona);
    
    if (corona->containments().count() > 1) {
        QAction *removeAct = containment()->actions()->action(QStringLiteral("remove"));
        
        if (removeAct) {
            removeAct->trigger();
        }
    }
}

QQmlListProperty<QScreen> NowDockView::screens()
{
    return QQmlListProperty<QScreen>(this, nullptr, &countScreens, &atScreens);
}

int NowDockView::countScreens(QQmlListProperty<QScreen> *property)
{
    Q_UNUSED(property)
    return qGuiApp->screens().count();
}

QScreen *NowDockView::atScreens(QQmlListProperty<QScreen> *property, int index)
{
    Q_UNUSED(property)
    return qGuiApp->screens().at(index);
}

void NowDockView::showConfigurationInterface(Plasma::Applet *applet)
{
    if (!applet || !applet->containment())
        return;
        
    Plasma::Containment *c = qobject_cast<Plasma::Containment *>(applet);
    
    if (m_configView && c && c->isContainment() && c == containment()) {
        if (m_configView->isVisible()) {
            m_configView->hide();
        } else {
            m_configView->show();
            m_configView->requestActivate();
        }
        
        return;
    } else if (m_configView) {
        if (m_configView->applet() == applet) {
            m_configView->show();
            m_configView->requestActivate();
            return;
        } else {
            m_configView->hide();
            m_configView->deleteLater();
        }
    }
    
    if (c && containment() && c->isContainment() && c->id() == containment()->id()) {
        m_configView = new NowDockConfigView(c, this);
    } else {
        m_configView = new PlasmaQuick::ConfigView(applet);
    }
    
    m_configView->init();
    m_configView->show();
    m_configView->requestActivate();
}

void NowDockView::resizeWindow()
{
    setVisible(true);
    
    QSize screenSize = screen()->size();
    
    if (formFactor() == Plasma::Types::Vertical) {
        const QSize size{maxThickness(), screenSize.height()};
        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);
        
        qDebug() << "dock size:" << size;
    } else {
        const QSize size{screenSize.width(), maxThickness()};
        setMinimumSize(size);
        setMaximumSize(size);
        resize(size);
        
        qDebug() << "dock size:" << size;
    }
}

inline void NowDockView::updateDockPosition()
{
    if (!containment())
        return;
        
    const QRect screenGeometry = screen()->geometry();
    QPoint position;
    
    qDebug() << "current dock geometry: " << geometry();
    
    // containment()->setFormFactor(Plasma::Types::Horizontal);
    position = {0, 0};
    m_maxLength = screenGeometry.width();
    
    switch (location()) {
        case Plasma::Types::TopEdge:
            containment()->setFormFactor(Plasma::Types::Horizontal);
            position = {screenGeometry.x(), screenGeometry.y()};
            m_maxLength = screenGeometry.width();
            break;
            
        case Plasma::Types::BottomEdge:
            containment()->setFormFactor(Plasma::Types::Horizontal);
            position = {screenGeometry.x(), screenGeometry.y() + screenGeometry.height() - height()};
            m_maxLength = screenGeometry.width();
            break;
            
        case Plasma::Types::RightEdge:
            containment()->setFormFactor(Plasma::Types::Vertical);
            position = {screenGeometry.x() + screenGeometry.width() - width(), screenGeometry.y()};
            m_maxLength = screenGeometry.height();
            break;
            
        case Plasma::Types::LeftEdge:
            containment()->setFormFactor(Plasma::Types::Vertical);
            position = {screenGeometry.x(), screenGeometry.y()};
            m_maxLength = screenGeometry.height();
            break;
            
        default:
            qWarning() << "wrong location, couldn't update the panel position"
                       << location();
    }
    
    emit maxLengthChanged();
    setX(position.x());
    setY(position.y());
    //setPosition(position);
    qDebug() << "dock position:" << position;
}

int NowDockView::currentThickness() const
{
    if (containment()->formFactor() == Plasma::Types::Vertical) {
        return m_maskArea.isNull() ? width() : m_maskArea.width();
    } else {
        return m_maskArea.isNull() ? height() : m_maskArea.height();
    }
}

bool NowDockView::compositing() const
{
    return KWindowSystem::compositingActive();
}

/*Candil::VisibilityManager *NowDockView::visibility()
{
    return  m_visibility.data();
}*/

int NowDockView::maxThickness() const
{
    return m_maxThickness;
}

void NowDockView::setMaxThickness(int thickness)
{
    if (m_maxThickness == thickness)
        return;
        
    m_maxThickness = thickness;
    m_timerGeometry.start();
    emit maxThicknessChanged();
}

int NowDockView::length() const
{
    return m_length;
}

void NowDockView::setLength(int length)
{
    if (m_length == length)
        return;
        
    if (length > m_maxLength)
        m_length = m_maxLength;
    else
        m_length = length;
        
    m_timerGeometry.start();
    emit lengthChanged();
}

int NowDockView::maxLength() const
{
    return m_maxLength;
}

void NowDockView::setMaxLength(int maxLength)
{
    if (m_maxLength == maxLength)
        return;
        
    m_maxLength = maxLength;
    emit maxLengthChanged();
}


QRect NowDockView::maskArea() const
{
    return m_maskArea;
}

void NowDockView::setMaskArea(QRect area)
{
    if (m_maskArea == area) {
        return;
    }
    
    m_maskArea = area;
    m_visibility->setMaskArea(area);
    
    setMask(m_maskArea);
    
    //qDebug() << "dock mask set:" << m_maskArea;
    emit maskAreaChanged();
}

/*Dock::Alignment NowDockView::alignment() const
{
    return m_alignment;
}

void NowDockView::setAlignment(Dock::Alignment align)
{
    if (m_alignment == align)
        return;

    m_alignment = align;
    emit alignmentChanged();
}
*/
int NowDockView::offset() const
{
    return m_offset;
}

void NowDockView::setOffset(int offset)
{
    if (m_offset == offset)
        return;
        
    m_offset = offset;
    m_timerGeometry.start();
    emit offsetChanged();
}

void NowDockView::updateOffset()
{
    if (!containment())
        return;
        
    const float offsetPercent = containment()->config().readEntry("offset").toFloat();
    const int offset = offsetPercent * (m_maxLength - m_length) / 2;
    
    if (offset == m_offset)
        return;
        
    m_offset = offset;
    emit offsetChanged();
}

VisibilityManager *NowDockView::visibility()
{
    return m_visibility;
}

bool NowDockView::event(QEvent *e)
{

    /* if (ev->type() == QEvent::Enter) {
         m_visibility->show();
         emit entered();
     } else if (ev->type() == QEvent::Leave) {
         m_visibility->restore();
         emit exited();
     } */
    
    //return QQuickWindow::event(e);
    if (m_visibility) {
        m_visibility->event(e);
    }
    
    return ContainmentView::event(e);
}

/*void NowDockView::showEvent(QShowEvent *ev)
{
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setOnAllDesktops(winId(), true);

    //QQuickWindow::showEvent(ev);
    ContainmentView::showEvent(ev);
}*/

bool NowDockView::containmentContainsPosition(const QPointF &point) const
{
    QQuickItem *containmentItem = containment()->property("_plasma_graphicObject").value<QQuickItem *>();
    
    if (!containmentItem) {
        return false;
    }
    
    return QRectF(containmentItem->mapToScene(QPoint(0, 0)), QSizeF(containmentItem->width(), containmentItem->height())).contains(point);
}

QPointF NowDockView::positionAdjustedForContainment(const QPointF &point) const
{
    QQuickItem *containmentItem = containment()->property("_plasma_graphicObject").value<QQuickItem *>();
    
    if (!containmentItem) {
        return point;
    }
    
    QRectF containmentRect(containmentItem->mapToScene(QPoint(0, 0)), QSizeF(containmentItem->width(), containmentItem->height()));
    
    return QPointF(qBound(containmentRect.left() + 2, point.x(), containmentRect.right() - 2),
                   qBound(containmentRect.top() + 2, point.y(), containmentRect.bottom() - 2));
}

QList<int> NowDockView::freeEdges() const
{
    QList<Plasma::Types::Location> edges = m_corona->freeEdges(containment()->screen());

    QList<int> edgesInt;

    foreach (Plasma::Types::Location edge, edges) {
        edgesInt.append((int)edge);
    }

    return edgesInt;
}

void NowDockView::saveConfig()
{
    if (!containment())
        return;
        
    const auto writeEntry = [&](const char *entry, const QVariant & value) {
        containment()->config().writeEntry(entry, value);
    };
    
    //! convert offset to percent, range [-1,1] 0 is Centered
    //! offsetPercent = offset * 2 / (maxLength - length)
    //  const float offsetPercent = m_offset * 2.0f / (m_maxLength - m_length);
    //  writeEntry("offset", offsetPercent);
    //  writeEntry("iconSize", m_iconSize);
    //  writeEntry("zoomFactor", m_zoomFactor);
    //  writeEntry("alignment", static_cast<int>(m_alignment));
}

void NowDockView::restoreConfig()
{
    if (!containment())
        return;
        
    const auto readEntry = [&](const char *entry, QVariant defaultValue) -> QVariant {
        return containment()->config().readEntry(entry, defaultValue);
    };
    //! convert offset-percent to pixels
    //! offset = offsetPercent * (maxLength - length) / 2
//   const float offsetPercent {readEntry("offset", 0).toFloat()};
    //  const int offset {static_cast<int>(offsetPercent * (m_maxLength - m_length) / 2)};
    //  setOffset(offset);
    
    //  setIconSize(readEntry("iconSize", 32).toInt());
    //  setZoomFactor(readEntry("zoomFactor", 1.0).toFloat());
    //  setAlignment(static_cast<Dock::Alignment>(readEntry("alignment", Dock::Center).toInt()));
}

//!END SLOTS


//!END namespace
