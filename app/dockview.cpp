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

#include "dockview.h"
#include "dockconfigview.h"
#include "dockcorona.h"
#include "visibilitymanager.h"
#include "../liblattedock/windowsystem.h"

#include <QAction>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QMetaEnum>

#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <KActionCollection>
#include <KLocalizedContext>

namespace Latte {

DockView::DockView(Plasma::Corona *corona, QScreen *targetScreen)
    : PlasmaQuick::ContainmentView(corona)
{
    setVisible(false);
    setTitle(corona->kPackage().metadata().name());
    setIcon(QIcon::fromTheme(corona->kPackage().metadata().iconName()));
    
    setResizeMode(QuickViewSharedEngine::SizeRootObjectToView);
    setClearBeforeRendering(true);
    
    if (targetScreen)
        adaptToScreen(targetScreen);
    else
        adaptToScreen(qGuiApp->primaryScreen());
        
    m_timerGeometry.setSingleShot(true);
    m_timerGeometry.setInterval(400);
    
    m_lockGeometry.setSingleShot(true);
    m_lockGeometry.setInterval(700);
    
    connect(this, SIGNAL(localDockGeometryChanged()), this, SLOT(updateAbsDockGeometry()));
    connect(this, SIGNAL(xChanged(int)), this, SLOT(updateAbsDockGeometry()));
    connect(this, SIGNAL(yChanged(int)), this, SLOT(updateAbsDockGeometry()));
    
    connect(this, &DockView::containmentChanged
    , this, [&]() {
        if (!containment())
            return;
            
        if (!m_visibility) {
            m_visibility = new VisibilityManager(this);
        }

        QAction *lockWidgetsAction = containment()->actions()->action("lock widgets");
        containment()->actions()->removeAction(lockWidgetsAction);

        //QAction *addWidgetsAction = containment()->actions()->action("add widgets");
        //containment()->actions()->removeAction(addWidgetsAction);
        
    }, Qt::DirectConnection);
}

DockView::~DockView()
{
}

void DockView::init()
{
    connect(this, &DockView::screenChanged
            , this, &DockView::adaptToScreen
            , Qt::QueuedConnection);
            
            
    connect(&m_timerGeometry, &QTimer::timeout, [&]() {
        initWindow();
    });
    
    connect(this, &DockView::locationChanged, [&]() {
        //! avoid glitches
        m_timerGeometry.start();
    });
    
    connect(&WindowSystem::self(), &WindowSystem::compositingChanged
    , this, [&]() {
        emit compositingChanged();
    } , Qt::QueuedConnection);
    
    connect(this, &DockView::screenGeometryChanged
            , this, &DockView::updateDockPosition
            , Qt::QueuedConnection);
            
    connect(this, SIGNAL(widthChanged(int)), this, SIGNAL(widthChanged()));
    connect(this, SIGNAL(heightChanged(int)), this, SIGNAL(heightChanged()));
    
    rootContext()->setContextProperty(QStringLiteral("dock"), this);
    engine()->rootContext()->setContextObject(new KLocalizedContext(this));
    
    // engine()->rootContext()->setContextProperty(QStringLiteral("dock"), this);
    setSource(corona()->kPackage().filePath("lattedockui"));
    
    connect(this, SIGNAL(xChanged(int)), this, SLOT(updateDockPositionSlot()));
    connect(this, SIGNAL(yChanged(int)), this, SLOT(updateDockPositionSlot()));
    
    connect(&m_lockGeometry, &QTimer::timeout, [&]() {
        updateDockPosition();
    });
    
    qDebug() << "SOURCE:" << source();
    
    initialize();
}


void DockView::initialize()
{
    m_secondInitPass = true;
    m_timerGeometry.start();
}

void DockView::initWindow()
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

void DockView::updateDockPositionSlot()
{
    if (!m_lockGeometry.isActive()) {
        m_lockGeometry.start();
    }
}

//!BEGIN SLOTS
void DockView::adaptToScreen(QScreen *screen)
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

void DockView::addNewDock()
{
    DockCorona *corona = qobject_cast<DockCorona *>(this->corona());
    
    if (corona) {
        corona->loadDefaultLayout();
    }
}

void DockView::removeDock()
{
    DockCorona *corona = qobject_cast<DockCorona *>(this->corona());
    
    if (corona->containments().count() > 1) {
        QAction *removeAct = containment()->actions()->action(QStringLiteral("remove"));
        
        if (removeAct) {
            removeAct->trigger();
        }
    }
}

QQmlListProperty<QScreen> DockView::screens()
{
    return QQmlListProperty<QScreen>(this, nullptr, &countScreens, &atScreens);
}

int DockView::countScreens(QQmlListProperty<QScreen> *property)
{
    Q_UNUSED(property)
    return qGuiApp->screens().count();
}

QScreen *DockView::atScreens(QQmlListProperty<QScreen> *property, int index)
{
    Q_UNUSED(property)
    return qGuiApp->screens().at(index);
}

void DockView::showConfigurationInterface(Plasma::Applet *applet)
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
        m_configView = new DockConfigView(c, this);
    } else {
        m_configView = new PlasmaQuick::ConfigView(applet);
    }
    
    m_configView->init();
    m_configView->show();
    m_configView->requestActivate();
}

void DockView::resizeWindow()
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

