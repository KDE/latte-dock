/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "errordata.h"

namespace Latte {
namespace Data {

Error::Error()
    : Generic()
{
}

Error::Error(Error &&o)
    : Generic(o),
      information(o.information)
{
}

Error::Error(const Error &o)
    : Generic(o),
      information(o.information)
{
}

Error &Error::operator=(const Error &rhs)
{
    id = rhs.id;
    name = rhs.name;
    information = rhs.information;

    return (*this);
}

Error &Error::operator=(Error &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    information = rhs.information;

    return (*this);
}

bool Error::operator==(const Error &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (information == rhs.information);
}

bool Error::operator!=(const Error &rhs) const
{
    return !(*this == rhs);
}

bool Error::isValid() const
{
    return !id.isEmpty();
}

}
}
