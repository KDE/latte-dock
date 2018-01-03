configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-latte-lib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-latte-lib.h)

set(lattedock_SRCS
    lattedockplugin.cpp
    quickwindowsystem.cpp
    dock.cpp
    iconitem.cpp
)

add_library(lattedockplugin SHARED ${lattedock_SRCS})

target_link_libraries(lattedockplugin
    Qt5::Quick
    Qt5::Qml
    KF5::CoreAddons
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::QuickAddons
    KF5::IconThemes
)

if(HAVE_X11)
    target_link_libraries(lattedockplugin KF5::WindowSystem)
endif()

install(TARGETS lattedockplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte)
