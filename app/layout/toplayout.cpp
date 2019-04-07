/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "toplayout.h"

// local
#include "activelayout.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../view/view.h"

namespace Latte {

TopLayout::TopLayout(ActiveLayout *assigned, QObject *parent, QString layoutFile, QString layoutName)
    : Layout::GenericLayout (parent, layoutFile, layoutName)
{
    initToCorona(assigned->corona());
    addActiveLayout(assigned);
}


TopLayout::~TopLayout()
{
}

bool TopLayout::isCurrent() const
{
    for (const auto  &layout : m_activeLayouts) {
        if (layout->isCurrent()) {
            return true;
        }
    }

    return false;
}

const QStringList TopLayout::appliedActivities()
{
    if (!m_corona) {
        return {};
    }

    QStringList activities;

    for (const auto  &layout : m_activeLayouts) {
        activities << layout->appliedActivities();
    }

    return activities;
}

ActiveLayout *TopLayout::currentActiveLayout() const
{
    for (const auto  &layout : m_activeLayouts) {
        if (layout->isCurrent()) {
            return layout;
        }
    }

    return nullptr;
}

void TopLayout::addActiveLayout(ActiveLayout *layout)
{
    if (layout != nullptr && !m_activeLayouts.contains(layout)) {
        m_activeLayouts.append(layout);

        connect(layout, &GenericLayout::activitiesChanged, this, &GenericLayout::activitiesChanged);
        emit activitiesChanged();
        emit viewsCountChanged();

        updateLastUsedActivity();
    }
}

void TopLayout::removeActiveLayout(ActiveLayout *layout)
{
    if (m_activeLayouts.contains(layout)) {
        qDebug() << "TOPLAYOUT <" << name() << "> : Removing active layout, " << layout->name();

        m_activeLayouts.removeAll(layout);

        disconnect(layout, &GenericLayout::activitiesChanged, this, &GenericLayout::activitiesChanged);
        emit activitiesChanged();

        //! viewsCount signal is not needed to be trigerred here because
        //! in such case the views number has not been changed for the rest
        //! active layouts
    }
}

//! OVERRIDE
int TopLayout::viewsCount(int screen) const
{
    if (!m_corona) {
        return 0;
    }

    int views  = Layout::GenericLayout::viewsCount(screen);
    ActiveLayout *current = currentActiveLayout();

    if (current) {
        views += current->viewsCount(screen);
    }

    return views;
}

int TopLayout::viewsCount(QScreen *screen) const
{
    if (!m_corona) {
        return 0;
    }

    int views = Layout::GenericLayout::viewsCount(screen);
    ActiveLayout *current = currentActiveLayout();

    if (current) {
        views += current->viewsCount(screen);
    }

    return views;
}

int TopLayout::viewsCount() const
{
    if (!m_corona) {
        return 0;
    }

    int views  = Layout::GenericLayout::viewsCount();
    ActiveLayout *current = currentActiveLayout();

    if (current) {
        views += current->viewsCount();
    }

    return views;
}

QList<Plasma::Types::Location> TopLayout::availableEdgesForView(QScreen *scr, Latte::View *forView) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    edges = Layout::GenericLayout::availableEdgesForView(scr, forView);

    ActiveLayout *current = currentActiveLayout();
    if (current) {
        for (const auto view : current->latteViews()) {
            //! make sure that availabe edges takes into account only views that should be excluded,
            //! this is why the forView should not be excluded
            if (view && view != forView && view->positioner()->currentScreenName() == scr->name()) {
                edges.removeOne(view->location());
            }
        }
    }

    return edges;
}

QList<Plasma::Types::Location> TopLayout::freeEdges(QScreen *scr) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    edges = Layout::GenericLayout::freeEdges(scr);

    ActiveLayout *current = currentActiveLayout();

    if (current) {
        for (const auto view : current->latteViews()) {
            if (view && view->positioner()->currentScreenName() == scr->name()) {
                edges.removeOne(view->location());
            }
        }
    }

    return edges;
}

QList<Plasma::Types::Location> TopLayout::freeEdges(int screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    edges = Layout::GenericLayout::freeEdges(screen);
    QScreen *scr = m_corona->screenPool()->screenForId(screen);
    ActiveLayout *current = currentActiveLayout();

    if (current) {
        for (const auto view : current->latteViews()) {
            if (view && scr && view->positioner()->currentScreenName() == scr->name()) {
                edges.removeOne(view->location());
            }
        }
    }

    return edges;
}

QList<Latte::View *> TopLayout::sortedLatteViews(QList<Latte::View *> views)
{
    QList<Latte::View *> combined = latteViews();

    ActiveLayout *current = currentActiveLayout();

    if (current) {
        combined << current->latteViews();
    }

    return Layout::GenericLayout::sortedLatteViews(combined);
}

}
