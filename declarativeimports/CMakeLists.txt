add_subdirectory(core)
install(DIRECTORY applet/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/applet)
install(DIRECTORY components/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/components)
install(DIRECTORY container/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/container)


