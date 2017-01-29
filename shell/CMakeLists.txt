file(COPY "contents" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/release)
file(COPY "defaults" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/release)
configure_file(metadata.desktop.cmake release/metadata.desktop)
