file(COPY "contents" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/release)
#update the version number in the configuration window
configure_file(metadata.desktop.cmake release/metadata.desktop)
