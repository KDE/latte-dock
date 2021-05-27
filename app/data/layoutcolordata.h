/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTCOLORDATA_H
#define LAYOUTCOLORDATA_H

//local
#include "genericdata.h"

//Qt
#include <QMetaType>
#include <QString>

namespace Latte {
namespace Data {

class LayoutColor : public Generic
{
public:
    LayoutColor();
    LayoutColor(LayoutColor &&o);
    LayoutColor(const LayoutColor &o);

    //! Color data
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
