add_definitions(-DTRANSLATION_DOMAIN=\"plasma_containmentactions_lattecontextmenu\")

set(contextmenu_SRCS
    generictools.cpp
    layoutmenuitemwidget.cpp
    menu.cpp
)

add_library(plasma_containmentactions_lattecontextmenu MODULE ${contextmenu_SRCS})

target_link_libraries(plasma_containmentactions_lattecontextmenu
                        Qt::DBus
                        Qt::Widgets
                        KF6::I18n
                        KF6::XmlGui
                        Plasma
                      )

install(TARGETS plasma_containmentactions_lattecontextmenu DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/containmentactions)
