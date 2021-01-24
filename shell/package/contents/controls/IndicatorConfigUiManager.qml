/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7
import QtQuick.Layouts 1.3

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: uiManager
    visible: false

    property Item stackView: null

    //! it is used during first window creation in order to avoid clearing custom indicators from its views
    //! when the window is created the current view indicator type is stored and restored after
    //! the tabBar of indicators has completed its creation/initialization
    property string typeDuringCreation: ""

    Item {
        id: hiddenIndicatorPage
        anchors.fill: parent

        readonly property int optionsWidth: dialog.optionsWidth
        readonly property bool deprecatedOptionsAreHidden: true // @since 0.10.0

        readonly property Item nextPage: stackView.currentItem === page1 ? page2 : page1
        readonly property Item previousPage: nextPage === page1 ? page2 : page1

        function showNextIndicator() {
            var nextIndicator;

            //var pageShown = stackView.currentItem ? 1 : 0;
            //var total = page1.children.length + page2.children.length + hiddenPages.children.length + pageShown;
            //console.log(" >>>>>>>>>>>>>>>>> ALL PAGES :: " + total);

            if (children.length > 0) {
                nextIndicator = children[0];
            }

            if (nextIndicator && (!stackView.currentItem || !stackView.currentItem.isCurrent)) {
                //!empty nextPage by moving its pages into hiddenPages
                var childrenCount = nextPage.children.length;
                for (var i=0; i<childrenCount; ++i) {
                    var previousIndicator = nextPage.children[0];
                    previousIndicator.visible = false;
                    previousIndicator.parent = hiddenPages;
                }

                nextIndicator.parent = nextPage;
                nextIndicator.visible = true;
                stackView.Layout.minimumHeight = nextIndicator.height;
                nextPage.type = latteView.indicator.type;

                var currentIndex = -1;

                if (stackView.currentItem) {
                    currentIndex = viewConfig.indicatorUiManager.index(stackView.currentItem.type);
                }

                var nextIndex = viewConfig.indicatorUiManager.index(latteView.indicator.type);

                stackView.forwardSliding = (nextIndex<currentIndex);
                stackView.replace(stackView.currentItem, nextPage);
            }

            if (children.length>0) {
                //!empty all remaining pages by moving them into hiddenPages
                var tempcount = children.length;
                for (var i=0; i<tempcount; ++i) {
                    var tempIndicator = children[0];
                    tempIndicator.visible = false;
                    tempIndicator.parent = hiddenPages;
                }
            }
        }

        Component.onCompleted: {
            viewConfig.indicatorUiManager.setParentItem(hiddenIndicatorPage);
            tabBar.selectTab(latteView.indicator.type);

            if (latteView.indicator.type !== latteBtn.type) {
                uiManager.typeDuringCreation = latteView.indicator.type;
            }

            viewConfig.indicatorUiManager.ui(latteView.indicator.type, latteView);
        }

        Connections {
            target: latteView.indicator
            onPluginChanged: {
                if (viewConfig.isReady) {
                    if (uiManager.typeDuringCreation === "") {
                        tabBar.selectTab(latteView.indicator.type);
                        viewConfig.indicatorUiManager.ui(latteView.indicator.type, latteView);
                    } else {
                        //! restore the first assigned indicator after first window creation. This way we avoid
                        //! unsetting custom indicators from views during first settings window creation.
                        latteView.indicator.type = uiManager.typeDuringCreation;
                        tabBar.selectTab(latteView.indicator.type);
                        uiManager.typeDuringCreation = "";
                    }
                }
            }
        }

        Connections {
            target: viewConfig
            onIsReadyChanged: {
                if (viewConfig.isReady) {
                    tabBar.selectTab(latteView.indicator.type);
                    viewConfig.indicatorUiManager.ui(latteView.indicator.type, latteView);
                }
            }
        }
    }

    Item {
        id: hidden
        visible: false
        ColumnLayout {
            id: page1
            width: stackView.width
            readonly property bool isCurrent: latteView.indicator.type === type && viewConfig.isReady/*update flag*/
            readonly property bool deprecatedOptionsAreHidden: true // @since 0.10.0
            readonly property int optionsWidth: dialog.optionsWidth

            property string type: ""
        }

        ColumnLayout {
            id: page2
            width: stackView.width
            readonly property bool isCurrent: latteView.indicator.type === type && viewConfig.isReady/*update flag*/
            readonly property bool deprecatedOptionsAreHidden: true // @since 0.10.0
            readonly property int optionsWidth: dialog.optionsWidth

            property string type: ""
        }

        ColumnLayout {
            id: hiddenPages
            width: stackView.width
            readonly property bool isCurrent: latteView.indicator.type === type && viewConfig.isReady/*update flag*/
            readonly property bool deprecatedOptionsAreHidden: true // @since 0.10.0
            readonly property int optionsWidth: dialog.optionsWidth

            property string type: ""
        }
    }
}
