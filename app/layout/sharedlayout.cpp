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

#include "sharedlayout.h"

// local
#include "centrallayout.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/manager.h"
#include "../view/view.h"

namespace Latte {

SharedLayout::SharedLayout(CentralLayout *assigned, QObject *parent, QString layoutFile, QString layoutName)
    : Layout::GenericLayout (parent, layoutFile, layoutName)
{
    initToCorona(assigned->corona());

    connect(m_corona->layoutsManager(), &Layouts::Manager::currentLayoutNameChanged, this, &SharedLayout::updateLastUsedCentralLayout);

    addCentralLayout(assigned);
    updateLastUsedCentralLayout();
}


SharedLayout::~SharedLayout()
{
    qDebug() << " SHARED LAYOUT destroying ::: " << name();
}

bool SharedLayout::isCurrent() const
{
    for (const auto  &layout : m_centralLayouts) {
        if (layout->isCurrent()) {
            return true;
        }
    }

    return false;
}

Layout::Type SharedLayout::type() const
{
    return Layout::Type::Shared;
}


const QStringList SharedLayout::appliedActivities()
{
    if (!m_corona) {
        return {};
    }

    QStringList activities;

    for (const auto  &layout : m_centralLayouts) {
        activities << layout->appliedActivities();
    }

    return activities;
}

void SharedLayout::updateLastUsedCentralLayout()
{
    for (const auto  &layout : m_centralLayouts) {
        if (layout->isCurrent()) {
            m_lastUsedCentralLayout = layout->name();
            break;
        }
    }
}

CentralLayout *SharedLayout::currentCentralLayout() const
{
    //! first the current active one
    for (const auto  &layout : m_centralLayouts) {
        if (layout->isCurrent()) {
            return layout;
        }
    }

    //! the last used
    for (const auto  &layout : m_centralLayouts) {
        if (layout->name() == m_lastUsedCentralLayout) {
            return layout;
        }
    }

    return nullptr;
}

void SharedLayout::addCentralLayout(CentralLayout *layout)
{
    if (layout != nullptr && !m_centralLayouts.contains(layout)) {
        m_centralLayouts.append(layout);

        qDebug() << " ADDING Central : " << layout->name() << " at Shared: " << name();
        connect(layout, &GenericLayout::activitiesChanged, this, &GenericLayout::activitiesChanged);
        emit activitiesChanged();
        emit viewsCountChanged();

        updateLastUsedActivity();
    }
}

void SharedLayout::removeCentralLayout(CentralLayout *layout)
{
    if (m_centralLayouts.contains(layout)) {
        qDebug() << "SHAREDLAYOUT <" << name() << "> : Removing active layout, " << layout->name();
        m_centralLayouts.removeAll(layout);

        disconnect(layout, &GenericLayout::activitiesChanged, this, &GenericLayout::activitiesChanged);

        if (m_centralLayouts.count() > 0) {
            emit activitiesChanged();
        } else {
            //! all assigned layouts have been unloaded so the shared layout should be destroyed also
            emit layoutDestroyed(this);
        }

        //! viewsCount signal is not needed to be trigerred here because
        //! in such case the views number has not been changed for the rest
        //! active layouts
    }
}

//! OVERRIDE
int SharedLayout::viewsCount(int screen) const
{
    if (!m_corona) {
        return 0;
    }

    CentralLayout *current = currentCentralLayout();

    if (current) {
        return current->viewsCount(screen);
    }

    return Layout::GenericLayout::viewsCount(screen);
}

int SharedLayout::viewsCount(QScreen *screen) const
{
    if (!m_corona) {
        return 0;
    }

    CentralLayout *current = currentCentralLayout();

    if (current) {
        return current->viewsCount(screen);
    }

    return Layout::GenericLayout::viewsCount(screen);;
}

int SharedLayout::viewsCount() const
{
    if (!m_corona) {
        return 0;
    }
    
    CentralLayout *current = currentCentralLayout();

    if (current) {
        return current->viewsCount();
    }

    return Layout::GenericLayout::viewsCount();
}

QList<Plasma::Types::Location> SharedLayout::availableEdgesForView(QScreen *scr, Latte::View *forView) const
{
 /*   using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }*/

    return Layout::GenericLayout::availableEdgesForView(scr, forView);
}

QList<Plasma::Types::Location> SharedLayout::freeEdges(QScreen *scr) const
{
 /*   using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    CentralLayout *current = currentCentralLayout();

    if (current) {
        return current->freeEdges(scr);
    }*/

    return Layout::GenericLayout::freeEdges(scr);
}

QList<Plasma::Types::Location> SharedLayout::freeEdges(int screen) const
{
  /*  using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                Types::TopEdge, Types::RightEdge};

    if (!m_corona) {
        return edges;
    }

    CentralLayout *current = currentCentralLayout();

    if (current) {
        return current->freeEdges(screen);
    }*/

    return Layout::GenericLayout::freeEdges(screen);
}

QList<Latte::View *> SharedLayout::sortedLatteViews(QList<Latte::View *> views)
{
    CentralLayout *current = currentCentralLayout();

    if (current) {
        return current->sortedLatteViews();
    }

    return Layout::GenericLayout::sortedLatteViews();
}

}
