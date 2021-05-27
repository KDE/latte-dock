/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


//! bound the preferred value between minimum and maximum boundaries
function bound(min, pref, max)
{
    return Math.max(min, Math.min(pref, max));
}
