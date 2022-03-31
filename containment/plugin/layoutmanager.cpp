/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutmanager.h"

// local
#include <plugin/lattetypes.h>

// Qt
#include <QtMath>

// KDE
#include <KDeclarative/ConfigPropertyMap>

// Plasma
#include <Plasma>
#include <Plasma/Applet>
#include <PlasmaQuick/AppletQuickItem>

#define ISAPPLETLOCKEDOPTION "lockZoom"
#define ISCOLORINGBLOCKEDOPTION "userBlocksColorizing"

namespace Latte{
namespace Containment{

const int LayoutManager::JUSTIFYSPLITTERID;

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent)
{
    m_option[ISAPPLETLOCKEDOPTION] = "lockedZoomApplets";
    m_option[ISCOLORINGBLOCKEDOPTION] = "userBlocksColorizingApplets";

    connect(this, &LayoutManager::rootItemChanged, this, &LayoutManager::onRootItemChanged);

    m_hasRestoredAppletsTimer.setInterval(2000);
    m_hasRestoredAppletsTimer.setSingleShot(true);
    connect(&m_hasRestoredAppletsTimer, &QTimer::timeout, this, [&]() {
        m_hasRestoredApplets = true;
        emit hasRestoredAppletsChanged();
    });
}

bool LayoutManager::hasRestoredApplets() const
{
    return m_hasRestoredApplets;
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

QList<int> LayoutManager::appletOrder() const
{
    return m_appletOrder;
}

void LayoutManager::setAppletOrder(const QList<int> &order)
{
    if (m_appletOrder == order) {
        return;
    }

    m_appletOrder = order;
    emit appletOrderChanged();
}

QList<int> LayoutManager::lockedZoomApplets() const
{
    return m_lockedZoomApplets;
}

void LayoutManager::setLockedZoomApplets(const QList<int> &applets)
{
    if (m_lockedZoomApplets == applets) {
        return;
    }

    m_lockedZoomApplets = applets;
    emit lockedZoomAppletsChanged();
}

QList<int> LayoutManager::order() const
{
    return m_order;
}

void LayoutManager::setOrder(const QList<int> &order)
{
    if (m_order == order) {
        return;
    }

    m_order = order;
    emit orderChanged();
}

QList<int> LayoutManager::userBlocksColorizingApplets() const
{
    return m_userBlocksColorizingApplets;
}

void LayoutManager::setUserBlocksColorizingApplets(const QList<int> &applets)
{
    if (m_userBlocksColorizingApplets == applets) {
        return;
    }

    m_userBlocksColorizingApplets = applets;
    emit userBlocksColorizingAppletsChanged();
}

QList<int> LayoutManager::appletsInScheduledDestruction() const
{
    return m_appletsInScheduledDestruction.keys();
}

void LayoutManager::setAppletInScheduledDestruction(const int &id, const bool &enabled)
{
    if (m_appletsInScheduledDestruction.contains(id) && !enabled) {
        m_appletsInScheduledDestruction.remove(id);
        emit appletsInScheduledDestructionChanged();
    } else if (!m_appletsInScheduledDestruction.contains(id) && enabled) {
        m_appletsInScheduledDestruction[id] = appletItem(id);
        emit appletsInScheduledDestructionChanged();
    }
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

void LayoutManager::updateOrder()
{
    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());

    auto nextorder = m_appletOrder;

    if (alignment==Latte::Types::Justify) {
        nextorder.insert(m_splitterPosition-1, JUSTIFYSPLITTERID);
        nextorder.insert(m_splitterPosition2-1, JUSTIFYSPLITTERID);
    }

    setOrder(nextorder);
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

    int initAppletContainerIndex = rootMetaObject->indexOfMethod("initAppletContainer(QVariant,QVariant)");
    m_initAppletContainerMethod = rootMetaObject->method(initAppletContainerIndex);
}

bool LayoutManager::isValidApplet(const int &id)
{
    //! should be loaded after m_plasmoid has been set properly
    if (!m_plasmoid) {
        return false;
    }

    QList<QObject *> applets = m_plasmoid->property("applets").value<QList<QObject *>>();

    for(int i=0; i<applets.count(); ++i) {
        uint appletid = applets[i]->property("id").toUInt();
        if (id>0 && appletid == (uint)id) {
            return true;
        }
    }

    return false;
}

//! Actions
void LayoutManager::restore()
{
    QList<int> appletIdsOrder = toIntList((*m_configuration)["appletOrder"].toString());
    QList<QObject *> applets = m_plasmoid->property("applets").value<QList<QObject *>>();

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());
    int splitterPosition = (*m_configuration)["splitterPosition"].toInt();
    int splitterPosition2 = (*m_configuration)["splitterPosition2"].toInt();

    if (alignment==Latte::Types::Justify) {
        if (splitterPosition!=-1 && splitterPosition2!=-1) {
            appletIdsOrder.insert(splitterPosition-1, -1);
            appletIdsOrder.insert(splitterPosition2-1, -1);
        } else {
            appletIdsOrder.insert(0, -1);
            appletIdsOrder << -1;
        }
    }

    QList<int> invalidApplets;

    //! track invalid applets, meaning applets that have not be loaded properly
    for (int i=0; i<appletIdsOrder.count(); ++i) {
        int aid = appletIdsOrder[i];

        if (aid>0 && !isValidApplet(aid)) {
            invalidApplets << aid;
        }
    }

    //! remove invalid applets from the ids order
    for (int i=0; i<invalidApplets.count(); ++i) {
        appletIdsOrder.removeAll(invalidApplets[i]);
    }

    //! order valid applets based on the cleaned applet ids order
    QList<QObject *> orderedApplets;

    for (int i=0; i<appletIdsOrder.count(); ++i) {
        if (appletIdsOrder[i] == -1) {
            orderedApplets << nullptr;
            continue;
        }

        for(int j=0; j<applets.count(); ++j) {
            if (applets[j]->property("id").toInt() == appletIdsOrder[i]) {
                orderedApplets << applets[j];
                break;
            }
        }
    }

    QList<int> orphanedIds;
    for(int i=0; i<applets.count(); ++i) {
        uint id = applets[i]->property("id").toUInt();

        if (!appletIdsOrder.contains(id)) {
            orphanedIds << id;
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

    for(int i=0; i<applets.count(); ++i) {
        if (!orderedApplets.contains(applets[i])) {
            //! after order has been loaded correctly all renaming applets that do not have specified position are added in the end
            orderedApplets<<applets[i];
        }
    }

    qDebug() << "org.kde.latte ::: applets found :: " << applets.count() << " : " << appletIdsOrder << " :: " << splitterPosition << " : " << splitterPosition2 << " | " << alignment;
    qDebug() << "org.kde.latte ::: applets orphaned added in the end:: " << orphanedIds;
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

    restoreOptions();
    save(); //will restore also a valid applets ids order
    cleanupOptions();
    initSaveConnections();
    m_hasRestoredAppletsTimer.start();
}

void LayoutManager::initSaveConnections()
{
    connect(this, &LayoutManager::appletOrderChanged, this, &LayoutManager::cleanupOptions);
    connect(this, &LayoutManager::splitterPositionChanged, this, &LayoutManager::saveOptions);
    connect(this, &LayoutManager::splitterPosition2Changed, this, &LayoutManager::saveOptions);
    connect(this, &LayoutManager::lockedZoomAppletsChanged, this, &LayoutManager::saveOptions);
    connect(this, &LayoutManager::userBlocksColorizingAppletsChanged, this, &LayoutManager::saveOptions);
}

void LayoutManager::restoreOptions()
{
    restoreOption(ISAPPLETLOCKEDOPTION);
    restoreOption(ISCOLORINGBLOCKEDOPTION);
}

void LayoutManager::restoreOption(const char *option)
{
    QList<int> applets = toIntList((*m_configuration)[m_option[option]].toString());

    if (option == ISAPPLETLOCKEDOPTION) {
        setLockedZoomApplets(applets);
    } else if (option == ISCOLORINGBLOCKEDOPTION) {
        setUserBlocksColorizingApplets(applets);
    }
}

bool LayoutManager::isJustifySplitter(const QQuickItem *item) const
{
    return item && (item->property("isInternalViewSplitter").toBool() == true);
}

bool LayoutManager::isMasqueradedIndex(const int &x, const int &y)
{
    return (x==y && x<=MASQUERADEDINDEXTOPOINTBASE && y<=MASQUERADEDINDEXTOPOINTBASE);
}

int LayoutManager::masquearadedIndex(const int &x, const int &y)
{
    return qAbs(x - MASQUERADEDINDEXTOPOINTBASE);
}

QPoint LayoutManager::indexToMasquearadedPoint(const int &index)
{
    return QPoint(MASQUERADEDINDEXTOPOINTBASE-index, MASQUERADEDINDEXTOPOINTBASE-index);
}

void LayoutManager::reorderParabolicSpacers()
{
    QQuickItem *startParabolicSpacer = m_mainLayout->property("startParabolicSpacer").value<QQuickItem *>();
    QQuickItem *endParabolicSpacer = m_mainLayout->property("endParabolicSpacer").value<QQuickItem *>();

    if (!startParabolicSpacer || !endParabolicSpacer) {
        return;
    }

    insertAtLayoutTail(m_mainLayout, startParabolicSpacer);
    insertAtLayoutHead(m_mainLayout, endParabolicSpacer);
}

void LayoutManager::save()
{
    QList<int> appletIds;

    reorderParabolicSpacers();

    auto collectLayoutAppletIds = [](QQuickItem *layout, QList<int> &appletIds) {
        int childCount = 0;
        for (int i=0; i<layout->childItems().count(); ++i) {
            QQuickItem *item = layout->childItems()[i];
            bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
            bool isParabolicEdgeSpacer = item->property("isParabolicEdgeSpacer").toBool();
            if (!isInternalSplitter && !isParabolicEdgeSpacer) {
                QVariant appletVariant = item->property("applet");
                if (!appletVariant.isValid()) {
                    continue;
                }

                QObject *applet = appletVariant.value<QObject *>();

                if (!applet) {
                    continue;
                }

                uint id = applet->property("id").toUInt();

                if (id>0) {
                    childCount++;
                    appletIds << id;
                }
            }
        }
        return childCount;
    };

    int startChilds = collectLayoutAppletIds(m_startLayout, appletIds);
    int mainChilds  = collectLayoutAppletIds(m_mainLayout,  appletIds);
    int endChilds   = collectLayoutAppletIds(m_endLayout,   appletIds);

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());

    if (alignment == Latte::Types::Justify) {
        setSplitterPosition(startChilds + 1);
        setSplitterPosition2(startChilds + 1 + mainChilds + 1);
    } else {
        int splitterPosition = (*m_configuration)["splitterPosition"].toInt();
        int splitterPosition2 = (*m_configuration)["splitterPosition2"].toInt();

        setSplitterPosition(splitterPosition);
        setSplitterPosition2(splitterPosition2);
    }

    //! are not writing in config file for some cases mentioned in class header so they are not used
    //(*m_configuration)["splitterPosition"] = QVariant(startChilds + 1);
    //(*m_configuration)["splitterPosition2"] = QVariant(startChilds + 1 + mainChilds + 1);
    //(*m_configuration)["appletOrder"] = appletIds.join(";");

    setAppletOrder(appletIds);

    //! publish updated order
    updateOrder();

    //! save applet order
    QString appletsserialized = toStr(appletIds);

    if ((*m_configuration)["appletOrder"] != appletsserialized) {
        m_configuration->insert("appletOrder", appletsserialized);
        emit m_configuration->valueChanged("appletOrder", appletsserialized);
    }
}

void LayoutManager::saveOptions()
{    
    QString lockedserialized =  toStr(m_lockedZoomApplets);
    if ((*m_configuration)[m_option[ISAPPLETLOCKEDOPTION]] != lockedserialized) {
        m_configuration->insert(m_option[ISAPPLETLOCKEDOPTION], lockedserialized);
        emit m_configuration->valueChanged(m_option[ISAPPLETLOCKEDOPTION], lockedserialized);
    }

    QString colorsserialized = toStr(m_userBlocksColorizingApplets);
    if ((*m_configuration)[m_option[ISCOLORINGBLOCKEDOPTION]] != colorsserialized) {
        m_configuration->insert(m_option[ISCOLORINGBLOCKEDOPTION], colorsserialized);
        emit m_configuration->valueChanged(m_option[ISCOLORINGBLOCKEDOPTION], colorsserialized);
    }

    if ((*m_configuration)["splitterPosition"] != m_splitterPosition) {
        m_configuration->insert("splitterPosition", m_splitterPosition);
        emit m_configuration->valueChanged(m_option["splitterPosition"], m_splitterPosition);
    }

    if ((*m_configuration)["splitterPosition2"] != m_splitterPosition2) {
        m_configuration->insert("splitterPosition2", m_splitterPosition2);
        emit m_configuration->valueChanged(m_option["splitterPosition2"], m_splitterPosition2);
    }
}

void LayoutManager::setOption(const int &appletId, const QString &property, const QVariant &value)
{
    if (property == ISAPPLETLOCKEDOPTION) {
        bool enabled = value.toBool();

        if (enabled && !m_lockedZoomApplets.contains(appletId)) {
            QList<int> applets = m_lockedZoomApplets; applets << appletId;
            setLockedZoomApplets(applets);
        } else if (!enabled && m_lockedZoomApplets.contains(appletId)) {
            QList<int> applets = m_lockedZoomApplets; applets.removeAll(appletId);
            setLockedZoomApplets(applets);
        }
    } else if (property == ISCOLORINGBLOCKEDOPTION) {
        bool enabled = value.toBool();

        if (enabled && !m_userBlocksColorizingApplets.contains(appletId)) {
            QList<int> applets = m_userBlocksColorizingApplets; applets << appletId;
            setUserBlocksColorizingApplets(applets);
        } else if (!enabled && m_userBlocksColorizingApplets.contains(appletId)) {
            QList<int> applets = m_userBlocksColorizingApplets; applets.removeAll(appletId);
            setUserBlocksColorizingApplets(applets);
        }
    }
}

void LayoutManager::insertBefore(QQuickItem *hoveredItem, QQuickItem *item)
{
    if (!hoveredItem || !item || hoveredItem == item) {
        return;
    }

    item->setParentItem(hoveredItem->parentItem());
    item->stackBefore(hoveredItem);
}

void LayoutManager::insertAfter(QQuickItem *hoveredItem, QQuickItem *item)
{
    if (!hoveredItem || !item || hoveredItem == item) {
        return;
    }

    item->setParentItem(hoveredItem->parentItem());
    item->stackAfter(hoveredItem);
}

void LayoutManager::insertAtLayoutTail(QQuickItem *layout, QQuickItem *item)
{
    if (!layout || !item) {
        return;
    }

    if (layout->childItems().count() > 0) {
        if (layout == m_endLayout && isJustifySplitter(layout->childItems()[0])) {
            //! this way we ignore the justify splitter in start layout
            insertAfter(layout->childItems()[0], item);
        } else {
            insertBefore(layout->childItems()[0], item);
        }
        return;
    }

    item->setParentItem(layout);
}

void LayoutManager::insertAtLayoutHead(QQuickItem *layout, QQuickItem *item)
{
    if (!layout || !item) {
        return;
    }

    int count = layout->childItems().count();

    if (count > 0) {
        if (layout == m_startLayout && isJustifySplitter(layout->childItems()[count-1])) {
            //! this way we ignore the justify splitter in end layout
            insertBefore(layout->childItems()[count-1], item);
        } else {
            insertAfter(layout->childItems()[count-1], item);
        }
        return;
    }

    item->setParentItem(layout);
}

void LayoutManager::insertAtLayoutIndex(QQuickItem *layout, QQuickItem *item, const int &index)
{
    if (!layout || !item) {
        return;
    }

    if (index == 0) {
        insertAtLayoutTail(layout, item);
    } else if (index >= layout->childItems().count()) {
        insertAtLayoutHead(layout, item);
    } else {
        insertBefore(layout->childItems()[index], item);
    }
}

bool LayoutManager::insertAtLayoutCoordinates(QQuickItem *layout, QQuickItem *item, int x, int y)
{
    if (!layout || !item || !m_plasmoid || !layout->contains(QPointF(x,y))) {
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
                if (i>=layout->childItems().count()) {
                    break;
                }

                QQuickItem *candidate = layout->childItems()[i];
                int right = candidate->x() + candidate->width() + rowspacing;
                if (x>=candidate->x() && x<right) {
                    hovered = candidate;
                    break;
                }
            }
        } else {
            for (int i = 0; i < size; ++i) {
                if (i>=layout->childItems().count()) {
                    break;
                }

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

    if (hovered) {
        if ((vertical && y < (hovered->y() + hovered->height()/2) && hovered->height() > 1) ||
                (horizontal && x < (hovered->x() + hovered->width()/2) && hovered->width() > 1)) {
            insertBefore(hovered, item);
        } else {
            insertAfter(hovered, item);
        }

        return true;
    }

    return false;
}

QQuickItem *LayoutManager::firstSplitter()
{
    for(int i=0; i<m_startLayout->childItems().count(); ++i) {
        QQuickItem *item = m_startLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (isInternalSplitter) {
            return item;
        }
    }

    for(int i=0; i<m_mainLayout->childItems().count(); ++i) {
        QQuickItem *item = m_mainLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (isInternalSplitter) {
            return item;
        }
    }

    for(int i=0; i<m_endLayout->childItems().count(); ++i) {
        QQuickItem *item = m_endLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (isInternalSplitter) {
            return item;
        }
    }

    return nullptr;
}

QQuickItem *LayoutManager::lastSplitter()
{
    for(int i=m_endLayout->childItems().count()-1; i>=0; --i) {
        QQuickItem *item = m_endLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (isInternalSplitter) {
            return item;
        }
    }

    for(int i=m_mainLayout->childItems().count()-1; i>=0; --i) {
        QQuickItem *item = m_mainLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (isInternalSplitter) {
            return item;
        }
    }

    for(int i=m_endLayout->childItems().count()-1; i>=0; --i) {
        QQuickItem *item = m_endLayout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        if (isInternalSplitter) {
            return item;
        }
    }

    return nullptr;
}

QQuickItem *LayoutManager::appletItemInLayout(QQuickItem *layout, const int &id)
{
    if (!layout) {
        return nullptr;
    }

    for(int i=0; i<layout->childItems().count(); ++i) {
        QQuickItem *item = layout->childItems()[i];
        bool isInternalSplitter = item->property("isInternalViewSplitter").toBool();
        bool isParabolicEdgeSpacer = item->property("isParabolicEdgeSpacer").toBool();
        if (!isInternalSplitter && !isParabolicEdgeSpacer) {
            QVariant appletVariant = item->property("applet");
            if (!appletVariant.isValid()) {
                continue;
            }

            QObject *applet = appletVariant.value<QObject *>();

            if (!applet) {
                continue;
            }

            int tempid = applet->property("id").toInt();

            if (id == tempid) {
                return item;
            }
        }
    }

    return nullptr;
}

QQuickItem *LayoutManager::appletItem(const int &id)
{
    QQuickItem *item = appletItemInLayout(m_mainLayout, id);

    if (!item) {
        item = appletItemInLayout(m_startLayout, id);
    }

    if (!item) {
        item = appletItemInLayout(m_endLayout, id);
    }

    return item;
}

int LayoutManager::dndSpacerIndex()
{
    if (m_dndSpacer->parentItem() != m_startLayout
            && m_dndSpacer->parentItem() != m_mainLayout
            && m_dndSpacer->parentItem() != m_endLayout) {
        return -1;
    }

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());
    int index = -1;

    if (alignment == Latte::Types::Justify) {
        for(int i=0; i<m_startLayout->childItems().count(); ++i) {
            QQuickItem *item = m_startLayout->childItems()[i];
            bool isparabolicspacer = item->property("isParabolicEdgeSpacer").toBool();

            if (isparabolicspacer) {
                continue;
            }

            index++;
            if (item == m_dndSpacer) {
                return index;
            }
        }
    }

    for(int i=0; i<m_mainLayout->childItems().count(); ++i) {
        QQuickItem *item = m_mainLayout->childItems()[i];       
        bool isparabolicspacer = item->property("isParabolicEdgeSpacer").toBool();

        if (isparabolicspacer) {
            continue;
        }

        index++;
        if (item == m_dndSpacer) {
            return index;
        }
    }

    if (alignment == Latte::Types::Justify) {
        for(int i=0; i<m_endLayout->childItems().count(); ++i) {
            QQuickItem *item = m_endLayout->childItems()[i];
            bool isparabolicspacer = item->property("isParabolicEdgeSpacer").toBool();

            if (isparabolicspacer) {
                continue;
            }

            index++;
            if (item == m_dndSpacer) {
                return index;
            }
        }
    }

    return -1;
}


void LayoutManager::requestAppletsOrder(const QList<int> &order)
{
    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());
    QQuickItem *nextlayout = alignment != Latte::Types::Justify ? m_mainLayout : m_startLayout;
    QQuickItem *previousitem = nullptr;

    int addedsplitters{0};

    for (int i=0; i<order.count(); ++i) {
        QQuickItem *currentitem;

        if (alignment != Latte::Types::Justify || order[i] != JUSTIFYSPLITTERID) {
            currentitem = appletItem(order[i]);
        } else if (alignment == Latte::Types::Justify && order[i] == JUSTIFYSPLITTERID) {
            currentitem = addedsplitters == 0 ? firstSplitter() : lastSplitter();
            addedsplitters++;
        }


        if (previousitem) {
            insertAfter(previousitem, currentitem);
        } else {
            insertAtLayoutTail(nextlayout, currentitem);
        }

        previousitem = currentitem;

        if (alignment == Latte::Types::Justify && order[i] == JUSTIFYSPLITTERID) {
            nextlayout = addedsplitters == 1 ? m_mainLayout : m_endLayout;
        }
    }

    if (alignment == Latte::Types::Justify) {
        moveAppletsBasedOnJustifyAlignment();
        save();
    }
}

void LayoutManager::requestAppletsInLockedZoom(const QList<int> &applets)
{
    setLockedZoomApplets(applets);
}

void LayoutManager::requestAppletsDisabledColoring(const QList<int> &applets)
{
    setUserBlocksColorizingApplets(applets);
}

int LayoutManager::distanceFromTail(QQuickItem *layout, QPointF pos) const
{
    return (int)qSqrt(qPow(pos.x() - 0, 2) + qPow(pos.y() - 0, 2));
}

int LayoutManager::distanceFromHead(QQuickItem *layout, QPointF pos) const
{
    float rightX = layout->width() - 1;
    float rightY = layout->height() - 1;
    return  (int)qSqrt(qPow(pos.x() - rightX, 2) + qPow(pos.y() - rightY, 2));
}

void LayoutManager::insertAtCoordinates(QQuickItem *item, const int &x, const int &y)
{
    if (!m_configuration || !item) {
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

    if (result) {
        return;
    }

    //! item was not added because it does not hover any of the layouts and layout items
    //! so the place that will be added is specified by the distance of the item from the layouts

    QPointF startrelpos = m_startLayout->mapFromItem(m_rootItem, QPointF(x, y));
    QPointF endrelpos = m_endLayout->mapFromItem(m_rootItem, QPointF(x, y));
    QPointF mainrelpos = m_mainLayout->mapFromItem(m_rootItem, QPointF(x, y));

    int starttaildistance = distanceFromTail(m_startLayout, startrelpos);
    int startheaddistance = distanceFromHead(m_startLayout, startrelpos);
    int maintaildistance = distanceFromTail(m_mainLayout, mainrelpos);
    int mainheaddistance = distanceFromHead(m_mainLayout, mainrelpos);
    int endtaildistance = distanceFromTail(m_endLayout, endrelpos);
    int endheaddistance = distanceFromHead(m_endLayout, endrelpos);

    int startdistance = qMin(starttaildistance, startheaddistance);
    int maindistance = qMin(maintaildistance, mainheaddistance);
    int enddistance = qMin(endtaildistance, endheaddistance);

    if (alignment != Latte::Types::Justify || (maindistance < startdistance && maindistance < enddistance)) {
        if (maintaildistance <= mainheaddistance) {
            insertAtLayoutTail(m_mainLayout, item);
        } else {
            insertAtLayoutHead(m_mainLayout, item);
        }
    } else if (startdistance < maindistance && startdistance < enddistance) {
        if (maintaildistance <= mainheaddistance) {
            insertAtLayoutTail(m_startLayout, item);
        } else {
            insertAtLayoutHead(m_startLayout, item);
        }
    } else {
        if (endtaildistance <= endheaddistance) {
            insertAtLayoutTail(m_endLayout, item);
        } else {
            insertAtLayoutHead(m_endLayout, item);
        }
    }
}

void LayoutManager::cleanupOptions()
{
    auto inlockedzoomcurrent = m_lockedZoomApplets;
    QList<int> inlockedzoomnext;
    for(int i=0; i<inlockedzoomcurrent.count(); ++i) {
        if (m_appletOrder.contains(inlockedzoomcurrent[i])) {
            inlockedzoomnext << inlockedzoomcurrent[i];
        }
    }
    setLockedZoomApplets(inlockedzoomnext);

    auto disabledcoloringcurrent = m_userBlocksColorizingApplets;
    QList <int> disabledcoloringnext;
    for(int i=0; i<disabledcoloringcurrent.count(); ++i) {
        if (m_appletOrder.contains(disabledcoloringcurrent[i])) {
            disabledcoloringnext << disabledcoloringcurrent[i];
        }
    }
    setUserBlocksColorizingApplets(disabledcoloringnext);
}

void LayoutManager::addAppletItem(QObject *applet, int index)
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout || index < 0) {
        return;
    }

    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());
    QVariant appletItemVariant;
    QVariant appletVariant; appletVariant.setValue(applet);
    m_createAppletItemMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, appletItemVariant), Q_ARG(QVariant, appletVariant));
    QQuickItem *aitem = appletItemVariant.value<QQuickItem *>();

    if (m_dndSpacer) {
        m_dndSpacer->setParentItem(m_rootItem);
    }

    QQuickItem *previousItem{nullptr};

    if (index >= m_order.count()) {
        // do nothing it should be added at the end
    } else {
        if (alignment == Latte::Types::Justify && m_order[index] == JUSTIFYSPLITTERID) {
            if (index<m_splitterPosition2-1) {
                previousItem = firstSplitter();
            } else {
                previousItem = lastSplitter();
            }
        } else {
            previousItem = appletItem(m_order[index]);
        }
    }

    if (previousItem) {
        insertBefore(previousItem, aitem);
    } else {
        if (alignment == Latte::Types::Justify) {
            insertAtLayoutHead(m_endLayout, aitem);
        } else {
            insertAtLayoutHead(m_mainLayout, aitem);
        }
    }

    if (alignment == Latte::Types::Justify) {
        moveAppletsBasedOnJustifyAlignment();
    }

    save();
}

