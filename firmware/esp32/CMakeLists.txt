cmake_minimum_required(VERSION 3.16)

# Trim ESP-IDF to the application and its transitive component dependencies.
# PlatformIO registers src/ as our application component, and injects
# __pio_env into the ESP-IDF framework to derive generic build flags.
# Both components MUST be listed: omitting __pio_env breaks PlatformIO's
# CMake code model even when the firmware itself is otherwise configured.
# MINIMAL_BUILD is only for projects with a special main/ component.
set(COMPONENTS src __pio_env)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(experiment_009_matrix_esp32)
