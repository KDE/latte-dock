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

#ifndef LAYOUTCOLORDATA_H
#define LAYOUTCOLORDATA_H

//Qt
#include <QMetaType>
#include <QString>

namespace Latte {
namespace Data {

class LayoutColor
{
public:
    LayoutColor();
    LayoutColor(LayoutColor &&o);
    LayoutColor(const LayoutColor &o);

    //! Color data
    QString id;
    QString name;
    QString path;
    QString textColor;

    //! Operators
    LayoutColor &operator=(const LayoutColor &rhs);
    LayoutColor &operator=(LayoutColor &&rhs);
    bool operator==(const LayoutColor &rhs) const;
    bool operator!=(const LayoutColor &rhs) const;

    void setData(const QString &newid, const QString &newname, const QString &newpath, const QString &newtextcolor);
};

}
}

Q_DECLARE_METATYPE(Latte::Data::LayoutColor)

#endif
