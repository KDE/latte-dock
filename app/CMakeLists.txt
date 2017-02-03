include(KDEInstallDirs)
include(KDECMakeSettings)
#include(KDECompilerSettings NO_POLICY_SCOPE)
#include(ECMPackageConfigHelpers)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(KDEPackageAppTemplates)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-latte.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-latte.h)

set(lattedock-app_SRCS
    ../liblattedock/dock.cpp
    ../liblattedock/windowsystem.cpp
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
    main.cpp
)

add_executable(latte-dock ${lattedock-app_SRCS})

include(FakeTarget.cmake)

target_link_libraries(
    latte-dock
    Qt5::Quick
    Qt5::Qml
    Qt5::X11Extras
    KF5::I18n
    KF5::Declarative
    KF5::CoreAddons
    KF5::XmlGui
    KF5::Plasma
    KF5::PlasmaQuick
    KF5::Activities
    KF5::QuickAddons
    KF5::WindowSystem
    KF5::WaylandClient
)

if(HAVE_X11)
  target_link_libraries(latte-dock ${X11_LIBRARIES} ${XCB_LIBRARIES} )
endif()

install(TARGETS latte-dock ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#even if hidden, the desktop file is needed anyways for kdbusservice::unique
install(FILES latte-dock.desktop DESTINATION ${KDE_INSTALL_APPDIR})
