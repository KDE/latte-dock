file(READ TRANSLATORS translators_str)

string(REGEX REPLACE "\n" ";" translators_list "${translators_str}")

list(GET translators_list 0 TRANSLATORS)
list(GET translators_list 1 TRANSLATORS_EMAIL)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app/config-latte.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/app/config-latte.h)
