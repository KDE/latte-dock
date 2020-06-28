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
#include "../view.h"

namespace Latte {

ViewSettingsFactory::ViewSettingsFactory(QObject *parent)
    : QObject(parent)
{
}

ViewSettingsFactory::~ViewSettingsFactory()
{
}

ViewPart::PrimaryConfigView *ViewSettingsFactory::primary(Latte::View *view)
{
    if (!m_primaryConfigView) {
        m_primaryConfigView = new ViewPart::PrimaryConfigView(view);
    } else {
        auto previousView = m_primaryConfigView->view();

        if (previousView) {
            previousView->releaseConfigView();
        }

        m_primaryConfigView->setView(view);
    }

    return m_primaryConfigView;
}


}
