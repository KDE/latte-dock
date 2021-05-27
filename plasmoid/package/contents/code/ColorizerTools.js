/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

function colorBrightness(color) {
    return colorBrightnessFromRGB(color.r * 255, color.g * 255, color.b * 255);
}

// formula for brightness according to:
// https://www.w3.org/TR/AERT/#color-contrast
function colorBrightnessFromRGB(r, g, b) {
    return (r * 299 + g * 587 + b * 114) / 1000
}

function colorLuminas(color) {
    return colorLuminasFromRGB(color.r, color.g, color.b)
}

// formula for luminance according to:
// https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
function colorLuminasFromRGB(r, g, b) {
    var rS =  (r <= 0.03928) ? ( r / 12.92) : Math.pow( ((r + 0.055) / 1.055), 2.4 );
    var gS =  (g <= 0.03928) ? ( g / 12.92) : Math.pow( ((g + 0.055) / 1.055), 2.4 );
    var bS =  (b <= 0.03928) ? ( b / 12.92) : Math.pow( ((b + 0.055) / 1.055), 2.4 );

    return 0.2126*rS + 0.7152*gS + 0.0722*bS;
}
