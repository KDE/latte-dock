/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "viewsettingsfactory.h"

#include "primaryconfigview.h"
#include "widgetexplorerview.h"
#include "../view.h"

// Plasma
#include <Plasma/Containment>

namespace Latte {

ViewSettingsFactory::ViewSettingsFactory(QObject *parent)
    : QObject(parent)
{
}

ViewSettingsFactory::~ViewSettingsFactory()
{
    if (m_primaryConfigView) {
        delete m_primaryConfigView;
    }

    if (m_widgetExplorerView) {
        delete m_widgetExplorerView;
    }
}

bool ViewSettingsFactory::hasOrphanSettings() const
{
    return m_primaryConfigView && !m_primaryConfigView->parentView();
}

bool ViewSettingsFactory::hasVisibleSettings() const
{
    return m_primaryConfigView && m_primaryConfigView->isVisible();
}


Plasma::Containment *ViewSettingsFactory::lastContainment()
{
    return m_lastContainment;
}

ViewPart::PrimaryConfigView *ViewSettingsFactory::primaryConfigView()
{
    return m_primaryConfigView;
}

ViewPart::PrimaryConfigView *ViewSettingsFactory::primaryConfigView(Latte::View *view)
{
    if (!m_primaryConfigView) {
        m_primaryConfigView = new ViewPart::PrimaryConfigView(view);
    } else {
        auto previousView = m_primaryConfigView->parentView();

        if (previousView) {
            previousView->releaseConfigView();
        }

        m_primaryConfigView->setParentView(view);
    }

    if (view) {
        m_lastContainment = view->containment();
    }

    return m_primaryConfigView;
}

ViewPart::WidgetExplorerView *ViewSettingsFactory::widgetExplorerView(Latte::View *view)
{
    if (!m_widgetExplorerView) {
        m_widgetExplorerView = new ViewPart::WidgetExplorerView(view);
    } else {
        m_widgetExplorerView->setParentView(view);
    }

    return m_widgetExplorerView;
}


}
