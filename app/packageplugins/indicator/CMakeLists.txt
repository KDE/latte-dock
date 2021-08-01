set(indicator_SRCS
    indicatorpackage.cpp
)

add_library(latte_packagestructure_indicator MODULE ${indicator_SRCS})

target_link_libraries(latte_packagestructure_indicator
   KF5::Declarative
   KF5::I18n
   KF5::Package
)

kcoreaddons_desktop_to_json(latte_packagestructure_indicator latte-packagestructure-indicator.desktop)

install(TARGETS latte_packagestructure_indicator DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)
set_target_properties(latte_packagestructure_indicator PROPERTIES OUTPUT_NAME latte_indicator)

install(FILES latte-indicator.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
