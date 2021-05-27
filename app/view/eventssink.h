/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWEVENTSSINK_H
#define VIEWEVENTSSINK_H

// Qt
#include <QEvent>
#include <QObject>
#include <QList>
#include <QPointer>
#include <QQuickItem>

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {

//! This class is used in order to sunk events from children rects of originParentItem
//! into the destination Item. Each applet container from containment qml part is responsible
//! to initialize properly the originParentItem and the destinationItem to be used for
//! sunk events

class EventsSink: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *originParentItem READ originParentItem NOTIFY itemsChanged)
    Q_PROPERTY(QQuickItem *destinationItem READ destinationItem NOTIFY itemsChanged)

public:
    EventsSink(Latte::View *parent);
    virtual ~EventsSink();

    bool isActive();

    QQuickItem *originParentItem() const;
    QQuickItem *destinationItem() const;

public slots:
    Q_INVOKABLE void setSink(QQuickItem *originParent, QQuickItem *destination);

    QEvent *onEvent(QEvent *e);

signals:
    void itemsChanged();

private slots:
    void release();

private:
    QPointF positionAdjustedForDestination(const QPointF &point) const;

    bool originSinksContain(const QPointF &point) const;
    bool destinationContains(const QPointF &point) const;
private:
    QPointer<Latte::View> m_view;

    QPointer<QQuickItem> m_originParentItem;
    QPointer<QQuickItem> m_destinationItem;
};

}
}

#endif

