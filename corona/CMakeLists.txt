
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


set(nowdock-app_SRCS
    ../libnowdock/types.cpp
    abstractinterface.cpp
    xwindowinterface.cpp
    visibilitymanager.cpp
    nowdockconfigview.cpp
    nowdockview.cpp
    packageplugins/shell/nowdockpackage.cpp
    nowdockcorona.cpp
    main.cpp
)

#add_subdirectory(packageplugins)

add_executable(nowdock ${nowdock-app_SRCS})

target_link_libraries(
    nowdock
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

install(TARGETS nowdock ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#even if hidden, the desktop file is needed anyways for kdbusservice::unique
install(FILES nowdock.desktop DESTINATION ${KDE_INSTALL_APPDIR})
