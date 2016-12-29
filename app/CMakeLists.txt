find_package(KF5I18n NO_MODULE)

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


set(lattedock-app_SRCS
    ../liblattedock/dock.cpp
    windowinfowrap.cpp
    abstractwindowinterface.cpp
    xwindowinterface.cpp
    visibilitymanager.cpp
    nowdockconfigview.cpp
    nowdockview.cpp
    packageplugins/shell/nowdockpackage.cpp
    nowdockcorona.cpp
    main.cpp
)

#add_subdirectory(packageplugins)

add_executable(latte-dock ${lattedock-app_SRCS})

target_link_libraries(
    latte-dock
    Qt5::Widgets
    Qt5::Quick
    Qt5::Qml
    KF5::I18n
    KF5::CoreAddons
    KF5::XmlGui
    KF5::PlasmaQuick
    KF5::Plasma
    KF5::QuickAddons
    KF5::DBusAddons    
    KF5::Notifications
)

install(TARGETS latte-dock ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#even if hidden, the desktop file is needed anyways for kdbusservice::unique
install(FILES latte-dock.desktop DESTINATION ${KDE_INSTALL_APPDIR})
