New features/fixes that are found only in master in contrast with the current stable version

#### Version (master) - 11/12/2018

* new Coloring mechanism. The user can now choose for its Latte docks/panels classic Plasma theme style/Reversed Colors from plasma theme and Smart that takes into account the underlying desktop background
* Coloring mechanism can now use also the windows color schemes in order to paint properly its contents. User can choose from None/Active window/Touching window styles.
* dynamic background can now identify distorted backgrounds in order to inform the containment
to draw proper contrasted panel background underneath
* support new Advanced mode for View settings that fills all screen height
* Tasks left click and hover event gain their own options in order for the user to choose
their behavior
* support Plasma 5.15 new Virtual Desktops interface
* support fill(s) applets for Left,Center,Right alignment properly. That means that the user can now add plasma taskmanagers in Latte panel/docks and use at the same time the previous alignments.
* drag/maximize/restore active window from panel empty areas
* add a new Communicator item in order to handle all communications between containment and its applets
* support new painting based on active window color scheme
* user option to define which panel/dock will have the highest priority for global shortcuts
* user option to define per panel/dock if unified global shortcuts will be used for applets or not
* user option to scale up/dock the dock settings window through Meta+Wheel at its empty areas
* identify automatically which unified global shortcuts have defined by the user
* identify automatically which plasma style have been assigned to applets and show them also when the shortcuts badges are requested
* enforce specific ratio for dock settings window in order to look the same between different fonts
* user option to forward Meta event to Latte in order to show application launcher
* add a new Meta badge to identify the application launcher that will be triggered from the Meta event


* fix: dock settings visual inconsistencies between different pages
* fix: Show timer works now with KWin edge behavior
