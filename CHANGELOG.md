#**CHANGELOG**#

#### Version 0.6.1

* fix #385,load font through QFontDatabase 
* fix #412,fix for groupDialog in plasma 5.8 
* expose action for remove Latte plasmoid in desktop 
* do not update iconSize automatically 
* return correct screen geometry from corona 
* fix #358, [Crash] - recreating dock for AlwaysVisible mode 
* fix #351,use correct screenGeometry at task filter 
* Update README.md 
* links for repos 
* fix no return in nonvoid function

#### Version 0.6.0

* fix #344, edit mode still active when I change the session
* fix #330, Visibility doesn't load when switch to session
* fix coloring for tasks group indicator
* fix #331, set onAllDesktops early in the process
* fix #336, launcher action a bit earlier
* fix #334, block dropping internal launchers on dock
* fix #333, restore previous functionality

* update list of contributors
* various improvements for task tooltips

#### Version 0.5.98 Beta

* fix #323, a better fix for hover effect crashes
* fix #323, preserve compatibility with plasma 5.8
* fix #323, TaskManagerBackend groupDialog
* fix #316, [Crash] Exporting settings and trying to open directory of the file
* fix #308, Always visible option does not work for side-set dock on unusual multi screen setup
* fix #292, workaround for KF5.32 empty mask
* fix #298, default add launchers in taskmanager
* fix #285, expanded to false for applets
* fix #262, dont hide preview window for buffer init
* fix #282, fix removal animation
* fix #281, enable/disable auto decrease applets size
* fix #277, use normal dock window from tweaks
* fix #275, top dock gains priority for AlwaysVisible
* fix #272, expose alternative session in menu
* fix #270, enable blur for panel background
* fix #258, protect removal phase
* fix #215, support a modifier action
* fix #266, expose middle click actions of libtask
* fix #260, hide tooltips if dock becomes hidden
* fix #264, introduce lastValidSourceName
* fix #256, Dodging/Layering Issues
* fixes for no compositing automatic icon size
* fix #259, user can set distance between applets
* fix #257, crash alternative session & !compositing
* fix crash deleting visibilityManager
* fix restore config for raiseOnDesktop/ActivityChange
* fix #246, Incorrect/inconsistent behaviour when switching virtual desktops
* fix #250, workaround hovering issue for applets
* fix #235, set icon for docks differently
* fix #248, option to shring thick margins to minimum
* fix #102, show apps menu with Super key
* fix #204, improve Latte clicking signaling
* fix #217, adaptive applet size based on screen
* fix #238, disable/enable raise dock temporary
* fix #233, support autostart through tweaks page
* fix #126, improve behavior for auto positioning
* fix #226, add applets correctly in Fill mode
* fix #194, the user can enable a solid background
* fix #191, use percentage to calculate panel size
* fix #188, fix launchers behavior for Plasma>=5.9
* fix #218, reenable garbage collect and trim cache
* fix #214, comment new multi-screen behavior
* fix #216, fix glitch for none animations
* fix right edge positioning
* fix typo for LeftEdge
* fix spacing at screen combo box
* fix #208, crash on exit through quit button
* fix #198, flag ByPassWindowManagerHint
* fix #12,multi-screen support
* support always on primary case
* drop dock primary screen behavior from plasma
* fix setting a dock's screen for config window
* fix config window positioning in multi-screens
* fix #116, update screen in screengeometry changes
* fix memory leak
* fix #197, add/remove task animation
* fix applets centering with new rendering
* fix #195, new direct rendering mechanism
* fix #187, counter for actions blockHiding
* fix #183, parentIndex wrong initialization in ToolTipDelegate2
* fix #186, drop shadowedImage and use Latte IconItem
* fix #185, changes for previews broke hover behavior
* fix #178, fix grid layout for group tasks
* fix #177, protect containment clearZoom in previews
* fix #176, dont hide tooltip when hovering same task
* fix #174, account screen size for the debug window
* fix #172, disable hidpi scaling
* fix #171, use lock file instead of QSharedMemory
* fix #171, allow only one instance for Latte app
* fix #48, Support Alternatives from the Context Menu
* fix #164, Dodge Active is broken for windows on all desktops
* fix #116, remove QueuedConnection
* fix #116, endless showing loop at startup
* fix #139, Un-hide Latte at desktop and activity changes
* fix #116, add protections in multi-screen
* fix #159, show correct previews for windows
* fix #133, dodge active window when switching desktop
* fix #141, dodge decoration
* fix #136, load visibility with a timer in startup
* fix #136, clean autoHidden
* fix #155, fix availableScreenRect of latte corona
* fix #42, latte icon shown correctly in ksysguard
* fix #154, crash on screenGeometry change
* fix #153, improve tasks progress visuals
* fix #148, improve tooltips focus
* fix #118, support dragging a file at windows group
* fix #138, import new tooltips from plasma 5.9
* fix #127, saving layouts in justify normal state
* fix #118, raise window for hovering files over task
* fix #128, reimplement the panel draw
* fix #103, fixes in calculations and orchestration
* fix #125, fix calculations for dock geometry
* fix #119, improve animations heuristics
* fix restoring splitters on startup
* fix #92, improve more three layouts for Fill
* fix issues with new maxLength
* fix Wrong license version
* fix #94, --debug must be set for debugging messages
* fix #93, masking updated correctly on config win
* fix #96, fix right click for systray
* fix #88, protect launchers from adding icon info
* fix #80, Dock do not dodge Spotify
* fix #74, hide settings button from plasmoid tooltip
* fix #85, removing Dock crashes Latte
* fix #84, closing Latte cleans config file
* fix two serious crashes from config window

* improved readability of CMakeLists
* removing intltool dependence and locale scripts improved
* #98, added about dialog
* support maxLength through config win and container
* rename Latte's configuration files #81
* #199, AbstractWindowInterface is now a shared resource
* #199, QQuickWindowSystem replacement of WindowSystem

* avoid overlaping
* improvements to no compositing
* added multi screen support
* added a lot information into debug window
* added support for alternative session
* added, #205 backup and restore
* adedd, #224 Added option to add spacers applets
* added german translation
* added zh_TW translations

#### Version 0.5.91 Alpha

* fix #54, dock is shown when applet needs attention
* fix #65, stop bouncing animation in hidden state
* fix #66, many automatic size issues
* fix #51, protect add task animation more
* fix #76, Latte should not allow removing all dock
* The task tooltip now shows all the borders
* INSTALLATION file added with instructions for Kubuntu and Arch Linux
* Update README.md
* Added rpm package for openSUSE
* Added spanish translation
* Translation strings improved #70

#### Version 0.5.90 Alpha

* First released
