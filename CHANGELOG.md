#**CHANGELOG**#

#### Version 0.10.8
* multi-screen: fix docks/panels screen repositioning when the user specifies different screen for dock or panel
* fix borders identification for autopositioning vertical docks/panels
* fix vertical docks/panels autopositioning which is relevant to top and bottom panels
* position kwin edges helper window properly on startup after offscreen positioning
* x11: center applets config window on screen
* do not shrink vertical docks/panels on startup after switching from offscreen to onscreen
* make dock and panel work properly when they undo their removal

#### Version 0.10.7
* multi-screen: always trust KWin under X11 in order to set correct struts for panels and docks
* automatically offset centered applets when the left or right widgets overlap the centered one
* windowsgobelow: show properly during startup
* kwinedges: calculate accurately their size and position
* improve applets dragging during rearrange and configure widgets mode
* hide thin tooltips properly at some cases that were stack
* fill one pixel gap of justify splitters during rearrange and configure widgets mode
* fix, windows belong to screen whenever they are touching it. This way a window could belong in more than one screens and be taken into account for visibility modes and active window tracking
* remove file extensions for layouts and templates properly in order to identify the layout or template name
* fix build for Freebsd
* x11: force OnAllDesktops for all docks and panels during creation
* fix empty availableScreenRegion broadcast to Plasma

#### Version 0.10.6
* fix build with Qt 5.12
* initialize windows tracking properly

#### Version 0.10.5
* improve startup behavior and reduce visual glitches whenever a dock or panel is first appearing
* prepare code for Plasma LookNFeel packages
* improve applets dragging behavior when changing their position in a dock or panel
* ignore window states when kwin window manager is showing desktop effect
* reposition vertical docks and panels when surrounding docks and panels are changing their maximum length independent of their visibility mode
* reduce/increase maximum length through canvas ruler even when the minimum length equals the maximum length
* do not update kwin borderless windows behavior when it is not really changed for the current running layouts structure
* copied and duplicated docks or panels should not have AllBorders background enabled with no reason
* ignore maxlength that equals zero for fillLength applets as Qt already doing
* support 10bit systems under x11 and not show only a blurred area in that case
* multiscreen: disable struts under x11 for docks or panels that stand between two or more different screens
* tasks: reassign launchers group properly when a dock or panel view is recreated for any of reasons
* tasks: identify vivaldi audio stream properly and show the audio badge in that case
* cmd: support --add-dock through command line even when application is not already running
* cmd: provide option to print all available templates
* cmd: provide option to enable/disable application autostart

#### Version 0.10.4
* Feature Indicators: extend Indicators API in order for indicators to be able to animate their parent icon including when a task launcher is activated
* Feature Indicators: expose more Icon properties to indicators
* Important: Fix 25secs startup freezes from QDBusInterface desktop geometry calls
* Important: Fix startup delays because KWin was reconfiguring even though it was needed when BorderlessMaximized windows were activated
* Important: Remove plasma workarounds that were hiding plasma desktop bug #445495
* wayland: hide black line in the dock/panel center for AlwaysVisible visibility mode
* LastActiveWindow: update last activated window properties when changed
* initialize VirtualDesktopsWrappingAround property in a proper way
* plasmoid: decouple bouncing launcher animation for task real removal animation
* fix binding loops for Indexer qml ability
* respect applets maximumLength when equals zero

#### Version 0.10.3
* support GlobalScale in combination with PLASMA_USE_QT_SCALING properly under X11 environment
* add CornerMargin option for latte and plasma indicators and expose it through the indicators api
* unblock visibility mode properly when Meta is used to show an application launcher such as Win11, Simple menu etc.
* fix focus behavior when applets are requesting input such as knotes applet
* expose indicators iconOffsetX/Y value to applets
* enable/disable "CanBeAboveFullscreenWindows" option properly
* disable GtkFrameExtents for docks and panels that ByPassWindowManager ("CanBeAboveFullscreenWindows" option) under X11
* draw always a contrasted border for latte indicator
* simplify latte indicator implementation
* enforce RoundToIconSize for all applets always and as such the Items Size is always respected. If the user has blur issues with its icons, he should specify an items size that is provided by its icons theme. For example, 16px., 24px., 36px., 48px.
* identify kickofflegacy applet properly
* fix popup positioning for plasma-style popups when the dock background is using very big corner roundness
* prevent session manager from restoring latte just like Spectacle is already doing
* respect virtual desktops navigation wrapping around option
* expose translations for default dock and panel templates

