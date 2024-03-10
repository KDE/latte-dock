configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-latte-lib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-latte-lib.h)

set(lattecoreplugin_SRCS
    lattecoreplugin.cpp
    dialog.cpp
    environment.cpp
    iconitem.cpp
    quickwindowsystem.cpp
    tools.cpp
    types.h
)

add_library(lattecoreplugin SHARED ${lattecoreplugin_SRCS})

target_link_libraries(lattecoreplugin
    Qt::Quick
    Qt::Qml
    KF6::CoreAddons
    KF6::IconThemes
    Plasma
    PlasmaQuick
)

if(HAVE_X11)
    target_link_libraries(lattecoreplugin KF6::WindowSystem)
endif()

install(TARGETS lattecoreplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/core)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/core)
