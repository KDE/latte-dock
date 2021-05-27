/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "errorinformationdata.h"

namespace Latte {
namespace Data {

ErrorInformation::ErrorInformation()
    : Generic()
{
}

ErrorInformation::ErrorInformation(ErrorInformation &&o)
    : Generic(o),
      containment(o.containment),
      applet(o.applet)
{
}

ErrorInformation::ErrorInformation(const ErrorInformation &o)
    : Generic(o),
      containment(o.containment),
      applet(o.applet)
{
}

ErrorInformation &ErrorInformation::operator=(const ErrorInformation &rhs)
{
    id = rhs.id;
    name = rhs.name;
    containment = rhs.containment;
    applet = rhs.applet;

    return (*this);
}

ErrorInformation &ErrorInformation::operator=(ErrorInformation &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    containment = rhs.containment;
    applet = rhs.applet;

    return (*this);
}

bool ErrorInformation::operator==(const ErrorInformation &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (containment == rhs.containment)
            && (applet == rhs.applet);
}

bool ErrorInformation::operator!=(const ErrorInformation &rhs) const
{
    return !(*this == rhs);
}

bool ErrorInformation::isValid() const
{
    return containment.isValid() || applet.isValid();
}

}
}