void LayoutManager::addAppletItem(QObject *applet, int x, int y)
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }    

    PlasmaQuick::AppletQuickItem *aqi = qobject_cast<PlasmaQuick::AppletQuickItem *>(applet);

    if (aqi && aqi->applet() && !aqi->applet()->destroyed() && m_appletsInScheduledDestruction.contains(aqi->applet()->id())) {
        int id = aqi->applet()->id();
        QVariant appletContainerVariant; appletContainerVariant.setValue(m_appletsInScheduledDestruction[id]);
        QVariant appletVariant; appletVariant.setValue(applet);
        m_initAppletContainerMethod.invoke(m_rootItem, Q_ARG(QVariant, appletContainerVariant), Q_ARG(QVariant, appletVariant));
        setAppletInScheduledDestruction(id, false);
        return;
    }

    QVariant appletItemVariant;
    QVariant appletVariant; appletVariant.setValue(applet);
    m_createAppletItemMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, appletItemVariant), Q_ARG(QVariant, appletVariant));
    QQuickItem *appletItem = appletItemVariant.value<QQuickItem *>();

    if (m_dndSpacer->parentItem() == m_mainLayout
            || m_dndSpacer->parentItem() == m_startLayout
            || m_dndSpacer->parentItem() == m_endLayout) {
        insertBefore(m_dndSpacer, appletItem);

        QQuickItem *currentlayout = m_dndSpacer->parentItem();
        m_dndSpacer->setParentItem(m_rootItem);

        if (currentlayout == m_startLayout) {
            reorderSplitterInStartLayout();
        } else if (currentlayout ==m_endLayout) {
            reorderSplitterInEndLayout();
        }
    } else if (x >= 0 && y >= 0) {
        // If the provided position is valid, use it.
        insertAtCoordinates(appletItem, x , y);
    } else {
        // Fall through to adding at the end of main layout.
        appletItem->setParentItem(m_mainLayout);
    }

    save();
}

