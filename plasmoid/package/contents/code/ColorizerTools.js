/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
