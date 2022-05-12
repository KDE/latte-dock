add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.latte.plasmoid\")

configure_file(metadata.json.in ${CMAKE_CURRENT_BINARY_DIR}/metadata.json @ONLY)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/plasmoids/org.kde.latte.plasmoid
)
plasma_install_package(package org.kde.latte.plasmoid)

set(tasks_SRCS
    plugin/types.cpp
    plugin/lattetasksplugin.cpp
)

add_library(lattetasksplugin SHARED ${tasks_SRCS})

target_link_libraries(lattetasksplugin
                      Qt5::Core
                      Qt5::Qml
                      KF5::Plasma
                      KF5::PlasmaQuick)
                      
install(TARGETS lattetasksplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/private/tasks)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/private/tasks)
