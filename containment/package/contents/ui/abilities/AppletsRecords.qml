/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0

Item {
    id: records

    property int windowsTrackingCount: 0 //! number of applets requesting windows tracking
    property real maxInnerZoomFactor: 1.0 //! maximum inner zoom factor based on all applets

    property var applets: []

    function indexOf(applet) {
        var len = applets.length;

        for (var i=0; i<len; ++i) {
            if (applets[i].id === applet) {
                return i;
            }
        }

        return -1;
    }

    function removeApplet(applet) {
        var ind = indexOf(applet);

        if (ind>=0) {
            //! remove
            applets.splice(ind, 1);
            updateValues();
        }
    }

    function addDefaultRecord(applet) {
        applets.push({id: applet, innerZoomFactor: 1.0, windowsTrackingEnabled: false});
    }

    function setInnerZoomFactor(applet, inner) {
        var len = applets.length;
        var ind = indexOf(applet);

        if (ind >=0) {
            //! update
            if(applets[ind].innerZoomFactor !== inner) {
                applets[ind].innerZoomFactor = inner;
                updateInnerZoomFactor();
            }
        } else {
            //! add
            addDefaultRecord(applet);
            applets[len].innerZoomFactor = inner;
            updateInnerZoomFactor();
        }
    }

    function setWindowsTrackingEnabled(applet, value) {
        var len = applets.length;
        var ind = indexOf(applet);

        if (ind>=0) {
            //! update
            if(applets[ind].windowsTrackingEnabled !== value) {
                applets[ind].windowsTrackingEnabled = value;
                updateWindowsTrackingCount();
            }
        } else {
            //! add
            addDefaultRecord(applet);
            applets[len].windowsTrackingEnabled = value;
            updateWindowsTrackingCount();
        }
    }

    function updateValues() {
        updateInnerZoomFactor();
        updateWindowsTrackingCount();
    }

    function updateInnerZoomFactor() {
        var len = applets.length;
        var max = 1;

        for (var i=0; i<len; ++i) {
            if (applets[i].innerZoomFactor > max) {
                max = applets[i].innerZoomFactor;
            }
        }

        maxInnerZoomFactor = max;
    }

    function updateWindowsTrackingCount() {
        var len = applets.length;
        var cnt = 0;

        for (var i=0; i<len; ++i) {
            if (applets[i].windowsTrackingEnabled) {
                cnt = cnt + 1;
            }
        }

        windowsTrackingCount = cnt;
    }
}
