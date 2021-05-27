/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    QPointer<QQuickItem> m_currentParabolicItem;

    QPointF m_lastOrphanParabolicMove;

    QTimer m_parabolicItemNullifier;
};

}
}

#endif
