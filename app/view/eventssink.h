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

#ifndef VIEWEVENTSSINK_H
#define VIEWEVENTSSINK_H

// Qt
#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {

class EventsSink: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *originItem READ originItem NOTIFY itemsChanged)
    Q_PROPERTY(QQuickItem *destinationItem READ destinationItem NOTIFY itemsChanged)

public:
    EventsSink(Latte::View *parent);
    virtual ~EventsSink();

    bool isActive();

    QQuickItem *originItem() const;
    QQuickItem *destinationItem() const;

public slots:
    Q_INVOKABLE void setSink(QQuickItem *origin, QQuickItem *destination);

    QEvent *onEvent(QEvent *e);

signals:
    void itemsChanged();

private slots:
    void release();

private:
    QPointF positionAdjustedForDestination(const QPointF &point) const;

private:
    QPointer<Latte::View> m_view;

    QPointer<QQuickItem> m_originItem;
    QPointer<QQuickItem> m_destinationItem;
};

}
}

#endif

