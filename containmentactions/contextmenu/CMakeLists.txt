add_definitions(-DTRANSLATION_DOMAIN=\"plasma_containmentactions_lattecontextmenu\")

set(contextmenu_SRCS
    menu.cpp
)

add_library(plasma_containmentactions_lattecontextmenu MODULE ${contextmenu_SRCS})

kcoreaddons_desktop_to_json(plasma_containmentactions_lattecontextmenu plasma-containmentactions-lattecontextmenu.desktop)

target_link_libraries(plasma_containmentactions_lattecontextmenu
                        Qt5::DBus
                        Qt5::Widgets
                        KF5::I18n
                        KF5::XmlGui
                        KF5::Plasma)

install(TARGETS plasma_containmentactions_lattecontextmenu DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-containmentactions-lattecontextmenu.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