void LayoutManager::removeAppletItem(QObject *applet)
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }

    PlasmaQuick::AppletQuickItem *aqi = qobject_cast<PlasmaQuick::AppletQuickItem *>(applet);

    if (!aqi) {
        return;
    }

    int id = aqi->applet()->id();

    if (aqi->applet() && aqi->applet()->destroyed() && !m_appletsInScheduledDestruction.contains(id)/*this way we really delete it in the end*/) {
        setAppletInScheduledDestruction(id, true);
        return;
    }

    destroyAppletContainer(aqi->applet());
}

void LayoutManager::destroyAppletContainer(QObject *applet)
{
    if (!applet || !m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }

    Plasma::Applet *ca = qobject_cast<Plasma::Applet *>(applet);

    if (!ca) {
        qDebug() << "org.kde.destroy destroying applet could not succeed Plasma/Applet was not identified...";
        return;
    }

    int id = ca->id();

    bool destroyed{false};

    if (m_appletsInScheduledDestruction.contains(id)) {
        //! when deleted from scheduled destruction
        m_appletsInScheduledDestruction[id]->setVisible(false);
        m_appletsInScheduledDestruction[id]->setParentItem(m_rootItem);
        m_appletsInScheduledDestruction[id]->deleteLater();
        setAppletInScheduledDestruction(id, false);
        destroyed = true;
    } else {
        //! when deleted directly for Plasma::Applet destruction e.g. synced applets
        for (int i=0; i<=2; ++i) {
            if (destroyed) {
                break;
            }

            QQuickItem *layout = (i==0 ? m_startLayout : (i==1 ? m_mainLayout : m_endLayout));

            if (layout->childItems().count() > 0) {
                int size = layout->childItems().count();
                for (int j=size-1; j>=0; --j) {
                    QQuickItem *item = layout->childItems()[j];
                    bool issplitter = item->property("isInternalViewSplitter").toBool();
                    if (issplitter) {
                        continue;
                    }

                    QVariant appletVariant = item->property("applet");
                    if (!appletVariant.isValid()) {
                        continue;
                    }
                    PlasmaQuick::AppletQuickItem *appletitem = appletVariant.value<PlasmaQuick::AppletQuickItem *>();

                    if (appletitem && appletitem->applet() == applet) {
                        item->setVisible(false);
                        item->setParentItem(m_rootItem);
                        item->deleteLater();
                        destroyed = true;
                        break;
                    }
                }
            }
        }
    }

    if (destroyed) {
        save();
    }
}

