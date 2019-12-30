project(lattedock)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(VERSION 0.9.6)
set(AUTHOR "Michail Vourlakos, Smith Ar")
set(EMAIL "mvourlakos@gmail.com, audoban@openmailbox.org")
set(WEBSITE "https://userbase.kde.org/LatteDock")
set(BUG_ADDRESS "https://bugs.kde.org/enter_bug.cgi?product=lattedock")
set(FAQS "https://userbase.kde.org/LatteDock/FAQ")

set(QT_MIN_VERSION "5.9.0")
set(KF5_MIN_VERSION "5.38.0")

set(KF5_LOCALE_PREFIX "")

find_package(ECM ${KF5_MIN_VER} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS DBus Gui Qml Quick)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Activities Archive CoreAddons GuiAddons Crash DBusAddons Declarative GlobalAccel I18n 
    IconThemes NewStuff Notifications Plasma PlasmaQuick Wayland WindowSystem XmlGui)

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

string(REGEX MATCH "\\.([^]]+)\\." KF5_VERSION_MINOR ${KF5_VERSION})
string(REGEX REPLACE "\\." "" KF5_VERSION_MINOR ${KF5_VERSION_MINOR})

message(STATUS "KF5 VERSION : ${KF5_VERSION}")
message(STATUS "KF5 VERSION MINOR : ${KF5_VERSION_MINOR}")

if(${KF5_VERSION_MINOR} LESS "62")
find_package(KF5 REQUIRED COMPONENTS SysGuard)
endif()

include(ECMQMLModules)
ecm_find_qmlmodule(QtQuick 2.7)
ecm_find_qmlmodule(QtQuick.Layouts 1.3)
ecm_find_qmlmodule(QtQuick.Controls 1.4)
ecm_find_qmlmodule(QtQuick.Controls.Styles.Plasma 2.0)
ecm_find_qmlmodule(QtGraphicalEffects 1.0)
ecm_find_qmlmodule(org.kde.kquickcontrolsaddons 2.0)
ecm_find_qmlmodule(org.kde.plasma.core 2.0)
ecm_find_qmlmodule(org.kde.plasma.components 2.0)

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

add_subdirectory(declarativeimports)
add_subdirectory(liblatte2)
add_subdirectory(indicators)
add_subdirectory(app)
add_subdirectory(containmentactions)
add_subdirectory(containment)
add_subdirectory(icons)
add_subdirectory(plasmoid)
add_subdirectory(shell)

ki18n_install(po)
