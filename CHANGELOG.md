#**CHANGELOG**#

#### Version 0.8.0

* Simultaneous Multiple Layouts in different Activities
* Smart dynamic Background
* Monochrome contents based on the underlying background
Activation Global Shortcuts are taken into account the surrounding applets and increased from nine to nineteen (Meta+1..9,0,z..>)
* New improved Edit Mode image patterns or User Set ones
* Download community-provided Latte layouts from Latte Settings window
* Beautiful Animations (e.g. changing alignment)
* New Maximum Length indicator at Edit Mode that you can change its value with the mouse wheel
* Multiple Separators between Tasks and Applets
* Borderless maximized windows per layout, the user can use that setting in order to replicate a Unity-style layout and a Plasma one at the same time.
* Lock/Unlock Layouts, different layouts can become read-only and writeable
* Ungroup tasks of the same application
* One-Click Option to change between Panel/Dock modes
* New Appearance Settings for Active Indicator and Glow
* Support big number of messages badge (<=9999), like the plasma taskmanager
* Expose Latte options in the context menu when plasma taskmanagers are used
* More command line options to handle Latte
* Various Wayland improvements. I use it daily in my system with Plasma 5.12 and it provides a fantastic experience with fantastic painting.
* Smoother parabolic animation
* Support Kwin edges behavior when hiding the dock or panel
* New improved splitters icons in Justify (Edit Mode)
* Improve the entire experience with Layouts/Latte Settings window
* Filter Windows by Launchers, show only windows that there is already a launcher present for the current running activity
* Vastly improve the experience in !compositing environments. No more showing an 1px line at the screen edge when the dock is hidden.
* New Global Shortcuts to open/hide dock settings and Latte settings (Meta+A, Meta+W, Meta+E)
* New Kwin script to trigger the application menu from a corner-edge
* Hide the audio badge when there no audio is coming from a pulseaudio stream
* various fixes for RTL languages
* New more robust animations all over the place
* Plenty of bug fixes and improvements all over the place


#### Version 0.7.5

* fix for dodge maximized in multi-screen environment
* when copying default layouts make sure they are writable in the destination
* new protocol to communicate between applets and Latte in order to
inform them when they are in a Latte panel/dock and when they dont want any change in their main icon behavior.

#### Version 0.7.4

* new improved image patterns for edit mode
* support v0.8 layout files in order to not break compatibility
* use an svg file for Latte trademark in settings window instead of a font
* fix for multi-screen environments
* fix parabolic effect in some corner cases
* import v0.6 launchers to v0.7 only once
* improve bouncing anchoring

#### Version 0.7.3

* support RTL languages
* fix crash occuring from badges
* improve shadow behavior and calculations for corner cases
* support "nomad systray" and "kdeconnect sms" in Latte heuristics
* expand applets when clicked at neutral areas
* shrink a bit the task number
* support Fitt's Law in more corner cases
* execute Latte at the same time between different users
* ignore X11 signals that are sent with no reason
    (e.g. Firefox 57 upstream bug: https://bugzilla.mozilla.org/show_bug.cgi?id=1389953)
* take into account the applets shadow size for mask calculations

#### Version 0.7.2

* fix crash introduced with qt 5.9.2 when the user hovers the dock
* highly improve the attention bouncing animation
* fix coloring for shortcut badges
* various fixes for animations and glitches
* hide internal tasks separator at the edges
* improvements for window manager !compositing window state
* pass kde review process
* move source to kde infrastructure

#### Version 0.7.1

* added “New” button in Layouts manager
* “Close” window from context menu was moved in the end
* provide always valid task geometries, fixes any lamb minimize/unminimize effect issues
* improve scroll wheel behavior, it is only used to show and activate windows and not minimizing them
* fix issue with Firefox 55 that was blocking the dock from showing
* improve combination or previews and highlight effect. (the user can now highlight windows from their previews)
* provide a previewsDelay which can be used from advanced users to lower the delay between showing previews or highlighting windows. Be careful, very low values dont provide correct previews. 150ms is by default the lowest value that is taken into account. The value must be added in the Latte plasmoid general settings in any layout file
* show correct icon when a single window is removed
* allow for 1px substitutions for applet sizes when in advanced mode and the user has disabled to automatic shrinking… This way for example you can have a Latte panel with size of 29px.
* Behavior for show background only for maximized windows now respects the applets shadows settings… concerning visibility, color, size etc…
* fix a crash when changing layouts from settings combobox


#### Version 0.7.0

* wayland tech preview
* dynamic layouts, different layouts per activities
* new layouts editor
* support copy, remove, presets, import, export for layouts
* basic/advanced mode for configuration window
* windows go below visibility mode
* title tooltips
* change dock offset, panel transparency, applets shadow, panel shadow
* dynamic background, show background only for maximized windows
* separators everywhere as applets and one special internal separator for tasks
* audio streams indicator, increase/decrease/mute volume
* different launchers groups, unique/global/layout to sync launchers between docks
* support applets that can fill all the free space of the dock
* support plasma taskmanagers in order to replace Latte plasmoid
* libunity support for progress indicators and counters
* global shortcuts for activating tasks,showing app launcher, show the hidden dock
* an all new dbus interface to show tasks counters
* a community thunderbird plugin using the new latte dbus interface
* switch completely to plasma libtaskmanager for launchers and tasks
* improve color handling for window different states
* add Widgets through the context menu
* places support from Plasma 5.10
* a special Latte spacer that its size can be set in pixels or in percentage according to the current Latte icon size
* active window indicator for window previews
* copy dock support, fantastic feature for multi-screen environments
* remember last active window in a tasks group and use the mouse wheel to cycle through a tasks group
* restart Latte in case of a crash
* overlay over plasma applets such as forlderview in order to provide correct parabolic effect (bug: showing problematic animations for icon sizes smaller of 64px)
* provide our own build-in active applet indicator in case the user wants to have uniformity across its docks
* move the lock button from the drag area into the tooltip
* disable automatic icon size shrinking when a plasma taskmanager is used
* improvements for parabolic effect and  various animations
* support import/export for layout independently and for the full configuration
* support automatic and manual importing from v0.6 architecture
* version 0.7 contains around 700 new commits comparing with the v0.6.x branch
* various small or big improvements/bug fixes etc 

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