void LayoutManager::reorderSplitterInStartLayout()
{
    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());

    if (alignment != Latte::Types::Justify) {
        return;
    }

    int size = m_startLayout->childItems().count();

    if (size > 0) {
        QQuickItem *splitter{nullptr};

        for (int i=0; i<size; ++i) {
            QQuickItem *item = m_startLayout->childItems()[i];
            bool issplitter = item->property("isInternalViewSplitter").toBool();

            if (issplitter && i<size-1) {
                splitter = item;
                break;
            }
        }

        if (splitter) {
            insertAfter(m_startLayout->childItems()[size-1],splitter);
        }
    }
}

void LayoutManager::reorderSplitterInEndLayout()
{
    Latte::Types::Alignment alignment = static_cast<Latte::Types::Alignment>((*m_configuration)["alignment"].toInt());

    if (alignment != Latte::Types::Justify) {
        return;
    }

    int size = m_endLayout->childItems().count();

    if (size > 0) {
        QQuickItem *splitter{nullptr};

        for (int i=0; i<size; ++i) {
            QQuickItem *item = m_endLayout->childItems()[i];
            bool issplitter = item->property("isInternalViewSplitter").toBool();

            if (issplitter && i!=0) {
                splitter = item;
                break;
            }
        }

        if (splitter) {
            insertBefore(m_endLayout->childItems()[0],splitter);
        }
    }
}

