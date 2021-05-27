/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tools.h"

// Qt
#include <QtMath>

namespace Latte{

Tools::Tools(QObject *parent)
    : QObject(parent)
{
}

float Tools::colorBrightness(QColor color)
{
    return colorBrightness(color.red(), color.green(), color.blue());
}

float Tools::colorBrightness(QRgb rgb)
{
    return colorBrightness(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

float Tools::colorBrightness(float r, float g, float b)
{
    float brightness = (r * 299 + g * 587 + b * 114) / 1000;

    return brightness;
}

float Tools::colorLumina(QRgb rgb)
{
    float r = (float)(qRed(rgb)) / 255;
    float g = (float)(qGreen(rgb)) / 255;
    float b = (float)(qBlue(rgb)) / 255;

    return colorLumina(r, g, b);
}

float Tools::colorLumina(QColor color)
{
    return colorLumina(color.redF(), color.greenF(), color.blueF());
}

float Tools::colorLumina(float r, float g, float b)
{
    // formula for luminance according to:
    // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef

    float rS = (r <= 0.03928 ? r / 12.92 : qPow(((r + 0.055) / 1.055), 2.4));
    float gS = (g <= 0.03928 ? g / 12.92 : qPow(((g + 0.055) / 1.055), 2.4));
    float bS = (b <= 0.03928 ? b / 12.92 : qPow(((b + 0.055) / 1.055), 2.4));

    float luminosity = 0.2126 * rS + 0.7152 * gS + 0.0722 * bS;

    return luminosity;
}

}
