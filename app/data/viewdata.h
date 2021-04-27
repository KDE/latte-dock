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
    enum State {
        IsInvalid = -1,
        IsCreated = 0,
        OriginFromViewTemplate, /*used for view templates files*/
        OriginFromLayout /*used from duplicate, copy, move view functions*/
    };

    View();
    View(View &&o);
    View(const View &o);

    //! View data
    bool isActive{false};
    bool onPrimary{true};
    int screen{Latte::ScreenPool::FIRSTSCREENID};
    int screenEdgeMargin{0};
    float maxLength{1.0};
    Plasma::Types::Location edge{Plasma::Types::BottomEdge};
    Latte::Types::Alignment alignment{Latte::Types::Center};
    GenericTable<Data::Generic> subcontainments;

    //! View sub-states
    bool isMoveOrigin{false};
    bool isMoveDestination{false};

    bool isValid() const;
    bool isCreated() const;
    bool hasViewTemplateOrigin() const;
    bool hasLayoutOrigin() const;
    bool hasSubContainment(const QString &subId) const;

    bool isHorizontal() const;
    bool isVertical() const;

    QString originFile() const;
    QString originLayout() const;
    QString originView() const;    

    View::State state() const;
    void setState(View::State state, QString file = QString(), QString layout = QString(), QString view = QString());

    //! Operators
    View &operator=(const View &rhs);
    View &operator=(View &&rhs);
    bool operator==(const View &rhs) const;
    bool operator!=(const View &rhs) const;
    operator QString() const;

protected:
    View::State m_state{IsInvalid};

    //! Origin Data
    QString m_originFile;
    QString m_originLayout;
    QString m_originView;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::View)

#endif
