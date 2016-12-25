file(COPY "contents" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/release)
#update the version number in the configuration window
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/release/contents/ui/ConfigAppearance.qml.cmake)
configure_file(metadata.desktop.cmake release/metadata.desktop)
configure_file(contents/ui/ConfigAppearance.qml.cmake release/contents/ui/ConfigAppearance.qml)