void DockView::setLocalDockGeometry(const QRect &geometry)
{
    if (geometry == m_localDockGeometry) {
        return;
    }
    
    m_localDockGeometry = geometry;
    
    emit localDockGeometryChanged();
}

void DockView::updateAbsDockGeometry()
{
    QRect absoluteGeometry {x() + m_localDockGeometry.x(), y() + m_localDockGeometry.y(), m_localDockGeometry.width(), m_localDockGeometry.height()};
    m_visibility->updateDockGeometry(absoluteGeometry);
}

inline void DockView::updateDockPosition()
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

int DockView::currentThickness() const
{
    if (containment()->formFactor() == Plasma::Types::Vertical) {
        return m_maskArea.isNull() ? width() : m_maskArea.width();
    } else {
        return m_maskArea.isNull() ? height() : m_maskArea.height();
    }
}

bool DockView::compositing() const
{
    return WindowSystem::self().compositingActive();
}

/*Candil::VisibilityManager *DockView::visibility()
{
    return  m_visibility.data();
}*/

int DockView::maxThickness() const
{
    return m_maxThickness;
}

void DockView::setMaxThickness(int thickness)
{
    if (m_maxThickness == thickness)
        return;
        
    m_maxThickness = thickness;
    m_timerGeometry.start();
    emit maxThicknessChanged();
}

int DockView::length() const
{
    return m_length;
}

void DockView::setLength(int length)
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

int DockView::maxLength() const
{
    return m_maxLength;
}

void DockView::setMaxLength(int maxLength)
{
    if (m_maxLength == maxLength)
        return;
        
    m_maxLength = maxLength;
    emit maxLengthChanged();
}


QRect DockView::maskArea() const
{
    return m_maskArea;
}

void DockView::setMaskArea(QRect area)
{
    if (m_maskArea == area) {
        return;
    }
    
    m_maskArea = area;
    
    setMask(m_maskArea);
    
    //qDebug() << "dock mask set:" << m_maskArea;
    emit maskAreaChanged();
}

/*Dock::Alignment DockView::alignment() const
{
    return m_alignment;
}
void DockView::setAlignment(Dock::Alignment align)
{
    if (m_alignment == align)
        return;
    m_alignment = align;
    emit alignmentChanged();
}
*/
int DockView::offset() const
{
    return m_offset;
}

void DockView::setOffset(int offset)
{
    if (m_offset == offset)
        return;
        
    m_offset = offset;
    m_timerGeometry.start();
    emit offsetChanged();
}

bool DockView::tasksPresent()
{
    foreach (Plasma::Applet *applet, containment()->applets()) {
        KPluginMetaData meta = applet->kPackage().metadata();

        if (meta.pluginId() == "org.kde.latte.plasmoid") {
            return true;
        }
    }

    return false;
}

void DockView::updateOffset()
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

VisibilityManager *DockView::visibility()
{
    return m_visibility;
}

bool DockView::event(QEvent *e)
{
    emit eventTriggered(e);
    
    return ContainmentView::event(e);
}

/*void DockView::showEvent(QShowEvent *ev)
{
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setOnAllDesktops(winId(), true);
    //QQuickWindow::showEvent(ev);
    ContainmentView::showEvent(ev);
}*/

bool DockView::containmentContainsPosition(const QPointF &point) const
{
    QQuickItem *containmentItem = containment()->property("_plasma_graphicObject").value<QQuickItem *>();
    
    if (!containmentItem) {
        return false;
    }
    
    return QRectF(containmentItem->mapToScene(QPoint(0, 0)), QSizeF(containmentItem->width(), containmentItem->height())).contains(point);
}

QPointF DockView::positionAdjustedForContainment(const QPointF &point) const
{
    QQuickItem *containmentItem = containment()->property("_plasma_graphicObject").value<QQuickItem *>();
    
    if (!containmentItem) {
        return point;
    }
    
    QRectF containmentRect(containmentItem->mapToScene(QPoint(0, 0)), QSizeF(containmentItem->width(), containmentItem->height()));
    
    return QPointF(qBound(containmentRect.left() + 2, point.x(), containmentRect.right() - 2),
                   qBound(containmentRect.top() + 2, point.y(), containmentRect.bottom() - 2));
}

QList<int> DockView::freeEdges() const
{
    QList<Plasma::Types::Location> edges = corona()->freeEdges(containment()->screen());
    
    QList<int> edgesInt;
    
    foreach (Plasma::Types::Location edge, edges) {
        edgesInt.append(static_cast<int>(edge));
    }
    
    return edgesInt;
}

void DockView::saveConfig()
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

void DockView::restoreConfig()
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

QVariantList DockView::containmentActions()
{
    QVariantList actions;

    /*if (containment()->corona()->immutability() != Plasma::Types::Mutable) {
        return actions;
    }*/

    //FIXME: the trigger string it should be better to be supported this way
    //const QString trigger = Plasma::ContainmentActions::eventToString(event);
    const QString trigger = "RightButton;NoModifier";

    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (!plugin) {
        return actions;
    }

    if (plugin->containment() != containment()) {
        plugin->setContainment(containment());

        // now configure it
        KConfigGroup cfg(containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    foreach (QAction *ac, plugin->contextualActions()) {
        actions << QVariant::fromValue<QAction *>(ac);
    }

    return actions;
}

}
//!END SLOTS
//!END namespace
