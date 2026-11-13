cmake_minimum_required(VERSION 3.16)

# PlatformIO uses src/ as the application component (not ESP-IDF's main/).
# Without COMPONENTS, ESP-IDF builds every discovered framework component.
# Select only the application: its REQUIRES/PRIV_REQUIRES dependencies
# recursively pull in the HUB75, LVGL, UART and other required components.
# MINIMAL_BUILD is not suitable for a project without a main/ component.
set(COMPONENTS src)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(experiment_009_matrix_esp32)
