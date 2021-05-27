/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERICDATA_H
#define GENERICDATA_H

// local
#include "generictable.h"

// Qt
#include <QString>

namespace Latte {
namespace Data {

class Generic
{
public:
    Generic() = default;
    Generic(Generic &&o);
    Generic(const Generic &o);
    Generic(const QString &newid, const QString &newname);

    //! Layout data
    QString id;
    QString name;

    //! Operators
    Generic &operator=(const Generic &rhs);
    Generic &operator=(Generic &&rhs);
    bool operator==(const Generic &rhs) const;
    bool operator!=(const Generic &rhs) const;
};

}
}



#endif
