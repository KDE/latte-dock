/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

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
}

#endif