void LayoutManager::addJustifySplittersInMainLayout()
{
    if (!m_configuration || !m_mainLayout) {
        return;
    }

    destroyJustifySplitters();

    int splitterPosition = (*m_configuration)["splitterPosition"].toInt();
    int splitterPosition2 = (*m_configuration)["splitterPosition2"].toInt();

    int splitterIndex = (splitterPosition >= 1 ? splitterPosition - 1 : -1);
    int splitterIndex2 = (splitterPosition2 >= 1 ? splitterPosition2 - 1 : -1);

    //! First Splitter
    QVariant splitterItemVariant;
    m_createJustifySplitterMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, splitterItemVariant));
    QQuickItem *splitterItem = splitterItemVariant.value<QQuickItem *>();

    int size = m_mainLayout->childItems().count()-2; //we need to remove parabolic spacers

    splitterItem->setParentItem(m_mainLayout);

    if (size>0 && splitterIndex>=0) {
        bool atend = (splitterIndex >= size);
        int validindex = (atend ? size-1 : splitterIndex) + 1; //we need to take into account first parabolic spacer
        QQuickItem *currentitem = m_mainLayout->childItems()[validindex];

        if (atend) {
            splitterItem->stackAfter(currentitem);
        } else {
            splitterItem->stackBefore(currentitem);
        }
    } else if (size>0) {
        //! add in first position
        QQuickItem *currentitem = m_mainLayout->childItems()[0];
        splitterItem->stackBefore(currentitem);
    }

    //! Second Splitter
    QVariant splitterItemVariant2;
    m_createJustifySplitterMethod.invoke(m_rootItem, Q_RETURN_ARG(QVariant, splitterItemVariant2));
    QQuickItem *splitterItem2 = splitterItemVariant2.value<QQuickItem *>();

    int size2 = m_mainLayout->childItems().count()-2; //we need to remove parabolic spacers

    splitterItem2->setParentItem(m_mainLayout);

    if (size2>0 && splitterIndex2>=0) {
        bool atend = (splitterIndex2 >= size2);
        int validindex2 = (atend ? size2-1 : splitterIndex2) + 1; //we need to take into account first parabolic spacer
        QQuickItem *currentitem2 = m_mainLayout->childItems()[validindex2];

        if (atend) {
            splitterItem2->stackAfter(currentitem2);
        } else {
            splitterItem2->stackBefore(currentitem2);
        }
    } else if (size2>1){
        //! add in last position
        QQuickItem *currentitem2 = m_mainLayout->childItems()[size2-1+1]; //we need to take into account first parabolic spacer
        splitterItem2->stackAfter(currentitem2);
    }
}

