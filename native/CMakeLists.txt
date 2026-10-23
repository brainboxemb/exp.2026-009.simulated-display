cmake_minimum_required(VERSION 3.20)
project(simulated_display_native C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)

# LVGL is configured from its own empty defconfig plus the small set of
# features used by the 160x64 matrix renderer. Unspecified LVGL defaults are
# intentionally not used.
set(FETCHCONTENT_BASE_DIR
    "${CMAKE_BINARY_DIR}/_deps"
    CACHE PATH "" FORCE)
set(LV_BUILD_CONF_PATH "" CACHE PATH "" FORCE)
set(LV_BUILD_USE_KCONFIG ON CACHE BOOL "" FORCE)
set(LV_BUILD_DEFCONFIG_STRICT ON CACHE BOOL "" FORCE)
set(LV_BUILD_DEFCONFIG_PATH
    "${FETCHCONTENT_BASE_DIR}/lvgl-src/configs/defconfigs/empty.defconfig;${CMAKE_CURRENT_SOURCE_DIR}/lvgl-matrix.defconfig"
    CACHE STRING "" FORCE)
set(LV_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LV_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG v9.6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(lvgl)

add_library(legacy-display
    src/display_framebuffer.c
    src/custom_bitmap.c
    src/custom_character_map.c
    src/serial_lcd.c
)
target_include_directories(legacy-display PUBLIC include)

add_library(matrix-display
    src/matrix_framebuffer.c
    src/matrix_font.c
    src/matrix_theme.c
    src/matrix_race_screen.c
)
target_include_directories(matrix-display PUBLIC include)
target_link_libraries(matrix-display PUBLIC lvgl::lvgl)

if(MSVC)
    target_compile_options(legacy-display PRIVATE /W4)
    target_compile_options(matrix-display PRIVATE /W4)
else()
    target_compile_options(legacy-display PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(matrix-display PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(display-host src/host_main.c)
target_link_libraries(display-host PRIVATE legacy-display)

add_executable(matrix-display-host src/matrix_host_main.c)
target_link_libraries(matrix-display-host PRIVATE matrix-display)

include(CTest)
if(BUILD_TESTING)
    add_executable(test-serial-lcd tests/test_serial_lcd.c)
    target_link_libraries(test-serial-lcd PRIVATE legacy-display)
    add_test(NAME serial-lcd COMMAND test-serial-lcd)

    add_executable(test-matrix-framebuffer tests/test_matrix_framebuffer.c)
    target_link_libraries(test-matrix-framebuffer PRIVATE matrix-display)
    add_test(NAME matrix-framebuffer COMMAND test-matrix-framebuffer)

    add_executable(test-matrix-race-screen tests/test_matrix_race_screen.c)
    target_link_libraries(test-matrix-race-screen PRIVATE matrix-display)
    add_test(NAME matrix-race-screen COMMAND test-matrix-race-screen)
endif()
