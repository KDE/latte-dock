/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VIEWDATA_H
#define VIEWDATA_H

// local
#include <coretypes.h>
#include "genericdata.h"
#include "../screenpool.h"

// Qt
#include <QMetaType>
#include <QString>

// Plasma
#include <Plasma>


namespace Latte {
namespace Data {

class View : public Generic
{
public:
    enum OriginType {
        IsCreated = 0,
        OriginFromViewTemplate,
        OriginFromLayout
    };

    View();
    View(View &&o);
    View(const View &o);

    //! View data
    bool onPrimary{true};
    int screen{Latte::ScreenPool::FIRSTSCREENID};
    float maxLength{1.0};
    Plasma::Types::Location edge{Plasma::Types::BottomEdge};
    Latte::Types::Alignment alignment{Latte::Types::Center};

    bool hasViewTemplateOrigin() const;
    bool hasLayoutOrigin() const;

    QString tempId() const;

    void setOrigin(OriginType origin, QString file = QString(), QString view = QString());

    //! Operators
    View &operator=(const View &rhs);
    View &operator=(View &&rhs);
    bool operator==(const View &rhs) const;
    bool operator!=(const View &rhs) const;

protected:
    OriginType originType{IsCreated};

    //! Origin Data
    QString originFile;
    QString originView;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::View)

#endif
