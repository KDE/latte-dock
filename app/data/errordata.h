/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GENERICERRORDATA_H
#define GENERICERRORDATA_H

//! local
#include "genericdata.h"
#include "appletdata.h"
#include "errorinformationdata.h"

//! Qt
#include <QList>
#include <QMetaType>
#include <QString>

namespace Latte {
namespace Data {

class Error : public Data::Generic
{
public:
    //!errors and warnings use a step of four between them
    static constexpr const char* APPLETSWITHSAMEID = "E103";
    static constexpr const char* ORPHANEDPARENTAPPLETOFSUBCONTAINMENT = "E107";
    static constexpr const char* ORPHANEDSUBCONTAINMENT = "W201";
    static constexpr const char* APPLETANDCONTAINMENTWITHSAMEID = "W205";

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

};

typedef Error Warning;
typedef QList<Error> ErrorsList;
typedef QList<Warning> WarningsList;

}
}

Q_DECLARE_METATYPE(Latte::Data::Error)
Q_DECLARE_METATYPE(Latte::Data::ErrorsList)

#endif
