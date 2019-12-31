New features/fixes that are found only in master in contrast with the current stable version

#### Version (master) - 31/12/2019

* Floating panels/docks, the user can have a gap between the screen edge and the dock/panel
* the user can choose to override the real window geometry that is applied to floating docks/panels
when the view is behaving like a plasma panel; and always Fitt's Law in such case. In that case the
user can always use the gap between the dock and the screen edge to activate applets or to drag
the active window
* Indicators infrastructure is now much smarter. Recreating views is now applied only when the updated
indicator is used by that specific view; all other views are not influenced
