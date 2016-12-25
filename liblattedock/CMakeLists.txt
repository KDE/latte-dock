set (REQUIRED_QT_VERSION "5.6.0")

find_package(ECM 1.8.0 REQUIRED NO_MODULE)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Qml)

find_package(KF5 REQUIRED COMPONENTS
    WindowSystem
    CoreAddons
)

set(CMAKE_AUTOMOC ON)

set(lattedock_SRCS
    lattedockplugin.cpp
    windowsystem.cpp
    dock.cpp
)
    
add_library(lattedockplugin SHARED ${lattedock_SRCS})

target_link_libraries(lattedockplugin
        Qt5::Quick
        Qt5::Qml
        KF5::WindowSystem
        KF5::CoreAddons
)

install(TARGETS lattedockplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/dock)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/latte/dock)
