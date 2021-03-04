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
#include <QQmlPropertyMap>
#include <QQuickItem>

namespace Latte{
namespace Containment{

class LayoutManager : public QObject
{
  Q_OBJECT
    Q_PROPERTY(QObject *plasmoidObj READ plasmoid() WRITE setPlasmoid NOTIFY plasmoidChanged)

    Q_PROPERTY(QQuickItem *rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)
    Q_PROPERTY(QQuickItem *mainLayout READ mainLayout WRITE setMainLayout NOTIFY mainLayoutChanged)
    Q_PROPERTY(QQuickItem *startLayout READ startLayout WRITE setStartLayout NOTIFY startLayoutChanged)
    Q_PROPERTY(QQuickItem *endLayout READ endLayout WRITE setEndLayout NOTIFY endLayoutChanged)

    Q_PROPERTY(QQuickItem *dndSpacerItem READ dndSpacer WRITE setDndSpacer NOTIFY dndSpacerChanged)
    Q_PROPERTY(QQuickItem *metrics READ metrics WRITE setMetrics NOTIFY metricsChanged)

public:
    LayoutManager(QObject *parent = nullptr);

    QObject *plasmoid() const;
    void setPlasmoid(QObject *plasmoid);

    QQuickItem *rootItem() const;
    void setRootItem(QQuickItem *root);

    QQuickItem *mainLayout() const;
    void setMainLayout(QQuickItem *main);

    QQuickItem *startLayout() const;
    void setStartLayout(QQuickItem *start);

    QQuickItem *endLayout() const;
    void setEndLayout(QQuickItem *end);

    QQuickItem *dndSpacer() const;
    void setDndSpacer(QQuickItem *dnd);

    QQuickItem *metrics() const;
    void setMetrics(QQuickItem *metrics);

public slots:
    Q_INVOKABLE void moveAppletsInJustifyAlignment();
    Q_INVOKABLE void joinLayoutsToMainLayout();
    Q_INVOKABLE void insertBefore(QQuickItem *hoveredItem, QQuickItem *item);
    Q_INVOKABLE void insertAfter(QQuickItem *hoveredItem, QQuickItem *item);
    Q_INVOKABLE void insertAtCoordinates(QQuickItem *item, const int &x, const int &y);

signals:
    void plasmoidChanged();
    void rootItemChanged();
    void dndSpacerChanged();
    void mainLayoutChanged();
    void metricsChanged();
    void startLayoutChanged();
    void endLayoutChanged();

private:
    bool insertAtLayoutCoordinates(QQuickItem *layout, QQuickItem *item, int x, int y);

private:
    QQuickItem *m_rootItem{nullptr};
    QQuickItem *m_dndSpacer{nullptr};

    QQuickItem *m_mainLayout{nullptr};
    QQuickItem *m_startLayout{nullptr};
    QQuickItem *m_endLayout{nullptr};
    QQuickItem *m_metrics{nullptr};

    QObject *m_plasmoid{nullptr};
    QQmlPropertyMap *m_configuration{nullptr};
};
}
}
#endif // CONTAINMENTLAYOUTMANAGER_H
