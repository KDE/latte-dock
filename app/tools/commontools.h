/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

// std
#include <functional>

// Qt
#include <QColor>
#include <QRect>
#include <QString>

namespace Latte {

float colorBrightness(QColor color);
float colorBrightness(QRgb rgb);
float colorBrightness(float r, float g, float b);

float colorLumina(QColor color);
float colorLumina(QRgb rgb);
float colorLumina(float r, float g, float b);

QString rectToString(const QRect &rect);
QRect stringToRect(const QString &str);

//! returns the standard path found that contains the subPath
//! local paths have higher priority by default
QString standardPath(QString subPath, bool localFirst = true);

QString configPath();

// Predicates.
// If this grows out of proportions, consider moving this out of here.
template <typename... Inputs>
class Predicate {
public:
    Predicate(const std::function<bool(Inputs...)>& p)  : m_pred(p) {}
    Predicate(const std::function<bool(Inputs...)>&& p) : m_pred(std::move(p)) {}
    std::function<bool(Inputs...)> toFunction() { return m_pred; }

    bool operator()(Inputs&...);
    Predicate operator|(const Predicate&);
    Predicate operator&(const Predicate&);
    Predicate operator^(const Predicate&);
    Predicate operator~();

private:
    const std::function<bool(Inputs...)> m_pred;
};

template <typename... Inputs>
class PredicateList : public QList<Predicate<Inputs...>> {
public:
    using QList<Predicate<Inputs...>>::QList;

    Predicate<Inputs...> all();
    Predicate<Inputs...> any();
    Predicate<Inputs...> none();
};
}

#endif
