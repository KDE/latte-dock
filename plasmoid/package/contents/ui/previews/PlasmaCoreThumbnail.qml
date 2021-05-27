/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.WindowThumbnail {
    winId: thumbnailSourceItem.winId

    onWinIdChanged: {
        //! WORKAROUND, in order for toolTipDelegate to re-instantiate the previews model when
        //! previews are changing from single instance preview to another single instance
        visible = false;
        visible = true;
    }
}
