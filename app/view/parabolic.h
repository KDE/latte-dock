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

#ifndef VIEWPARABOLIC_H
#define VIEWPARABOLIC_H

// Qt
#include <QEvent>
#include <QObject>
#include <QQuickItem>
#include <QPointer>
#include <QPointF>
#include <QTimer>

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {

class Parabolic: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *currentItem READ currentParabolicItem WRITE setCurrentParabolicItem NOTIFY currentParabolicItemChanged)

public:
    Parabolic(Latte::View *parent);
    virtual ~Parabolic();

    QQuickItem *currentParabolicItem() const;
    void setCurrentParabolicItem(QQuickItem *item);

signals:
    void currentParabolicItemChanged();

private slots:
    void onCurrentParabolicItemChanged();
    void onEvent(QEvent *e);

private:
    QPointer<Latte::View> m_view;

    QPointF m_lastOrphanParabolicMove;
    QQuickItem *m_currentParabolicItem{nullptr};

    QTimer m_parabolicItemNullifier;
};

}
}

#endif
