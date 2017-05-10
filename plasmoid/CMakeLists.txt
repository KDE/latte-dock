configure_file(metadata.desktop.cmake ${CMAKE_CURRENT_SOURCE_DIR}/package/metadata.desktop)

plasma_install_package(package org.kde.latte.plasmoid)
