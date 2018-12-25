configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-latte-lib.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-latte-lib.h)

set(latteplugin_SRCS
    latteplugin.cpp
    backgroundtracker.cpp
    commontools.cpp
    iconitem.cpp
    quickwindowsystem.cpp
    types.cpp
    plasma/extended/backgroundcache.cpp
    plasma/extended/screenpool.cpp
)

add_library(latte2plugin SHARED ${latteplugin_SRCS})

target_link_libraries(latte2plugin
    Qt5::Quick
    Qt5::Qml
    KF5::CoreAddons
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::QuickAddons
    KF5::IconThemes
)

if(HAVE_X11)
    target_link_libraries(latte2plugin KF5::WindowSystem)
endif()

install(TARGETS latte2plugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte)

install(DIRECTORY qml/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte)
