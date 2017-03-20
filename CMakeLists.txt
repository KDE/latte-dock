project(lattedock)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(VERSION 0.5.91)
set(AUTHOR "Michail Vourlakos, Smith Ar")
set(EMAIL "mvourlakos@gmail.com, audoban@openmailbox.org")
set(WEBSITE "https://github.com/psifidotos/Latte-Dock")
set(BUG_ADDRESS "https://github.com/psifidotos/Latte-Dock/issues")

set(QT_MIN_VERSION "5.6.0")
set(KF5_MIN_VERSION "5.26.0")

set(KF5_LOCALE_PREFIX "")

find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS Quick Qml DBus)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Plasma PlasmaQuick WindowSystem Declarative Activities Notifications
    I18n CoreAddons Archive XmlGui DBusAddons IconThemes Wayland)

find_package(X11 REQUIRED)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
    URL "http://www.x.org"
    TYPE OPTIONAL
    PURPOSE "Required for building the X11 based workspace")

if(X11_FOUND)
    find_package(XCB MODULE REQUIRED COMPONENTS XCB RANDR EVENT)
    set_package_properties(XCB PROPERTIES TYPE REQUIRED)
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS X11Extras)
endif()

if(X11_FOUND AND XCB_XCB_FOUND)
    set(HAVE_X11 ON)
endif()

include(KDEInstallDirs)
include(KDECMakeSettings)
#include(KDECompilerSettings NO_POLICY_SCOPE)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(KDEPackageAppTemplates)

include(Definitions.cmake)
include(Locale.cmake)

add_subdirectory(app)
add_subdirectory(liblattedock)
add_subdirectory(shell)
add_subdirectory(containment)
add_subdirectory(plasmoid)
add_subdirectory(icons)

plasma_install_package(build/shell/release org.kde.latte.shell shells shell)
plasma_install_package(build/containment/release org.kde.latte.containment)
plasma_install_package(build/plasmoid/release org.kde.latte.plasmoid)
