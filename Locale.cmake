find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(WARNING "-- msgfmt: not found. Translations will *not* be installed")

else()
    #containment translations
    set(catalogname plasma_applet_org.kde.latte.containment)
    add_custom_target(translations-containment ALL)

    file(GLOB PO_FILES po/containment/*.po)

    foreach(_poFile ${PO_FILES})
        get_filename_component(_poFileName ${_poFile} NAME)
        string(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName})
        string(REGEX REPLACE "\\.po$" "" _langCode ${_langCode})

        if(_langCode)
            get_filename_component(_lang ${_poFile} NAME_WE)
            set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/containment/${_lang}.gmo)

            add_custom_command(TARGET translations-containment
                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                DEPENDS ${_poFile})

            install(FILES ${_gmoFile}
                DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/
                RENAME ${catalogname}.mo)
        endif()
    endforeach()

    #plasmoid translations
    set(catalogname-plasmoid plasma_applet_org.kde.latte.plasmoid)
    add_custom_target(translations-plasmoid ALL)

    file(GLOB PO_FILES_PLASMOID po/plasmoid/*.po)

    foreach(_poFile ${PO_FILES_PLASMOID})
        get_filename_component(_poFileName ${_poFile} NAME)
        string(REGEX REPLACE "^${catalogname-plasmoid}_?" "" _langCode ${_poFileName})
        string(REGEX REPLACE "\\.po$" "" _langCode ${_langCode})

        if(_langCode)
            get_filename_component(_lang ${_poFile} NAME_WE)
            set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/plasmoid/${_lang}.gmo)

            add_custom_command(TARGET translations-plasmoid
                    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                    DEPENDS ${_poFile})

            install(FILES ${_gmoFile}
                DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/
                RENAME ${catalogname-plasmoid}.mo)
        endif()
    endforeach()

    #app translations
    set(catalogname-app latte-dock)
    add_custom_target(translations-app ALL)

    file(GLOB PO_FILES_CORONA po/app/*.po)

    foreach(_poFile ${PO_FILES_CORONA})
        get_filename_component(_poFileName ${_poFile} NAME)
        string(REGEX REPLACE "^${catalogname-app}_?" "" _langCode ${_poFileName} )
        string(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

        if(_langCode)
            get_filename_component(_lang ${_poFile} NAME_WE)
            set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/app/${_lang}.gmo)

            add_custom_command(TARGET translations-app
                    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                    DEPENDS ${_poFile})

            install(FILES ${_gmoFile}
                DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/
                RENAME ${catalogname-app}.mo)
        endif()
    endforeach()
endif()
