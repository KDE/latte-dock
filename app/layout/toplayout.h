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

#ifndef TOPLAYOUT_H
#define TOPLAYOUT_H

// local
#include "genericlayout.h"

// Qt
#include <QObject>

namespace Latte {
class ActiveLayout;
}


namespace Latte {

//! TopLayout is a layout that exists only as long as it belongs to one or
//! more ActiveLayout(s). It is a layer above an active or more layouts and can
//! be used from ActiveLayouts to share Latte:View(s) . Much of its functionality
//! is provided by the ActiveLayouts it belongs to. For example the activities
//! that its views should be shown is identified only from the active layouts
//! it belongs to

class TopLayout : public Layout::GenericLayout
{
public:
    TopLayout(ActiveLayout *assigned, QObject *parent, QString layoutFile, QString layoutName = QString());
    ~TopLayout() override;

    const QStringList appliedActivities();

public slots:
    void addActiveLayout(ActiveLayout *layout);
    void removeActiveLayout(ActiveLayout *layout);

private:
    QList<ActiveLayout *> m_activeLayouts;

};

}

#endif
