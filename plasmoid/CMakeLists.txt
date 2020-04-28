add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.latte.plasmoid\")

configure_file(metadata.desktop.cmake ${CMAKE_CURRENT_SOURCE_DIR}/package/metadata.desktop)

plasma_install_package(package org.kde.latte.plasmoid)

set(tasks_SRCS
    plugin/dialog.cpp
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
