include(ECMInstallIcons)

ecm_install_icons(ICONS
    16-apps-latte-dock.svg
    22-apps-latte-dock.svg
    24-apps-latte-dock.svg
    32-apps-latte-dock.svg
    48-apps-latte-dock.svg
    sc-apps-latte-dock.svg
THEME hicolor
DESTINATION ${ICON_INSTALL_DIR})

install(FILES org.kde.latte.plasmoid.svg DESTINATION ${ICON_INSTALL_DIR}/breeze/applets/256)
