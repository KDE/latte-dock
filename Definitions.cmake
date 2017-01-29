file(READ TRANSLATORS translators_str)

string(REGEX REPLACE "\n" ";" translators_list "${translators_str}")

list(GET translators_list 0 translators)
list(GET translators_list 1 translators_email)

add_definitions(
    -DLATTE_VERSION="${VERSION}"
    -DBUG_ADDRESS="${BUG_ADDRESS}"
    -DTRANSLATORS="${translators}"
    -DTRANSLATORS_EMAIL="${translators_email}"
)


