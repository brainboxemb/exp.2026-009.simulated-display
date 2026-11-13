file(READ "${CMAKE_CURRENT_LIST_DIR}/../../../pom.xml" SIMULATED_DISPLAY_POM)
string(
    REGEX MATCH
    "<artifactId>simulated-display</artifactId>[ \t\r\n]*<version>([^<]+)</version>"
    SIMULATED_DISPLAY_VERSION_MATCH
    "${SIMULATED_DISPLAY_POM}")
if(NOT SIMULATED_DISPLAY_VERSION_MATCH)
    message(FATAL_ERROR "Unable to read simulated-display version from pom.xml")
endif()
set(SIMULATED_DISPLAY_VERSION "${CMAKE_MATCH_1}")
string(
    REGEX REPLACE
    "-SNAPSHOT$"
    ""
    SIMULATED_DISPLAY_DISPLAY_VERSION
    "${SIMULATED_DISPLAY_VERSION}")

idf_component_register(
    SRCS "main.c"
    REQUIRES
        matrix_display
        hub75_output
        esp_driver_uart
        esp_timer
)

target_compile_definitions(
    ${COMPONENT_LIB}
    PRIVATE
        MATRIX_SOFTWARE_VERSION="v${SIMULATED_DISPLAY_DISPLAY_VERSION}")
