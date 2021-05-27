/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    //! Parabolic Item Nullifier does not need any big interval in order to avoid
    //! nullifing currentParabolicItem too fast and as such send a false signal
    //! that NO parabolic item is hovered currently
    m_parabolicItemNullifier.setInterval(1);
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

    if (m_currentParabolicItem) {
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

