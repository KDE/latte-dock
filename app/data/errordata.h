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

#ifndef ERRORDATA_H
#define ERRORDATA_H

//! local
#include "genericdata.h"
#include "appletdata.h"
#include "errorinformationdata.h"

//! Qt
#include <QMetaType>
#include <QString>

namespace Latte {
namespace Data {

class Error : public Data::Generic
{
public:
    Error();
    Error(Error &&o);
    Error(const Error &o);

    bool isValid() const;

    //! Operators
    Error &operator=(const Error &rhs);
    Error &operator=(Error &&rhs);
    bool operator==(const Error &rhs) const;
    bool operator!=(const Error &rhs) const;

    GenericTable<Data::ErrorInformation> information;

private:

};

}
}

Q_DECLARE_METATYPE(Latte::Data::Error)

#endif
