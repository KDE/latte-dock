file(COPY "contents" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/release)
#update the version number in the configuration window
file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/release/contents/configuration/LatteDockConfiguration.qml.cmake)
configure_file(metadata.desktop.cmake release/metadata.desktop)
configure_file(contents/configuration/LatteDockConfiguration.qml.cmake release/contents/configuration/LatteDockConfiguration.qml)
