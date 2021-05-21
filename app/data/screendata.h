/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later

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
    static constexpr const char* ONPRIMARYNAME = "{primary-screen}";

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
