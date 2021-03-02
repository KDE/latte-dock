/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
*
*
*  This file is part of Latte-Dock and is a Fork of PlasmaCore::IconItem
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

#ifndef CONTAINMENTLAYOUTMANAGER_H
#define CONTAINMENTLAYOUTMANAGER_H

//Qt
#include <QObject>
#include <QQuickItem>

namespace Latte{
namespace Containment{

class LayoutManager : public QObject
{
  Q_OBJECT
    Q_PROPERTY(QQuickItem *mainLayout READ mainLayout WRITE setMainLayout NOTIFY mainLayoutChanged)
    Q_PROPERTY(QQuickItem *startLayout READ startLayout WRITE setStartLayout NOTIFY startLayoutChanged)
    Q_PROPERTY(QQuickItem *endLayout READ endLayout WRITE setEndLayout NOTIFY endLayoutChanged)

public:
    LayoutManager(QObject *parent = nullptr);

    QQuickItem *mainLayout() const;
    void setMainLayout(QQuickItem *main);

    QQuickItem *startLayout() const;
    void setStartLayout(QQuickItem *start);

    QQuickItem *endLayout() const;
    void setEndLayout(QQuickItem *end);

public slots:
    Q_INVOKABLE void moveAppletsInJustifyAlignment();

signals:
    void mainLayoutChanged();
    void startLayoutChanged();
    void endLayoutChanged();

private:
    QQuickItem *m_mainLayout{nullptr};
    QQuickItem *m_startLayout{nullptr};
    QQuickItem *m_endLayout{nullptr};

};
}
}
#endif // CONTAINMENTLAYOUTMANAGER_H