#### Version 0.10.2
* fix crash from containmentactions loading after kde frameworks 5.86
* Fitt's Law fix for vertical panels in justify alignment
* Fitt's Law fix for applets touching the screen edge and at the same time using parabolic effect
* hide all context menu actions if the user has chosen it
* add missing translations for docks/panels dialog

#### Version 0.10.1
* fix autostart crash: do not show settings dialog too early
* fix build for Fedora
* specify product name in KAboutData to receive bug reports correctly at kde bug tracker

#### Version 0.10.0
* 2200 commits after version 0.9
* plenty of bug fixes and improvements
* Highlights:
  -- multiple docks and panels on the same screen edge
  -- floating docks and panels
  -- support background radius and background shadow size
  -- ten different visibility modes
  -- ondemand sidebars support
  -- inform Plasma Desktop about panels and docks geometries (since plasma 5.18)
  -- inform window managers about docks visible area (GTK_FRAME_EXTENTS support)
  -- provide internal Widgets Explorer dialog and thus being able to be used completely in other desktop environments such as GNOME and XFCE
  -- support multiple Latte Tasks in the same dock or panel
  -- improve applets positioning for Justify alignment in latte panels
  -- support latte centric applets that can use parabolic effect easily
  -- support Plasma Margins Area Separators
  -- user can specify custom color scheme per layout
  -- redesign and improve all layouts dialogs
  -- use templates for all layout functionality
  -- provide move/copy/paste functionality for docks and panels
  -- export layouts and docks/panels as templates for public use

#### Version 0.9.11

* fix context menu for classic systray items such as Viber and Telegram
* fix compatibility with KDE Frameworks 5.38

#### Version 0.9.10

* Dodge All Windows: work properly after the dock becomes hidden and not show inappropriately when any window touches the dock
* fix mask calculations for NO COMPOSITING environment
* fix blur for Latte panels when exiting edit mode
* protect LastActiveWindow application data updating in multi-screen environments and dont assign faulty application name and icons to irrelevant last active windows
* fix right click context menu for Group plasmoid. Take note that Group plasmoid can only be used in Single Layout mode and NOT in Multiple Layouts mode. It is scheduled to be fixed in next major stable version
* fix Indicators positioning when a Tasks is bouncing or requires attention
* x11: smart way to ignore all plasma popups
* wayland: smart way to ignore all plasma popups but first Plasma needs to update its popups flags in order to work correctly
* Support Chromium/Chrome audio channels for Tasks previews and context menu

#### Version 0.9.9