void LayoutManager::destroyJustifySplitters()
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }

    for (int i=0; i<=2; ++i) {
        QQuickItem *layout = (i==0 ? m_startLayout : (i==1 ? m_mainLayout : m_endLayout));

        if (layout->childItems().count() > 0) {
            int size = layout->childItems().count();
            for (int j=size-1; j>=0; --j) {
                QQuickItem *item = layout->childItems()[j];
                bool issplitter = item->property("isInternalViewSplitter").toBool();
                if (issplitter) {
                    item->deleteLater();
                }
            }
        }
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

    destroyJustifySplitters();
}

void LayoutManager::moveAppletsBasedOnJustifyAlignment()
{
    if (!m_startLayout || !m_mainLayout || !m_endLayout) {
        return;
    }

    reorderParabolicSpacers();

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
        bool isparabolicspacer = appletlist[i]->property("isParabolicEdgeSpacer").toBool();

        if (!firstSplitterFound) {
            insertAtLayoutIndex(m_startLayout, appletlist[i], i);
            if (issplitter) {
                firstSplitterFound = true;
                splitter1 = i;
            }
        } else if (firstSplitterFound && !secondSplitterFound) {
            if (issplitter) {
                secondSplitterFound = true;
                splitter2 = i;
                insertAtLayoutTail(m_endLayout, appletlist[i]);
            } else {
                insertAtLayoutIndex(m_mainLayout, appletlist[i], i-splitter1);
            }
        } else if (firstSplitterFound && secondSplitterFound) {
            insertAtLayoutIndex(m_endLayout, appletlist[i], i-splitter2);
        }
    }

    reorderParabolicSpacers();
}

