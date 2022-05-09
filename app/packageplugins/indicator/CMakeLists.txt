set(indicator_SRCS
    
)

kcoreaddons_add_plugin(latte_packagestructure_indicator SOURCES indicatorpackage.cpp INSTALL_NAMESPACE kpackage/packagestructure)
set_target_properties(latte_packagestructure_indicator PROPERTIES OUTPUT_NAME latte_indicator)

target_link_libraries(latte_packagestructure_indicator
   KF5::Declarative
   KF5::I18n
   KF5::Package
)

kcoreaddons_desktop_to_json(latte_packagestructure_indicator latte-packagestructure-indicator.desktop)

install(FILES latte-indicator.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})
