add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.latte.plasmoid\")

plasma_install_package(package org.kde.latte.plasmoid)

set(tasks_SRCS
    plugin/types.cpp
    plugin/lattetasksplugin.cpp
)

add_library(lattetasksplugin SHARED ${tasks_SRCS})

target_link_libraries(lattetasksplugin
                      Qt::Core
                      Qt::Qml
                      KF6::Plasma
                      KF6::PlasmaQuick)

install(TARGETS lattetasksplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/private/tasks)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/private/tasks)
