/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewsettingsfactory.h"

// local
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
        //!set user configuring early enough in order to give config windows time to be created properly
        view->containment()->setUserConfiguring(true);

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
    //! it is deleted on hiding
    auto widgetExplorerView = new ViewPart::WidgetExplorerView(view);
    return widgetExplorerView;
}


}
