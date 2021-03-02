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

//Qt

namespace Latte{
namespace Containment{

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent)
{
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

//! Actions
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
