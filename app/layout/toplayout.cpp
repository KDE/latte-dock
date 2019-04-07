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

void TopLayout::addActiveLayout(ActiveLayout *layout)
{
    if (layout != nullptr && !m_activeLayouts.contains(layout)) {
        m_activeLayouts.append(layout);

        connect(layout, &GenericLayout::activitiesChanged, this, &GenericLayout::activitiesChanged);
        emit activitiesChanged();
        emit viewsCountChanged();
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


}
