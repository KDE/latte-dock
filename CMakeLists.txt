project(lattedock)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(VERSION 0.5.91)
set(AUTHOR "Michail Vourlakos, Smith Ar")
set(EMAIL "mvourlakos@gmail.com, audoban@openmailbox.org")
set(WEBSITE "https://github.com/psifidotos/Latte-Dock")

set(QT_MIN_VERSION "5.6.0")
set(KF5_MIN_VERSION "5.26.0")

set(KF5_LOCALE_PREFIX "")

find_package(ECM 1.8.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
     Quick X11Extras)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Plasma PlasmaQuick WindowSystem Declarative
    I18n CoreAddons XmlGui DBusAddons IconThemes)


FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
    MESSAGE(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)

        #containment translations
        SET(catalogname plasma_applet_org.kde.latte.containment)
        ADD_CUSTOM_TARGET(translations-containment ALL)

        FILE(GLOB PO_FILES po/containment/*.po)

        FOREACH(_poFile ${PO_FILES})
                GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
                STRING(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
                STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

                IF( _langCode )
                        GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
                        SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/containment/${_lang}.gmo)

                        ADD_CUSTOM_COMMAND(TARGET translations-containment
                                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                                DEPENDS ${_poFile})
                        INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
                ENDIF( _langCode )
        ENDFOREACH(_poFile ${PO_FILES})

        #plasmoid translations
        SET(catalogname-plasmoid plasma_applet_org.kde.latte.plasmoid)
        ADD_CUSTOM_TARGET(translations-plasmoid ALL)

        FILE(GLOB PO_FILES_PLASMOID po/plasmoid/*.po)

        FOREACH(_poFile ${PO_FILES_PLASMOID})
                GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
                STRING(REGEX REPLACE "^${catalogname-plasmoid}_?" "" _langCode ${_poFileName} )
                STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

                IF( _langCode )
                        GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
                        SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/plasmoid/${_lang}.gmo)

                        ADD_CUSTOM_COMMAND(TARGET translations-plasmoid
                                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                                DEPENDS ${_poFile})
                        INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/ RENAME ${catalogname-plasmoid}.mo)
                ENDIF( _langCode )
        ENDFOREACH(_poFile ${PO_FILES})

        #shell translations
        #SET(catalogname-shell plasma_shell_org.kde.latte.shell)
        #ADD_CUSTOM_TARGET(translations-shell ALL)

        #FILE(GLOB PO_FILES_SHELL po/shell/*.po)

        #FOREACH(_poFile ${PO_FILES_SHELL})
        #        GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
        #        STRING(REGEX REPLACE "^${catalogname-shell}_?" "" _langCode ${_poFileName} )
        #        STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

        #        IF( _langCode )
        #                GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
        #                SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/shell/${_lang}.gmo)

        #                ADD_CUSTOM_COMMAND(TARGET translations-shell
        #                        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
        #                        DEPENDS ${_poFile})
        #                INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/ RENAME ${catalogname-shell}.mo)
        #        ENDIF( _langCode )
        #ENDFOREACH(_poFile ${PO_FILES})

        #app translations
        SET(catalogname-app latte-dock)
        ADD_CUSTOM_TARGET(translations-app ALL)

        FILE(GLOB PO_FILES_CORONA po/app/*.po)

        FOREACH(_poFile ${PO_FILES_CORONA})
                GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
                STRING(REGEX REPLACE "^${catalogname-app}_?" "" _langCode ${_poFileName} )
                STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

                IF( _langCode )
                        GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
                        SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/app/${_lang}.gmo)

                        ADD_CUSTOM_COMMAND(TARGET translations-app
                                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                                DEPENDS ${_poFile})
                        INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${KF5_LOCALE_PREFIX}${_langCode}/LC_MESSAGES/ RENAME ${catalogname-app}.mo)
                ENDIF( _langCode )
        ENDFOREACH(_poFile ${PO_FILES})

ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)

add_subdirectory(app)

add_subdirectory(liblattedock)
add_subdirectory(containment)
plasma_install_package(build/containment/release org.kde.latte.containment)
add_subdirectory(plasmoid)
plasma_install_package(build/plasmoid/release org.kde.latte.plasmoid)

add_subdirectory(shell)
plasma_install_package(build/shell/release org.kde.latte.shell shells shell)

add_subdirectory(icons)

