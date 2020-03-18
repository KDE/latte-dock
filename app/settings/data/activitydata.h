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

#ifndef SETTINGSDATAACTIVITY_H
#define SETTINGSDATAACTIVITY_H

//! Qt
#include <QMetaType>
#include <QIcon>
#include <QString>

//! KActivities
#include <KActivities/Info>

namespace Latte {
namespace Settings {
namespace Data {

class Activity
{
public:
    Activity();
    Activity(Activity &&o);
    Activity(const Activity &o);

    //! Layout data
    QString id;
    QString name;
    QString icon;
    bool isCurrent;
    KActivities::Info::State state;

    bool isValid() const;
    bool isRunning() const;

    //! Operators
    Activity &operator=(const Activity &rhs);
    Activity &operator=(Activity &&rhs);
};

//! This is an Activities map in the following structure:
//! #activityId -> activite_information
typedef QHash<const QString, Activity> ActivitiesMap;

}
}
}

Q_DECLARE_METATYPE(Latte::Settings::Data::Activity)
Q_DECLARE_METATYPE(Latte::Settings::Data::ActivitiesMap)

#endif
