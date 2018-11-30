set(lattedock-app_SRCS
    ../liblattedock/dock.cpp
    packageplugins/shell/dockpackage.cpp
    abstractwindowinterface.cpp
    alternativeshelper.cpp
    commontools.cpp
    dockcorona.cpp
    globalshortcuts.cpp
    importer.cpp
    infoview.cpp
    launcherssignals.cpp
    layout.cpp
    layoutmanager.cpp
    plasmathemeextended.cpp
    schemecolors.cpp
    screenpool.cpp
    settingsdialog.cpp
    sortedactivitiesmodel.cpp
    universalsettings.cpp
    waylandinterface.cpp
    windowinfowrap.cpp
    xwindowinterface.cpp
    dock/dockconfigview.cpp
    dock/dockmenumanager.cpp
    dock/docksecconfigview.cpp
    dock/dockview.cpp
    dock/effects.cpp
    dock/panelshadows.cpp
    dock/positioner.cpp
    dock/screenedgeghostwindow.cpp
    dock/visibilitymanager.cpp
    layoutsDelegates/activitycmbboxdelegate.cpp
    layoutsDelegates/checkboxdelegate.cpp
    layoutsDelegates/colorcmbboxdelegate.cpp
    layoutsDelegates/colorcmbboxitemdelegate.cpp
    layoutsDelegates/layoutnamedelegate.cpp
    main.cpp
)

set(latte_dbusXML dbus/org.kde.LatteDock.xml)
qt5_add_dbus_adaptor(lattedock-app_SRCS ${latte_dbusXML} dockcorona.h Latte::DockCorona lattedockadaptor)
ki18n_wrap_ui(lattedock-app_SRCS settingsdialog.ui)

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
    KF5::GlobalAccel
    KF5::Archive
    KF5::Crash
    KF5::XmlGui
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::Activities
    KF5::Notifications
    KF5::NewStuff
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

configure_file(org.kde.latte-dock.desktop.cmake org.kde.latte-dock.desktop)
configure_file(org.kde.latte-dock.appdata.xml.cmake org.kde.latte-dock.appdata.xml)

install(TARGETS latte-dock ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.latte-dock.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.latte-dock.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES dbus/org.kde.LatteDock.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
install(FILES lattedock.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
install(FILES latte-layouts.knsrc DESTINATION  ${CONFIG_INSTALL_DIR})
