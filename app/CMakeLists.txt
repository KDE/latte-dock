set(lattedock-app_SRCS
    ../liblatte2/commontools.cpp
    ../liblatte2/types.cpp
    alternativeshelper.cpp
    globalshortcuts.cpp
    importer.cpp
    infoview.cpp
    lattecorona.cpp
    launcherssignals.cpp
    layoutmanager.cpp
    schemecolors.cpp
    screenpool.cpp
    layout/layout.cpp
    package/lattepackage.cpp
    plasma/extended/screenpool.cpp
    plasma/extended/theme.cpp
    settings/settingsdialog.cpp
    settings/universalsettings.cpp
    settings/delegates/activitycmbboxdelegate.cpp
    settings/delegates/checkboxdelegate.cpp
    settings/delegates/colorcmbboxdelegate.cpp
    settings/delegates/colorcmbboxitemdelegate.cpp
    settings/delegates/layoutnamedelegate.cpp
    view/contextmenu.cpp
    view/effects.cpp
    view/panelshadows.cpp
    view/positioner.cpp
    view/screenedgeghostwindow.cpp
    view/view.cpp
    view/visibilitymanager.cpp
    view/settings/primaryconfigview.cpp
    view/settings/secondaryconfigview.cpp
    wm/abstractwindowinterface.cpp
    wm/waylandinterface.cpp
    wm/windowinfowrap.cpp
    wm/xwindowinterface.cpp
    main.cpp
)

set(latte_dbusXML dbus/org.kde.LatteDock.xml)
qt5_add_dbus_adaptor(lattedock-app_SRCS ${latte_dbusXML} lattecorona.h Latte::Corona lattedockadaptor)
ki18n_wrap_ui(lattedock-app_SRCS settings/settingsdialog.ui)

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
