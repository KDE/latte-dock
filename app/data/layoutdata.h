/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef SETTINGSDATALAYOUT_H
#define SETTINGSDATALAYOUT_H

// local
#include "genericdata.h"
#include "viewstable.h"
#include "../layout/abstractlayout.h"

//Qt
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace Latte {
namespace Data {

class Layout : public Generic
{
public:
    static constexpr const char* ALLACTIVITIESID = "{0}";
    static constexpr const char* FREEACTIVITIESID = "{free-activities}";
    static constexpr const char* CURRENTACTIVITYID = "{current-activity}";

    Layout();
    Layout(Layout &&o);
    Layout(const Layout &o);

    //! Layout data
    QString icon;
    QString color;
    QString background;
    QString textColor;
    QString lastUsedActivity;
    bool isActive{false};
    bool isConsideredActive{false}; //used from settings window to indicate activeness based on selected layouts mode
    bool isBroken{false};
    bool isLocked{false};
    bool isShownInMenu{false};
    bool isTemplate{false};
    bool hasDisabledBorders{false};
    QStringList activities;

    Latte::Layout::BackgroundStyle backgroundStyle{Latte::Layout::ColorBackgroundStyle};

    ViewsTable views;

    //! Functionality
    bool isOnAllActivities() const;
    bool isForFreeActivities() const;
    bool isTemporary() const;
    bool isNull() const;
    bool isEmpty() const;
    bool isSystemTemplate() const;

    //! Operators
    Layout &operator=(const Layout &rhs);
    Layout &operator=(Layout &&rhs);
    bool operator==(const Layout &rhs) const;
    bool operator!=(const Layout &rhs) const;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::Layout)

#endif
