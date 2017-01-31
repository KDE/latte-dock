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

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Qml)
find_package(KF5I18n NO_MODULE)
find_package(KF5Activities REQUIRED)

find_package(X11 REQUIRED)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE OPTIONAL
                        PURPOSE "Required for building the X11 based workspace")

if(X11_FOUND)
  find_package(XCB MODULE REQUIRED COMPONENTS XCB RANDR)
  set_package_properties(XCB PROPERTIES TYPE REQUIRED)
  if(NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif(NOT X11_SM_FOUND)

  find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED COMPONENTS X11Extras)
endif()

if(X11_FOUND AND XCB_XCB_FOUND)
  set(HAVE_X11 true)
endif()

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

#add_subdirectory(packageplugins)

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
