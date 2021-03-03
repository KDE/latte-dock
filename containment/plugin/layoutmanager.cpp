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

#include "layoutmanager.h"

//! Plasma
#include <Plasma>

const int CHILDFOUNDID = 11;

namespace Latte{
namespace Containment{

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent)
{
}

QObject *LayoutManager::plasmoid() const
{
    return m_plasmoid;
}

void LayoutManager::setPlasmoid(QObject *plasmoid)
{
    if (m_plasmoid == plasmoid) {
        return;
    }

    m_plasmoid = plasmoid;

    if (m_plasmoid) {
        m_configuration = qobject_cast<QQmlPropertyMap *>(m_plasmoid->property("configuration").value<QObject *>());
    }

    emit plasmoidChanged();
}

QQuickItem *LayoutManager::rootItem() const
{
    return m_rootItem;
}

void LayoutManager::setRootItem(QQuickItem *root)
{
    if (m_rootItem == root) {
        return;
    }

    m_rootItem = root;
    emit rootItemChanged();
}

QQuickItem *LayoutManager::mainLayout() const
{
    return m_mainLayout;
}

void LayoutManager::setMainLayout(QQuickItem *main)
{
    if (main == m_mainLayout) {
        return;
    }

    m_mainLayout = main;
    emit mainLayoutChanged();
}

QQuickItem *LayoutManager::startLayout() const
{
    return m_startLayout;
}

void LayoutManager::setStartLayout(QQuickItem *start)
{
    if (m_startLayout == start) {
        return;
    }

    m_startLayout = start;
    emit startLayoutChanged();
}

QQuickItem *LayoutManager::endLayout() const
{
    return m_endLayout;
}

void LayoutManager::setEndLayout(QQuickItem *end)
{
    if (m_endLayout == end) {
        return;
    }

    m_endLayout = end;
    emit endLayoutChanged();
}

QQuickItem *LayoutManager::metrics() const
{
    return m_metrics;
}

void LayoutManager::setMetrics(QQuickItem *metrics)
{
    if (m_metrics == metrics) {
        return;
    }

    m_metrics = metrics;
    emit metricsChanged();
}

//! Actions

void LayoutManager::insertBefore(QQuickItem *hoveredItem, QQuickItem *item)
{
    if (!hoveredItem || !item) {
        return;
    }

    item->setParentItem(hoveredItem->parentItem());
    item->stackBefore(hoveredItem);
}

void LayoutManager::insertAfter(QQuickItem *hoveredItem, QQuickItem *item)
{
    if (!hoveredItem || !item) {
        return;
    }

    item->setParentItem(hoveredItem->parentItem());
    item->stackAfter(hoveredItem);
}


bool LayoutManager::insertAtLayoutCoordinates(QQuickItem *layout, QQuickItem *item, int x, int y)
{
    if (!layout || !item || !m_plasmoid) {
        return false;
    }

    bool horizontal = (m_plasmoid->property("formFactor").toInt() != Plasma::Types::Vertical);
    bool vertical = !horizontal;
    int rowspacing = qMax(0, layout->property("rowSpacing").toInt());
    int columnspacing = qMax(0, layout->property("columnSpacing").toInt());

    if (horizontal) {
        y = layout->height() / 2;
    } else {
        x = layout->width() / 2;
    }

    //! child renamed at hovered
    QQuickItem *hovered = layout->childAt(x, y);

    //if we got a place inside the space between 2 applets, we have to find it manually
    if (!hovered) {
        int size = layout->childItems().count();
        if (horizontal) {
            for (int i = 0; i < size; ++i) {
                QQuickItem *candidate = layout->childItems()[i];
                int right = candidate->x() + candidate->width() + rowspacing;
                if (x>=candidate->x() && x<right) {
                    hovered = candidate;
                    break;
                }
            }
        } else {
            for (int i = 0; i < size; ++i) {
                QQuickItem *candidate = layout->childItems()[i];
                int bottom = candidate->y() + candidate->height() + columnspacing;
                if (y>=candidate->y() && y<bottom) {
                    hovered = candidate;
                    break;
                }
            }
        }

    }

    if (hovered == item && item->parentItem() == layout) {
        //! already hovered and in correct position
        return true;
    }

    if (!hovered) {
        QQuickItem *totals = m_metrics->property("totals").value<QQuickItem *>();
        float neededspace = 1.5 * (m_metrics->property("iconSize").toFloat() + totals->property("lengthEdge").toFloat());

        if ( ((vertical && ((y-neededspace) <= layout->height()) && (y>=0))
                 || (horizontal && ((x-neededspace) <= layout->width()) && (x>=0)))
             && layout->childItems().count()>0) {
            //! last item
            qDebug() << "org.kde.latte << last item ..";
            hovered = layout->childItems()[layout->childItems().count()-1];
        } else if ( ((vertical && (y >= -neededspace) && (y<=neededspace)))
                     || (horizontal && (x >= -neededspace) && (x<=neededspace))
                 && layout->childItems().count()>0) {
            //! first item
            qDebug() << "org.kde.latte << first item ..";
            hovered = layout->childItems()[0];
        } else {
            return false;
        }
    }

    item->setParentItem(layout);

    if ((vertical && y < (hovered->y() + hovered->height()/2)) ||
            (horizontal && x < hovered->x() + hovered->width()/2)) {
        item->stackBefore(hovered);
    } else {
        item->stackAfter(hovered);
    }

    return true;
}

void LayoutManager::insertAtCoordinates(QQuickItem *item, const int &x, const int &y)
{
    bool result{false};

    QPointF startPos = m_startLayout->mapFromItem(m_rootItem, QPointF(x, y));
    result = insertAtLayoutCoordinates(m_startLayout, item, startPos.x(), startPos.y());

    if (!result) {
        QPointF endPos = m_endLayout->mapFromItem(m_rootItem, QPointF(x, y));
        result = insertAtLayoutCoordinates(m_endLayout, item, endPos.x(), endPos.y());
    }

    if (!result) {
        QPointF mainPos = m_mainLayout->mapFromItem(m_rootItem, QPointF(x, y));
        //! in javascript direct insertAtCoordinates was usedd ???
        result = insertAtLayoutCoordinates(m_mainLayout, item, mainPos.x(), mainPos.y());
    }
}

void LayoutManager::moveAppletsInJustifyAlignment()
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }

    QList<QQuickItem *> appletlist;

    appletlist << m_startLayout->childItems();
    appletlist << m_mainLayout->childItems();
    appletlist << m_endLayout->childItems();

    bool firstSplitterFound{false};
    bool secondSplitterFound{false};
    int splitter1{-1};
    int splitter2{-1};

    for(int i=0; i<appletlist.count(); ++i) {
        bool issplitter = appletlist[i]->property("isInternalViewSplitter").toBool();

        if (!firstSplitterFound) {
            appletlist[i]->setParentItem(m_startLayout);
            if (issplitter) {
                firstSplitterFound = true;
                splitter1 = i;
            }
        } else if (firstSplitterFound && !secondSplitterFound) {
            if (issplitter) {
                secondSplitterFound = true;
                splitter2 = i;
                appletlist[i]->setParentItem(m_endLayout);
            } else {
                appletlist[i]->setParentItem(m_mainLayout);
            }
        } else if (firstSplitterFound && secondSplitterFound) {
            appletlist[i]->setParentItem(m_endLayout);
        }
    }

    for(int i=0; i<appletlist.count()-1; ++i) {
        QQuickItem *before = appletlist[i];
        QQuickItem *after = appletlist[i+1];

        if (before->parentItem() == after->parentItem()) {
            before->stackBefore(after);
        }
    }

    //! Confirm Last item of End Layout
    if (m_endLayout->childItems().count() > 0) {
        QQuickItem *lastItem = m_endLayout->childItems()[m_endLayout->childItems().count()-1];

        int correctpos{-1};

        for(int i=0; i<appletlist.count()-1; ++i) {
            if (lastItem == appletlist[i]) {
                correctpos = i;
                break;
            }
        }

        if (correctpos>=0) {
            lastItem->stackBefore(appletlist[correctpos+1]);
        }
    }
}

}
}