* CRITICAL: initialize properly configuration files during startup [kde#417886]
* IMPORTANT: Do not load a Shared Layout as Central when it is already loaded and as such do not mess the MultipleLayouts appearance when a Shared layout should also be used [kde#417886]
* [wayland] do not crash when right clicking Tasks plasmoid [kde#417546]
* update animations speed to support plasma 5.18 new animation speed values
* do not double release dock visibility after Meta application launcher triggering [kde#417239]
* improve blur region calculations in general and do not send invalid areas to kwin [kde#417254]
* improve blur area calculations during startup when parabolic effect is disabled [kde#416928]
* show Tasks icon size properly during startup when parabolic effect is disabled

#### Version 0.9.8.1

* fix availableScreenRegionWithCriteria calculations. A commit from master branch was lost even though it should be inside initial 0.9.8 release

#### Version 0.9.8

* provide new way to set which application launcher in all docks/panels has the highest priority to trigger with Meta. The one having a global shortcut applied is the one that has the highest priority
* consider plasma panels under x11 environment in order for dock settings window to not overlap with them
* fix which Plasma theme colors are used for all Latte painting mechanisms and make them consistent with Plasma
* Use KDE frameworks official Help Menu
* Provide KDE frameworks official way to set application's language
* add hidden debug option for "--kwinedges"
* paint properly the dock settings window external shadows
* fix margins/padding for applets that must follow Fitt's Law at the thick screen edge and at the same time be consisten with all surrounding applets
* add new LastActiveWindow APIs for window properties Closable/Minimizable/Maximizable etc. and provide them to applets. Applet Window Buttons applet is already using it in order to identify buttons that should not be drawn for specific windows
* add availableScreenRegion calculations for Left and Right screen edge docks/panels in order to be ready for new Plasma 5.18 API that will let us expose to plasma what are the free areas that are not occupied by Latte panels/docks
* fix wayland crash when showing dock settings window
* improve kwin workarounds in order to reapply properly docks/panels activities to them when kwin faulty is losting them

#### Version 0.9.7

* fix built with qt 5.9 [kde#415715]

#### Version 0.9.6

* qt5.14 - restore properly the dragged tasks to normal mode [kde#415333]
* qt5.14 - release properly the edit mode animation state [kde#412940]
* do not wait for dock to show in order to activate based on position global shortcuts. The new approach is smart enough in order to identify applets with popups in order to wait for them to slide-in first. [kde#415417]
* dynamic touching of views enabled isBusy state for them only when they are really touching. If one of them is hidden from its visibility mode then isBusy states should not be applied [kde#415347]
* wayland: fix crash from unavailable windows
* set a minimum length of screen edge activation area to 25% of the entire screen length this view is attached at.
* fix crash from unloading views by disconnecting their sensitive signals early in the chain
* show proper tooltip for installed indicators in Effects page
* update panelshadows to latest plasma code
* improve behavior of the AutomaticItemSizer
* improve publish tasks geometries code
* accept only left click as valid action when clicking neutral areas of applets, that is areas that even though the visually belong to the applet, the applet does not have any access to it.
* do not require a window to intersect with the view in order to be considered maximized
* consider windows as maximized only when both MaxHorizontally and MaxVertically are both applied
* remove various deprecated code

#### Version 0.9.5

* improve dynamic background animated transitions [Adrien Brunelat]
* fix endless growing/shrinking for items size by introducing an AutomaticItemSizer class that is tracking the algorithm predictions and results in order to identify the issue/case
* improve paddings and margins for applets that are touching the screen edge and at the same time follow Fitt's Law
* remove workarounds for margins and paddings for plasma systray
* dont crash when changing layouts by unloading Latte::View(s) first and their Plasma::Containment(s) afterwards
* dont drag/maximize windows from different virtual desktops than the current one
* fixes for dynamic background: force transparency codepath
* ignore plasma panels for Latte heuristics. Any plasma window touching the screen edge and having thickness <=96px is considered a plasma panel.
* update application data after 1500ms after their first fetching to catch up with some applications that are changing them with delay such as libreoffice
* remove some deprecated code for qt>=5.14
* drop ksysguard dependency with kde frameworks >=5.62

#### Version 0.9.4

* import and a load layout when Latte is running through its dbus interface
* expose the last active window colorscheme in order to be used from applets. In upcoming Window AppMenu version the user will be able to define the color scheme to be used for its app menus
* fix advanced switch button when changing between different states
* fix autostart option for some distros that did not work such as Manjaro

#### Version 0.9.3

* important: plenty of fixes for margins and Fitt's Law
* support android click animation even for panel empty areas
* latte indicators can now support animations for panel empty areas
* track kwinrc changes only when needed
* latte plasmoid is now using the last used activity from its layouts instead of the plasma current one. In that way tasks animations are improved under multiple layouts environments
* fix for all screens last active window tracking
* fix automatic icon size calculations in order to avoid constant cpu usage
* update some qt deprecated code

#### Version 0.9.2

* do not hide contents/icons when qtquick software rendering is used
* reverse scrolling direction for desktops and activities through empty areas
* after dragging active windows send a leave event and restore this way applets in normal state
* close multiple windows from previews when using middle-click
* activate single windows directly with left click in non compositing mode, and do not show the preview window in that case
* send tasks progress information to latte indicators
* latte indicators can offset their icons if they want
* latte indicators can provide different length padding values for applets compared to tasks
* autostart Latte earlier in order to catch up with windows global menu activation. You need to reactivate it in order to work.
* forced solidness for panels has higher priority compared to panel backgrounds in isBusy state
* disable panel shadow if the user has enabled the corresponding option
* do not draw the panel background outline if the plasma default behavior was chosen for popups
* do not draw progress badge if user has disabled it
* support struts with thickness < 24px.
* fixes for Clang

#### Version 0.9.1

* improve: when preview windows left click action is used then for single windows is just activating them without triggering the preview window
* improve: consider the case when a horizontal and a vertical Latte dock/panel are touching each other and the vertical one is isBusy desktop background case, in such case the horizontal view is also changing to isBusy state
* fix: blurred icons for items size that should not be blurred e.g. 48px and >=64px
* fix: geometries calculation under !compositing environment
* fix: forward pressed/released event to applets even when parabolic effect is enabled. The issue was pretty obvious with lock/logout plasma applet
* fix: update progress badge properly
* fix: tasks icons pixelization when are dragged
* fix: wayland, show preview window for grouped tasks when clicked


#### Version 0.9.0

* Smart Coloring
  --maximum contrast with desktop background when needed
  --use active or touching window color scheme to paint dock/panel contents
  --use reverse colors from plasma theme, meaning dark plasma themes can provide also whitish docks/panels
* Self-packaged Indicators that can be installed from kde store
* Live Editing Mode to inspect your settings changes immediately
* Flexible window layout for settings
* Shared Layouts under Multiple Layouts Environments
* Improve badges experience and layout
* Enhanced Active Window experience, drag/maximize/restore active window from empty areas
* Track "LastActiveWindow" at per screen/activity and inform applets
* Support scrolling for Latte Tasks plasmoid
* Independent multi-screen dynamic background and identify "busy" backgrounds
* Outline option for background
* plenty more fixes and improvements all over the place

#### Version 0.8.9

* fix: show notifications applet when in Latte (for plasma >= 5.16)

#### Version 0.8.8

* fix: multi-screen, unload properly explicit screen docks when its screen
is not available any more

#### Version 0.8.7

* fix: Show dock properly on first startup. New users where trying Latte but
it was reported that something broke during updates and on first startup
Latte was not appearing at all. Problem was tracked down and identified
when ~/.config/latte directory was not created properly. This is fixed now.

#### Version 0.8.6

* fix: previews that broke after kf5>=5.55 upgrade
* fix: plasma shortcuts behavior for applets when "Multiple" layouts are used

#### Version 0.8.5

* FIX: important improvements for fillWidth(s)/Height(s) applets. Latte now
tries to use plasma panels as an example in order to provide very similar experience
with its Latte panels
* FIX: adjust Latte taskmanager in order to support new Plasma 5.15 Virtual Desktops interface
* improve: splitters positioning during startup for Justify alignment
* improve: --replace option in order to restart Latte properly for all systems
* fix: maximum length ruler behavior for Latte panels
* fix: create autostart folder when missing from user folder

#### Version 0.8.4

* FIX: restore mouse wheel action to activate your tasks that broke with v0.8.3
* FIX: support fillWidth(s)/Height(s) applets in Left/Center/Right alignments,
add a plasma taskmanager to see what happens
* FIX: do not break applets order in Justify alignment when some of
the applets in the layout are not found in the system
* fix: a crash that was related to grouped tasks
* fix: improve launchers synchronization between different docks/panels

#### Version 0.8.3

* FIX: support multi-screen plasmoids that use plasmoid.screenGeometry such
as plasma pager, plasma taskmanagers etc. Latte did not update the
plasmoid.screenGeometry value properly in previous versions
* FIX: do not crash when moving launchers that are being synced between
multiple docks/panels
* FIX: make sure that launchers order between synced docks/panels is always
the current one after the user has ended its dragging
* FIX: support fillWidth/Height plasmoids better (such as plasma taskmanagers),
now such applets can be added for all alignments including Left/Center/Right
* FIX: do not show the warning message "Your layout file is broken" when the
statement is not valid. This check validates that the containments and applets
ids are unique in a layout file but the way this was implemented in the past it was
returning false results in some cases


#### Version 0.8.2

* FIX: wrong placement of docks during startup for multi-screen environments
* FIX: show explicit docks automatically when their corresponding screen is added in a multi-screen environment
* fix: open files properly when dropping them on launchers
* fix: improve behavior according to Fitt's Law when shrinking panel margins
* fix: dont hide previews when hovering player buttons
* fix: update delete icons to plasma design
* fix: dont break BorderlessMaximized window default value

#### Version 0.8.1

* FIX: redesign the multi-screens implementation. OnPrimary docks have always higher priority in multi-screen environments
* fix: do not move explicit dock on primary screen
* fix: consider "not show background" case as full transparency
* fix: consider preferredSize(s) only for >0 values (case of locked analog clock disappearing at a vertical panel)
* fix: if there is not any active window at all, dodge set docks should reappear
* fix: do not crash in wayland when right clicking with touchpad
* fix: do not double paint plama theme background when the theme does not contain a shadow
* fix: draw properly plasma theme panel backgrounds based on the edge that the dock or panel is present, e.g. Unity Ambiance, Nilium
* fix: identify maximized window screen differently
* fix: show grouped windows previews properly (follow plasma design for window previews)
* fix: place correctly a new copied dock in a multi-screen environment
* fix: enable blur for solid theme panels
* fix: missing global shortcuts '9' badge
* fix: support unified or not global shortcuts in case the user does not want to use the Latte unified system for its applets

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
