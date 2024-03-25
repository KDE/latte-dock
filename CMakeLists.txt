cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(lattedock)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VERSION 0.10.240)
set(AUTHOR "Michail Vourlakos, Smith Ar")
set(EMAIL "mvourlakos@gmail.com, audoban@openmailbox.org")
set(WEBSITE "https://userbase.kde.org/LatteDock")
set(BUG_ADDRESS "https://bugs.kde.org/enter_bug.cgi?product=lattedock")
set(FAQS "https://userbase.kde.org/LatteDock/FAQ")

set(QT_MIN_VERSION "6.5.0")
set(KF6_MIN_VERSION "6.0.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.84.0")

set(KF6_LOCALE_PREFIX "")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs6)
include(CheckIncludeFiles)
include(CMakePackageConfigHelpers)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEPackageAppTemplates)
include(WriteBasicConfigVersionFile)


find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS DBus Gui Qml Quick)

#find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
#    Activities Archive CoreAddons GuiAddons Crash DBusAddons Declarative GlobalAccel Kirigami2
#    I18n IconThemes KIO NewStuff Notifications Plasma PlasmaQuick Wayland WindowSystem XmlGui)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Archive
    CoreAddons
    Crash
    DBusAddons
    Declarative
    GlobalAccel
    GuiAddons
    I18n
    IconThemes
    KIO
    Kirigami2
    NewStuff
    Notifications
    Package
    Svg
    WindowSystem
    XmlGui
)

find_package(Plasma ${KF6_MIN_VERSION} REQUIRED)
find_package(PlasmaActivities ${KF6_MIN_VERSION} REQUIRED)
find_package(PlasmaQuick ${KF6_MIN_VERSION} REQUIRED)

find_package(LibTaskManager REQUIRED)
add_definitions(-DPLASMA_WORKSPACE_VERSION="${LibTaskManager_VERSION}")

find_package(X11 REQUIRED)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
    URL "http://www.x.org"
    TYPE OPTIONAL
    PURPOSE "Required for building the X11 based workspace")

if(X11_FOUND)
    find_package(XCB MODULE REQUIRED COMPONENTS XCB RANDR SHAPE EVENT)
    set_package_properties(XCB PROPERTIES TYPE REQUIRED)

    if(NOT X11_SM_FOUND)
        message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.
                \nPlease install libSM.\n")
    endif(NOT X11_SM_FOUND)

endif()

if(X11_FOUND AND XCB_XCB_FOUND)
    set(HAVE_X11 ON)
endif()


#find_package(QtWaylandScanner REQUIRED)
find_package(Qt${QT_MAJOR_VERSION}WaylandClient)
find_package(PlasmaWaylandProtocols 1.6 REQUIRED)
find_package(Wayland REQUIRED COMPONENTS Client)

string(REGEX MATCH "\\.([^]]+)\\." KF6_VERSION_MINOR ${KF6_VERSION})
string(REGEX REPLACE "\\." "" KF6_VERSION_MINOR ${KF6_VERSION_MINOR})

message(STATUS "KF6 VERSION : ${KF6_VERSION}")
message(STATUS "KF6 VERSION MINOR : ${KF6_VERSION_MINOR}")


include(ECMFindQmlModule)
ecm_find_qmlmodule(org.kde.kquickcontrolsaddons 2.0)
ecm_find_qmlmodule(org.kde.plasma.core 2.0)
ecm_find_qmlmodule(org.kde.plasma.components 2.0)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app/config-latte.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/app/config-latte.h)

#hide warnings
string(REPLACE "-Wall" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-Wdeprecated-declarations" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-Wreorder" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-Wunused-variable" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-Wunused-parameter" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

#add format security check
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat -Werror=format-security")

message(STATUS "COMPILER FLAGS : ${CMAKE_CXX_FLAGS}")

# Share Same Core Types between declarativeimports and app
set(CORETYPESHEADER "LIBCORETYPES_H")
configure_file(declarativeimports/coretypes.h.in declarativeimports/core/types.h)
set(CORETYPESHEADER "APPCORETYPES_H")
configure_file(declarativeimports/coretypes.h.in app/coretypes.h)
set(CORETYPESHEADER "LATTEGENERALCONTAINMENTTYPES_H")
configure_file(declarativeimports/coretypes.h.in containment/plugin/lattetypes.h)

# Share App::Settings::GenericTools to containment actions
configure_file(app/settings/generic/generictools.h containmentactions/contextmenu/generictools.h)
configure_file(app/settings/generic/generictools.cpp containmentactions/contextmenu/generictools.cpp)
configure_file(app/data/contextmenudata.h containmentactions/contextmenu/contextmenudata.h)

# subdirectories
add_subdirectory(declarativeimports)
add_subdirectory(indicators)
add_subdirectory(app)
add_subdirectory(containmentactions)
add_subdirectory(containment)
add_subdirectory(icons)
add_subdirectory(plasmoid)
add_subdirectory(shell)

ki18n_install(po)
