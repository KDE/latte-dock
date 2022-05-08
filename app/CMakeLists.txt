set(lattedock-app_SRCS
    alternativeshelper.cpp
    apptypes.cpp
    infoview.cpp
    lattecorona.cpp
    screenpool.cpp
    primaryoutputwatcher.cpp
    main.cpp    
    coretypes.h
)

add_subdirectory(data)
add_subdirectory(declarativeimports)
add_subdirectory(indicator)
add_subdirectory(layout)
add_subdirectory(layouts)
add_subdirectory(package)
add_subdirectory(plasma/extended)
add_subdirectory(settings)
add_subdirectory(settings/generic)
add_subdirectory(settings/actionsdialog)
add_subdirectory(settings/detailsdialog)
add_subdirectory(settings/detailsdialog/delegates)
add_subdirectory(settings/exporttemplatedialog)
add_subdirectory(settings/exporttemplatedialog/delegates)
add_subdirectory(settings/screensdialog)
add_subdirectory(settings/screensdialog/delegates)
add_subdirectory(settings/settingsdialog)
add_subdirectory(settings/settingsdialog/delegates)
add_subdirectory(settings/viewsdialog)
add_subdirectory(settings/viewsdialog/delegates)
add_subdirectory(shortcuts)
add_subdirectory(templates)
add_subdirectory(tools)
add_subdirectory(view)
add_subdirectory(view/helpers)
add_subdirectory(view/indicator)
add_subdirectory(view/settings)
add_subdirectory(view/windowstracker)
add_subdirectory(wm)
add_subdirectory(wm/tracker)

set(latte_dbusXML dbus/org.kde.LatteDock.xml)
qt5_add_dbus_adaptor(lattedock-app_SRCS ${latte_dbusXML} lattecorona.h Latte::Corona lattedockadaptor)

ki18n_wrap_ui(lattedock-app_SRCS settings/actionsdialog/actionsdialog.ui)
ki18n_wrap_ui(lattedock-app_SRCS settings/detailsdialog/detailsdialog.ui)
ki18n_wrap_ui(lattedock-app_SRCS settings/exporttemplatedialog/exporttemplatedialog.ui)
ki18n_wrap_ui(lattedock-app_SRCS settings/screensdialog/screensdialog.ui)
ki18n_wrap_ui(lattedock-app_SRCS settings/settingsdialog/settingsdialog.ui)
ki18n_wrap_ui(lattedock-app_SRCS settings/viewsdialog/viewsdialog.ui)

ecm_add_qtwayland_client_protocol(lattedock-app_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/kde-primary-output-v1.xml
    BASENAME kde-primary-output-v1
)

add_executable(latte-dock ${lattedock-app_SRCS})

include(FakeTarget.cmake)

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
    Wayland::Client
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
install(FILES latte-layouts.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR})
install(FILES latte-indicators.knsrc DESTINATION  ${KDE_INSTALL_KNSRCDIR})


add_subdirectory(packageplugins)
