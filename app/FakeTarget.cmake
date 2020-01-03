execute_process(COMMAND find ../shell ../containment ../plasmoid -name "*.qml" -o -name "*.js"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE QML_SRCS_STRING)

string(REPLACE "\n" ";" QML_SRCS ${QML_SRCS_STRING})

# fake target for QtCreator project
add_custom_target(fake-target
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    SOURCES ${QML_SRCS})

# qmllint: qml static syntax checker
if(${CMAKE_BUILD_TYPE} MATCHES "Debug" AND NOT DEFINED ECM_ENABLE_SANITIZERS)
    find_program(QMLLINT qmllint)

    if(EXISTS "${QMLLINT}")
        message("-- Found qmllint: ${QMLLINT}")
        add_custom_command(TARGET latte-dock PRE_BUILD
            COMMAND ${QMLLINT} ${QML_SRCS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running qmllint")
    else()
        message("-- qmllint: QML Syntax verifier not found")
    endif()

    message("-- Enabling QML debugging and profiling")
    add_definitions(-DQT_QML_DEBUG)
    add_definitions(-DQT_FATAL_WARNINGS)

elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
    message("-- Disabling debug info")
    add_definitions(-DQT_NO_DEBUG)

endif()
