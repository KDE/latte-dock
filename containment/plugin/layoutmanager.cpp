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

// local
#include <plugin/lattetypes.h>

// KDE
#include <KDeclarative/ConfigPropertyMap>

// Plasma
#include <Plasma>



const int CHILDFOUNDID = 11;

namespace Latte{
namespace Containment{

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent)
{
    connect(this, &LayoutManager::rootItemChanged, this, &LayoutManager::onRootItemChanged);
}

int LayoutManager::splitterPosition() const
{
    return m_splitterPosition;
}

void LayoutManager::setSplitterPosition(const int &position)
{
    if (m_splitterPosition == position) {
        return;
    }

    m_splitterPosition = position;
    emit splitterPositionChanged();
}

int LayoutManager::splitterPosition2() const
{
    return m_splitterPosition2;
}

void LayoutManager::setSplitterPosition2(const int &position)
{
    if (m_splitterPosition2 == position) {
        return;
    }

    m_splitterPosition2 = position;
    emit splitterPosition2Changed();
}

QString LayoutManager::appletOrder() const
{
    return m_appletOrder;
}

void LayoutManager::setAppletOrder(const QString &order)
{
    if (m_appletOrder == order) {
        return;
    }

    m_appletOrder = order;
    emit appletOrderChanged();
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
        m_configuration = qobject_cast<KDeclarative::ConfigPropertyMap *>(m_plasmoid->property("configuration").value<QObject *>());
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

QQuickItem *LayoutManager::dndSpacer() const
{
    return m_dndSpacer;
}

void LayoutManager::setDndSpacer(QQuickItem *dnd)
{
    if (m_dndSpacer == dnd) {
        return;
    }

    m_dndSpacer = dnd;
    emit dndSpacerChanged();
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

void LayoutManager::onRootItemChanged()
{
    if (!m_rootItem) {
        return;
    }

    auto rootMetaObject = m_rootItem->metaObject();
    int createAppletItemIndex = rootMetaObject->indexOfMethod("createAppletItem(QVariant)");
    m_createAppletItemMethod = rootMetaObject->method(createAppletItemIndex);

    int createJustifySplitterIndex = rootMetaObject->indexOfMethod("createJustifySplitter()");
    m_createJustifySplitterMethod = rootMetaObject->method(createJustifySplitterIndex);

    qDebug() << "org.kde.latte :::: root item set methods....";
}

//! Actions
void LayoutManager::restore()
{
    QStringList appletStringIdsOrder = (*m_configuration)["appletOrder"].toString().split(";");
    QList<QObject *> applets = m_plasmoid->property("applets").value<QList<QObject *>>();

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());
    int splitterPosition = static_cast<Latte::Types::Alignment>((*m_configuration)["splitterPosition"].toInt());
    int splitterPosition2 = static_cast<Latte::Types::Alignment>((*m_configuration)["splitterPosition2"].toInt());

    QList<int> appletIdsOrder;
    for (int i=0; i<appletStringIdsOrder.count(); ++i) {
        appletIdsOrder << appletStringIdsOrder[i].toInt();
    }

    if (alignment==Latte::Types::Justify && splitterPosition!=-1 && splitterPosition2!=-1) {
        appletIdsOrder.insert(splitterPosition-1, -1);
        appletIdsOrder.insert(splitterPosition2-1, -1);
    }

    QList<QObject *> orderedApplets;

    for (int i=0; i<appletIdsOrder.count(); ++i) {
        if (appletIdsOrder[i] == -1) {
            orderedApplets << nullptr;
            continue;
        }

        for(int j=0; j<applets.count(); ++j) {
            if (applets[j]->property("id").toUInt() == appletIdsOrder[i]) {
                orderedApplets << applets[j];
                break;
            }
        }
    }

    QStringList orphanedIds;
    for(int i=0; i<applets.count(); ++i) {
        uint id = applets[i]->property("id").toUInt();
        if (!appletIdsOrder.contains(id)) {
            orphanedIds << QString::number(id);
        }
    }

    //Validator
    QList<int> validateAppletsOrder;
    for (int i=0; i<orderedApplets.count(); ++i) {
        if (orderedApplets[i] == nullptr) {
            validateAppletsOrder << -1;
            continue;
        }

        validateAppletsOrder << orderedApplets[i]->property("id").toUInt();
    }

    qDebug() << "org.kde.latte ::: applets found :: " << applets.count() << " : " << appletIdsOrder << " :: " << splitterPosition << " : " << splitterPosition2 << " | " << alignment;
    qDebug() << "org.kde.latte ::: applets orphaned :: " << orphanedIds.join(";");
    qDebug() << "org.kde.latte ::: applets recorded order :: " << appletIdsOrder;
    qDebug() << "org.kde.latte ::: applets produced order ?? " << validateAppletsOrder;

    if (alignment != Latte::Types::Justify) {
        for (int i=0; i<orderedApplets.count(); ++i) {
            if (orderedApplets[i] == nullptr) {
                continue;
            }

            QVariant appletItemVariant;
            QVariant appletVariant; appletVariant.setValue(orderedApplets[i]);
            m_createAppletItemMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, appletItemVariant), Q_ARG(QVariant, appletVariant));
            QQuickItem *appletItem = appletItemVariant.value<QQuickItem *>();
            appletItem->setParentItem(m_mainLayout);
        }
    } else {
        QQuickItem *parentlayout = m_startLayout;

        for (int i=0; i<orderedApplets.count(); ++i) {
            if (orderedApplets[i] == nullptr) {
                QVariant splitterItemVariant;
                m_createJustifySplitterMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, splitterItemVariant));
                QQuickItem *splitterItem = splitterItemVariant.value<QQuickItem *>();

                if (parentlayout == m_startLayout) {
                    //! first splitter as last child in startlayout
                    splitterItem->setParentItem(parentlayout);
                    parentlayout = m_mainLayout;
                } else if (parentlayout == m_mainLayout) {
                    //! second splitter as first child in endlayout
                    parentlayout = m_endLayout;
                    splitterItem->setParentItem(parentlayout);
                }

                continue;
            }

