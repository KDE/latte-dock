/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
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
    static const int ONALLSCREENSID = -100;
    static const int ONALLSECONDARYSCREENSID = -101;
    static constexpr const char* ONPRIMARYNAME = "{primary-screen}";
    static constexpr const char* ONALLSCREENSNAME = "{all-screens}";
    static constexpr const char* ONALLSECONDARYSCREENSNAME = "{all-secondary-screens}";

    Screen();
    Screen(Screen &&o);
    Screen(const Screen &o);
    Screen(const QString &screenId, const QString &serialized);

    //! Screen data
    bool hasExplicitViews{false};
    bool isActive{false};
    bool isRemovable{false};
    bool isSelected{false};
    QRect geometry;

    //! Operators
    Screen &operator=(const Screen &rhs);
    Screen &operator=(Screen &&rhs);
    bool operator==(const Screen &rhs) const;
    bool operator!=(const Screen &rhs) const;

    bool isScreensGroup() const;

    void init(const QString &screenId, const QString &serialized);
    QString serialize() const;

};

typedef GenericTable<Screen> ScreensTable;

}
}

Q_DECLARE_METATYPE(Latte::Data::Screen)
Q_DECLARE_METATYPE(Latte::Data::ScreensTable)

#endif
