add_subdirectory(core)
install(DIRECTORY applets/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/applets)
install(DIRECTORY components/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/components)
install(DIRECTORY containers/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/containers)


