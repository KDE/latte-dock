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

#include "parabolic.h"

// local
#include "view.h"

// Qt
#include <QMetaObject>

namespace Latte {
namespace ViewPart {

Parabolic::Parabolic(Latte::View *parent)
    : QObject(parent),
      m_view(parent)
{
    m_parabolicItemNullifier.setInterval(100);
    m_parabolicItemNullifier.setSingleShot(true);
    connect(&m_parabolicItemNullifier, &QTimer::timeout, this, [&]() {
        setCurrentParabolicItem(nullptr);
    });

    connect(this, &Parabolic::currentParabolicItemChanged, this, &Parabolic::onCurrentParabolicItemChanged);

    connect(m_view, &View::eventTriggered, this, &Parabolic::onEvent);
}

Parabolic::~Parabolic()
{
}

QQuickItem *Parabolic::currentParabolicItem() const
{
    return m_currentParabolicItem;
}

void Parabolic::setCurrentParabolicItem(QQuickItem *item)
{
    if (m_currentParabolicItem == item) {
        return;
    }

    if (m_currentParabolicItem) {
        QMetaObject::invokeMethod(m_currentParabolicItem, "parabolicExited", Qt::QueuedConnection);
    }

    m_currentParabolicItem = item;
    emit currentParabolicItemChanged();
}

void Parabolic::onEvent(QEvent *e)
{
    if (!e) {
        return;
    }

    switch (e->type()) {

    case QEvent::Leave:
        setCurrentParabolicItem(nullptr);
        break;
    case QEvent::MouseMove:
        if (auto me = dynamic_cast<QMouseEvent *>(e)) {
            if (m_currentParabolicItem) {
                QPointF internal = m_currentParabolicItem->mapFromScene(me->windowPos());

                if (m_currentParabolicItem->contains(internal)) {
                    m_parabolicItemNullifier.stop();
                    //! sending move event to parabolic item
                    QMetaObject::invokeMethod(m_currentParabolicItem,
                                              "parabolicMove",
                                              Qt::QueuedConnection,
                                              Q_ARG(qreal, internal.x()),
                                              Q_ARG(qreal, internal.y()));
                } else {
                    m_lastOrphanParabolicMove = me->windowPos();
                    //! clearing parabolic item
                    m_parabolicItemNullifier.start();
                }
            } else {
                m_lastOrphanParabolicMove = me->windowPos();
            }
        }
    default:
        break;
    }

}

void Parabolic::onCurrentParabolicItemChanged()
{
    m_parabolicItemNullifier.stop();

    if (m_currentParabolicItem != nullptr) {
        QPointF internal = m_currentParabolicItem->mapFromScene(m_lastOrphanParabolicMove);

        if (m_currentParabolicItem->contains(internal)) {
            //! sending enter event to parabolic item
            QMetaObject::invokeMethod(m_currentParabolicItem,
                                      "parabolicEntered",
                                      Qt::QueuedConnection,
                                      Q_ARG(qreal, internal.x()),
                                      Q_ARG(qreal, internal.y()));
        }
    }
}

}
}

