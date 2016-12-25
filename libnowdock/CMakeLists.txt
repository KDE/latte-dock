set (REQUIRED_QT_VERSION "5.6.0")

find_package(ECM 1.8.0 REQUIRED NO_MODULE)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Qml)

find_package(KF5 REQUIRED COMPONENTS
    Plasma
    PlasmaQuick
    WindowSystem
    KDELibs4Support
    CoreAddons
)

set(CMAKE_AUTOMOC ON)

set(nowdock_SRCS
    nowdockplugin.cpp
    panelwindow.cpp
    windowsystem.cpp
    xwindowinterface.cpp
    abstractinterface.cpp
    types.cpp
)
    
add_library(nowdockplugin SHARED ${nowdock_SRCS})

target_link_libraries(nowdockplugin
        Qt5::Quick
        Qt5::Qml
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::WindowSystem
        KF5::KDELibs4Support
        KF5::CoreAddons
)

install(TARGETS nowdockplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/nowdock)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/nowdock)
