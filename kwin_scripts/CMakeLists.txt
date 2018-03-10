install( DIRECTORY activatelattelaunchermenu DESTINATION ${DATA_INSTALL_DIR}/kwin/scripts )

# service files
install( FILES activatelattelaunchermenu/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME kwin-script-activatelattelaunchermenu.desktop )
