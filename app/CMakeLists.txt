set(lattedock-app_SRCS
    ../liblatte2/commontools.cpp
    ../liblatte2/types.cpp
    alternativeshelper.cpp
    infoview.cpp
    lattecorona.cpp
    screenpool.cpp
    main.cpp
)

add_subdirectory(indicator)
add_subdirectory(layout)
add_subdirectory(layouts)
add_subdirectory(package)
add_subdirectory(plasma/extended)
add_subdirectory(settings)
add_subdirectory(settings/controllers)
add_subdirectory(settings/data)
add_subdirectory(settings/delegates)
add_subdirectory(settings/models)
add_subdirectory(settings/tools)
add_subdirectory(shortcuts)
add_subdirectory(view)
add_subdirectory(view/helpers)
add_subdirectory(view/indicator)
add_subdirectory(view/settings)
add_subdirectory(view/windowstracker)
add_subdirectory(wm)
add_subdirectory(wm/tracker)

set(latte_dbusXML dbus/org.kde.LatteDock.xml)
qt5_add_dbus_adaptor(lattedock-app_SRCS ${latte_dbusXML} lattecorona.h Latte::Corona lattedockadaptor)
ki18n_wrap_ui(lattedock-app_SRCS settings/settingsdialog.ui)

add_executable(latte-dock ${lattedock-app_SRCS})

include(FakeTarget.cmake)

if(${KF5_VERSION_MINOR} LESS "62")
    target_link_libraries(latte-dock
        Qt5::DBus
        Qt5::Quick
        Qt5::Qml
        KF5::Activities
        KF5::Archive
        KF5::CoreAddons
        KF5::Crash
        KF5::DBusAddons
        KF5::Declarative
        KF5::GuiAddons
        KF5::GlobalAccel
        KF5::I18n
        KF5::IconThemes
        KF5::KIOWidgets
        KF5::Notifications
        KF5::NewStuff
        KF5::QuickAddons
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::ProcessCore
        KF5::WaylandClient
        KF5::XmlGui
    )
else()
    target_link_libraries(latte-dock
        Qt5::DBus
        Qt5::Quick
        Qt5::Qml
        KF5::Activities
        KF5::Archive
        KF5::CoreAddons
        KF5::Crash
        KF5::DBusAddons
        KF5::Declarative
        KF5::GuiAddons
        KF5::GlobalAccel
        KF5::I18n
        KF5::IconThemes
        KF5::KIOWidgets
        KF5::Notifications
        KF5::NewStuff
        KF5::QuickAddons
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::WaylandClient
        KF5::XmlGui
    )

endif()

if(HAVE_X11)
    target_link_libraries(latte-dock
        Qt5::X11Extras
        KF5::WindowSystem
        ${X11_LIBRARIES}
        ${XCB_LIBRARIES}
    )
endif()

configure_file(org.kde.latte-dock.desktop.cmake org.kde.latte-dock.desktop)
configure_file(org.kde.latte-dock.appdata.xml.cmake org.kde.latte-dock.appdata.xml)

install(TARGETS latte-dock ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.latte-dock.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.latte-dock.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES dbus/org.kde.LatteDock.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES lattedock.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
install(FILES latte-layouts.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
install(FILES latte-indicators.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})

add_subdirectory(packageplugins)
