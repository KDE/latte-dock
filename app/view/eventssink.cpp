/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "eventssink.h"

// local
#include "view.h"
#include "positioner.h"

// Qt
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QRectF>


namespace Latte {
namespace ViewPart {

EventsSink::EventsSink(Latte::View *parent)
    : QObject(parent),
      m_view(parent)
{
}

EventsSink::~EventsSink()
{
}

QQuickItem *EventsSink::originItem() const
{
    return m_originItem;
}

QQuickItem *EventsSink::destinationItem() const
{
    return m_destinationItem;
}

void EventsSink::setSink(QQuickItem *origin, QQuickItem *destination)
{
    if ((m_originItem == origin) && (m_destinationItem == destination)) {
        return;
    }

    m_originItem = origin;
    m_destinationItem = destination;


    emit itemsChanged();
}

bool EventsSink::isActive()
{
    return ((m_originItem != nullptr) && (m_destinationItem != nullptr));
}

void EventsSink::release()
{
    setSink(nullptr, nullptr);
}

QEvent *EventsSink::onEvent(QEvent *e)
{
    if (!e) {
        return nullptr;
    }

    if (!isActive()) {
        return e;
    }

    QEvent *sunkevent = e;

    switch (e->type()) {
    case QEvent::Leave:
        release();
        break;
    case QEvent::DragEnter:
        if (auto de = static_cast<QDragEnterEvent *>(e)) {
            QPointF point = de->posF();
            QPointF originInternal = m_originItem->mapFromScene(point);

            if (m_originItem->contains(originInternal)) {
                auto de2 = new QDragEnterEvent(positionAdjustedForDestination(point).toPoint(),
                                               de->possibleActions(),
                                               de->mimeData(),
                                               de->mouseButtons(),
                                               de->keyboardModifiers());
                sunkevent = de2;
            } else {
                release();
            }
        }
        break;

    case QEvent::DragMove:
        if (auto de = static_cast<QDragMoveEvent *>(e)) {
            QPointF point = de->posF();
            QPointF originInternal = m_originItem->mapFromScene(point);

            if (m_originItem->contains(originInternal)) {
                auto de2 = new QDragMoveEvent(positionAdjustedForDestination(point).toPoint(),
                                              de->possibleActions(),
                                              de->mimeData(),
                                              de->mouseButtons(),
                                              de->keyboardModifiers());

                sunkevent = de2;
            } else {
                release();
            }
        }
        break;

    case QEvent::Drop:
        if (auto de = static_cast<QDropEvent *>(e)) {
            QPointF point = de->posF();
            QPointF originInternal = m_originItem->mapFromScene(point);

            if (m_originItem->contains(originInternal)) {
                auto de2 = new QDropEvent(positionAdjustedForDestination(point).toPoint(),
                                          de->possibleActions(),
                                          de->mimeData(),
                                          de->mouseButtons(),
                                          de->keyboardModifiers());

                sunkevent = de2;
            } else {
                release();
            }
        }

        break;

    case QEvent::MouseMove:
        if (auto me = dynamic_cast<QMouseEvent *>(e)) {
            QPointF originInternal = m_originItem->mapFromScene(me->windowPos());

            if (m_view->positioner() && m_view->positioner()->isCursorInsideView() && m_originItem->contains(originInternal)) {
                auto positionadjusted = positionAdjustedForDestination(me->windowPos());
                auto me2 = new QMouseEvent(me->type(),
                                           positionadjusted,
                                           positionadjusted,
                                           positionadjusted + m_view->position(),
                                           me->button(), me->buttons(), me->modifiers());

                sunkevent = me2;
            } else {
                release();
            }
        }
        break;

    case QEvent::MouseButtonPress:
        if (auto me = dynamic_cast<QMouseEvent *>(e)) {
            QPointF originInternal = m_originItem->mapFromScene(me->windowPos());

            if (m_originItem->contains(originInternal)) {
                auto positionadjusted = positionAdjustedForDestination(me->windowPos());
                auto me2 = new QMouseEvent(me->type(),
                                           positionadjusted,
                                           positionadjusted,
                                           positionadjusted + m_view->position(),
                                           me->button(), me->buttons(), me->modifiers());

                qDebug() << "Sunk Event:: sunk event pressed...";
                sunkevent = me2;
            } else {
                release();
            }
        }
        break;

    case QEvent::MouseButtonRelease:
        if (auto me = dynamic_cast<QMouseEvent *>(e)) {
            QPointF originInternal = m_originItem->mapFromScene(me->windowPos());

            if (m_originItem->contains(originInternal)) {
                auto positionadjusted = positionAdjustedForDestination(me->windowPos());
                auto me2 = new QMouseEvent(me->type(),
                                           positionadjusted,
                                           positionadjusted,
                                           positionadjusted + m_view->position(),
                                           me->button(), me->buttons(), me->modifiers());

                sunkevent = me2;
            } else {
                release();
            }
        }
        break;

    case QEvent::Wheel:
        if (auto we = dynamic_cast<QWheelEvent *>(e)) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
            QPoint pos = QPoint(we->x(), we->y());
#else
            QPoint pos = we->position().toPoint();
#endif
            QPointF originInternal = m_originItem->mapFromScene(pos);

            if (m_originItem->contains(originInternal)) {
                auto positionadjusted = positionAdjustedForDestination(pos);
                auto we2 = new QWheelEvent(positionadjusted,
                                           positionadjusted + m_view->position(),
                                           we->pixelDelta(), we->angleDelta(), we->angleDelta().y(),
                                           we->orientation(), we->buttons(), we->modifiers(), we->phase());

                sunkevent = we2;
            } else {
                release();
            }
        }
        break;
    default:
        break;
    }

    return sunkevent;
}

QPointF EventsSink::positionAdjustedForDestination(const QPointF &point) const
{
    QRectF destinationRectToScene = m_destinationItem->mapRectToScene(QRectF(0, 0, m_destinationItem->width(), m_destinationItem->height()));

    return QPointF(qBound(destinationRectToScene.left(), point.x(), destinationRectToScene.right()),
                   qBound(destinationRectToScene.top(), point.y(), destinationRectToScene.bottom()));
}

}
}

