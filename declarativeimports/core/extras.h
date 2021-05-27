/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXTRAS_H
#define EXTRAS_H

// local
#include <config-latte-lib.h>

// C++
#include <type_traits>
#include <numeric>
#include <memory>
#include <cmath>

// Qt
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QRect>
#include <QMetaEnum>
#include <QMetaType>

// Plasma
#include <Plasma>

//! There are gcc versions that don't support yet that function even though they
//! publish themselves as C++14 compatible. Such a case is gcc 4.8.x that openSUSE
//! LEAP 42.2-3 is using. By enabling this flag such systems can be build correctly.
#if ENABLE_MAKE_UNIQUE
namespace std {
template<class T, class... Args>
unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#endif

/*!
 * @brief convert a QRect to a QString with format `(<x>, <y>) <width>x<height>`
 */
inline QString qRectToStr(const QRect &r)
{
    return "(" % QString::number(r.x()) % ", " % QString::number(r.y()) % ") "
           % QString::number(r.width()) % "x" % QString::number(r.height());
}

/*!
 * @brief convert a `Q_ENUM` to c-string
 */
template<typename EnumType>
inline const char *qEnumToStr(EnumType value)
{
    return QMetaEnum::fromType<EnumType>().valueToKey(value);
}

/*!
 * @brief convert a `Q_ENUMS` of `Plasma::Types::Location` to c-string
 */
inline const char *qEnumToStr(Plasma::Types::Location Enum)
{
    static const int Index = Plasma::Types::staticMetaObject.indexOfEnumerator("Location");
    return Plasma::Types::staticMetaObject.enumerator(Index).valueToKey(Enum);
}

/*!
 * @brief convert a `Q_ENUMS` of `Plasma::Types::FormFactor` to c-string
 */
inline const char *qEnumToStr(Plasma::Types::FormFactor Enum)
{
    static const int Index = Plasma::Types::staticMetaObject.indexOfEnumerator("FormFactor");
    return Plasma::Types::staticMetaObject.enumerator(Index).valueToKey(Enum);
}

/*!
 * @brief machine epsilon
 */
template<class T>
typename std::enable_if < !std::is_integral<T>(), bool >::type almost_equal(T x, T y, int ulp)
{
    return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
           || std::abs(x - y) < std::numeric_limits<T>::min();
}

#endif // EXTRAS_H
