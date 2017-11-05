project(lattedock)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(VERSION 0.7.75)
set(AUTHOR "Michail Vourlakos, Smith Ar")
set(EMAIL "mvourlakos@gmail.com, audoban@openmailbox.org")
set(WEBSITE "https://github.com/psifidotos/Latte-Dock")
set(BUG_ADDRESS "https://github.com/psifidotos/Latte-Dock/issues")
set(FAQS "https://github.com/psifidotos/Latte-Dock/wiki/F.A.Q.")

set(QT_MIN_VERSION "5.7.0")
set(KF5_MIN_VERSION "5.29.0")

set(KF5_LOCALE_PREFIX "")

find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS DBus Gui Qml Quick)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Activities Archive CoreAddons Crash DBusAddons Declarative GlobalAccel I18n
    IconThemes Notifications Plasma PlasmaQuick Wayland WindowSystem XmlGui)

find_package(X11 REQUIRED)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
    URL "http://www.x.org"
    TYPE OPTIONAL
    PURPOSE "Required for building the X11 based workspace")

if(X11_FOUND)
    find_package(XCB MODULE REQUIRED COMPONENTS XCB RANDR EVENT)
    set_package_properties(XCB PROPERTIES TYPE REQUIRED)
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS X11Extras)

    if(NOT X11_SM_FOUND)
        message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.
                \nPlease install libSM.\n")
    endif(NOT X11_SM_FOUND)

endif()

if(X11_FOUND AND XCB_XCB_FOUND)
    set(HAVE_X11 ON)
endif()

include(CheckIncludeFiles)
include(CMakePackageConfigHelpers)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(KDECMakeSettings)
#include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEInstallDirs)
include(KDEPackageAppTemplates)
include(WriteBasicConfigVersionFile)

include(Definitions.cmake)

add_subdirectory(app)
add_subdirectory(applets)
add_subdirectory(containment)
add_subdirectory(icons)
add_subdirectory(liblattedock)
add_subdirectory(plasmoid)
add_subdirectory(shell)

ki18n_install(${CMAKE_CURRENT_BINARY_DIR}/po)
