set(lattedock-app_SRCS
    ../liblattedock/dock.cpp
    windowinfowrap.cpp
    abstractwindowinterface.cpp
    xwindowinterface.cpp
    windowinfowrap.cpp
    visibilitymanager.cpp
    dockcorona.cpp
    dockview.cpp
    dockconfigview.cpp
    packageplugins/shell/dockpackage.cpp
    panelshadows.cpp
    alternativeshelper.cpp
    screenpool.cpp
    globalsettings.cpp
    main.cpp
)

set(latte_dbusXML dbus/org.kde.LatteDock.xml)
qt5_add_dbus_adaptor(lattedock-app_SRCS ${latte_dbusXML} dockcorona.h Latte::DockCorona lattedockadaptor)

add_executable(latte-dock ${lattedock-app_SRCS})

include(FakeTarget.cmake)

target_link_libraries(latte-dock
    Qt5::DBus
    Qt5::Quick
    Qt5::Qml
    KF5::I18n
    KF5::DBusAddons
    KF5::Declarative
    KF5::CoreAddons
    KF5::Archive
    KF5::XmlGui
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::Activities
    KF5::Notifications
    KF5::QuickAddons
    KF5::WaylandClient
)

if(HAVE_X11)
    target_link_libraries(latte-dock
        Qt5::X11Extras
        KF5::WindowSystem
        ${X11_LIBRARIES}
        ${XCB_LIBRARIES}
    )
endif()

configure_file(latte-dock.desktop.cmake latte-dock.desktop)

install(TARGETS latte-dock ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/latte-dock.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES dbus/org.kde.LatteDock.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES lattedock.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