void LayoutManager::printAppletList(QList<QQuickItem *> list)
{
    for(int i=0; i<list.count(); ++i) {
        bool issplitter = list[i]->property("isInternalViewSplitter").toBool();
        bool isparabolicspacer = list[i]->property("isParabolicEdgeSpacer").toBool();

        if (issplitter) {
            qDebug() << i << " __ JUSTIFY SPLITTER";
            continue;
        }

        if (isparabolicspacer) {
            qDebug() << i << " __ PARABOLICSPACER";
            continue;
        }

        QVariant appletVariant = list[i]->property("applet");
        if (!appletVariant.isValid()) {
            continue;
        }
        PlasmaQuick::AppletQuickItem *appletitem = appletVariant.value<PlasmaQuick::AppletQuickItem *>();

        if (appletitem) {
            qDebug() << i << " __ " << appletitem->applet()->pluginMetaData().pluginId();
        }
    }
}

QList<int> LayoutManager::toIntList(const QString &serialized)
{
    QList<int> list;
    QStringList items = serialized.split(";");
    items.removeAll(QString());

    for(const auto &item: items) {
        list << item.toInt();
    }

    return list;
}

QString LayoutManager::toStr(const QList<int> &list)
{
    QString str;
    QStringList strlist;

    for(const auto &num: list) {
        strlist << QString::number(num);
    }

    return strlist.join(";");
}

}
}
