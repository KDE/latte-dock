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

#ifndef SCREENDATA_H
#define SCREENDATA_H

//! local
#include "genericdata.h"
#include "generictable.h"

//! Qt
#include <QMetaType>
#include <QRect>
#include <QString>

namespace Latte {
namespace Data {

class Screen : public Generic
{
public:
    static constexpr const char* SERIALIZESPLITTER = ":::";
    static const int ONPRIMARYID = 0;

    Screen();
    Screen(Screen &&o);
    Screen(const Screen &o);
    Screen(const QString &screenId, const QString &serialized);

    //! Screen data
    bool hasExplicitViews{false};
    bool isActive{false};
    QRect geometry;

    //! Operators
    Screen &operator=(const Screen &rhs);
    Screen &operator=(Screen &&rhs);
    bool operator==(const Screen &rhs) const;
    bool operator!=(const Screen &rhs) const;

    void init(const QString &screenId, const QString &serialized);
    QString serialize() const;

};

typedef GenericTable<Screen> ScreensTable;

}
}

Q_DECLARE_METATYPE(Latte::Data::Screen)
Q_DECLARE_METATYPE(Latte::Data::ScreensTable)

#endif