            QVariant appletItemVariant;
            QVariant appletVariant; appletVariant.setValue(orderedApplets[i]);
            m_createAppletItemMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, appletItemVariant), Q_ARG(QVariant, appletVariant));
            QQuickItem *appletItem = appletItemVariant.value<QQuickItem *>();
            appletItem->setParentItem(parentlayout);
        }
    }

    save();
}

void LayoutManager::save()
{
    QStringList appletIds;

    int startChilds{0};
    for(int i=0; i<m_startLayout->childItems().count(); ++i) {
        QQuickItem *item = m_startLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (!isInternalSplitter) {
            QVariant appletVariant = item->property("applet");
            if (!appletVariant.isValid()) {
                continue;
            }

            QObject *applet = appletVariant.value<QObject *>();
            uint id = applet->property("id").toUInt();

            if (id>0) {
                startChilds++;
                appletIds << QString::number(id);
            }
        }
    }

    int mainChilds{0};
    for(int i=0; i<m_mainLayout->childItems().count(); ++i) {
        QQuickItem *item = m_mainLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (!isInternalSplitter) {
            QVariant appletVariant = item->property("applet");
            if (!appletVariant.isValid()) {
                continue;
            }

            QObject *applet = appletVariant.value<QObject *>();
            uint id = applet->property("id").toUInt();

            if (id>0) {
                mainChilds++;
                appletIds << QString::number(id);
            }
        }
    }

    int endChilds{0};
    for(int i=0; i<m_endLayout->childItems().count(); ++i) {
        QQuickItem *item = m_endLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (!isInternalSplitter) {
            QVariant appletVariant = item->property("applet");
            if (!appletVariant.isValid()) {
                continue;
            }

            QObject *applet = appletVariant.value<QObject *>();
            uint id = applet->property("id").toUInt();

            if (id>0) {
                endChilds++;
                appletIds << QString::number(id);
            }
        }
    }

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());

    if (alignment == Latte::Types::Justify) {
        setSplitterPosition(startChilds + 1);
        setSplitterPosition2(startChilds + 1 + mainChilds + 1);
    }

    //! are not writing in config file for some cases mentioned in class header so they are not used
    //(*m_configuration)["splitterPosition"] = QVariant(startChilds + 1);
    //(*m_configuration)["splitterPosition2"] = QVariant(startChilds + 1 + mainChilds + 1);
    //(*m_configuration)["appletOrder"] = appletIds.join(";");

    setAppletOrder(appletIds.join(";"));
}

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
            hovered = layout->childItems()[layout->childItems().count()-1];
        } else if ( ((vertical && (y >= -neededspace) && (y<=neededspace)))
                    || (horizontal && (x >= -neededspace) && (x<=neededspace))
                    && layout->childItems().count()>0) {
            //! first item
            hovered = layout->childItems()[0];
        } else {
            return false;
        }
    }

    if ((vertical && y < (hovered->y() + hovered->height()/2)) ||
            (horizontal && x < hovered->x() + hovered->width()/2)) {
        insertBefore(hovered, item);
    } else {
        insertAfter(hovered, item);
    }

    return true;
}

void LayoutManager::insertAtCoordinates(QQuickItem *item, const int &x, const int &y)
{
    if (!m_configuration) {
        return;
    }

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());

    bool result{false};

    if (alignment == Latte::Types::Justify) {
        QPointF startPos = m_startLayout->mapFromItem(m_rootItem, QPointF(x, y));
        result = insertAtLayoutCoordinates(m_startLayout, item, startPos.x(), startPos.y());

        if (!result) {
            QPointF endPos = m_endLayout->mapFromItem(m_rootItem, QPointF(x, y));
            result = insertAtLayoutCoordinates(m_endLayout, item, endPos.x(), endPos.y());
        }
    }

    if (!result) {
        QPointF mainPos = m_mainLayout->mapFromItem(m_rootItem, QPointF(x, y));
        //! in javascript direct insertAtCoordinates was usedd ???
        result = insertAtLayoutCoordinates(m_mainLayout, item, mainPos.x(), mainPos.y());
    }
}

void LayoutManager::joinLayoutsToMainLayout()
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }

    if (m_startLayout->childItems().count() > 0) {
        int size = m_startLayout->childItems().count();
        for (int i=size-1; i>=0; --i) {
            QQuickItem *lastStartLayoutItem = m_startLayout->childItems()[i];
            QQuickItem *firstMainLayoutItem = m_mainLayout->childItems().count() > 0 ? m_mainLayout->childItems()[0] : nullptr;

            lastStartLayoutItem->setParentItem(m_mainLayout);

            if (firstMainLayoutItem) {
                lastStartLayoutItem->stackBefore(firstMainLayoutItem);
            }
        }
    }

    if (m_endLayout->childItems().count() > 0) {
        int size = m_endLayout->childItems().count();
        for (int i=0; i<size; ++i) {
            QQuickItem *firstEndLayoutItem = m_endLayout->childItems()[0];
            QQuickItem *lastMainLayoutItem = m_mainLayout->childItems().count() > 0 ? m_mainLayout->childItems()[m_mainLayout->childItems().count()-1] : nullptr;

            firstEndLayoutItem->setParentItem(m_mainLayout);

            if (lastMainLayoutItem) {
                firstEndLayoutItem->stackAfter(lastMainLayoutItem);
            }
        }
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
