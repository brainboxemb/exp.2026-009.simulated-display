cmake_minimum_required(VERSION 3.20)
project(simulated_display_native C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)

set(LV_BUILD_CONF_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h
    CACHE PATH "" FORCE)
set(LV_BUILD_USE_KCONFIG OFF CACHE BOOL "" FORCE)
set(LV_BUILD_SET_CONFIG_OPTS OFF CACHE BOOL "" FORCE)
set(LV_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
set(LV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LV_FETCH_DEPENDENCIES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG v9.6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(lvgl)

#
# LVGL's default CMake target recursively includes every source file and lets
# preprocessor configuration make unused platform/features empty. For this
# small matrix renderer we remove whole optional subsystems that are disabled
# in lv_conf.h, so they are not compiled at all.
#
get_target_property(LVGL_MATRIX_SOURCES lvgl SOURCES)

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX "/src/drivers/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX "/src/libs/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX "/src/others/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX "/src/debugging/")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/draw/(dma2d|espressif|eve|nanovg|nema_gfx|nxp|opengles|renesas|sdl|sifli|snapshot|vg_lite)/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/draw/sw/arm2d/")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/layouts/(flex|grid)/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/themes/(default|mono|simple)/")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/widgets/")
list(APPEND LVGL_MATRIX_SOURCES
    "${lvgl_SOURCE_DIR}/src/widgets/label/lv_label.c")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/font/(binfont_loader|font_manager|freetype|imgfont|tiny_ttf)/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/font/lv_font_(dejavu|source_han_sans).*\\.c$")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/font/lv_font_montserrat_.*\\.c$")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/image/svg/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/image/lv_(bmp|libjpeg_turbo|libpng|libwebp|lodepng|tjpgd)\\.c$")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/fs/lv_fs_(arduino_esp_littlefs|arduino_sd|fatfs|frogfs|littlefs|memfs|posix|stdio|uefi|win32)\\.(c|cpp)$")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/osal/lv_(cmsis_rtos2|freertos|linux|mqx|pthread|rtthread|sdl2|windows)\\.c$")

list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/stdlib/(builtin|micropython|rtthread|uefi)/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/logging/")
list(FILTER LVGL_MATRIX_SOURCES EXCLUDE REGEX
    "/src/indev/lv_(gridnav|indev_gesture)\\.c$")

set_property(
    TARGET lvgl
    PROPERTY SOURCES "${LVGL_MATRIX_SOURCES}")

add_library(legacy-display
    src/display_framebuffer.c
    src/custom_bitmap.c
    src/custom_character_map.c
    src/serial_lcd.c
)
target_include_directories(legacy-display PUBLIC include)

add_library(matrix-display
    src/matrix_framebuffer.c
    src/matrix_pixel_font.c
    src/matrix_vbz_font.c
    src/matrix_numeric_font.c
    src/matrix_sprite.c
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
    target_include_directories(test-matrix-race-screen PRIVATE src)
    target_link_libraries(test-matrix-race-screen PRIVATE matrix-display)
    add_test(NAME matrix-race-screen COMMAND test-matrix-race-screen)
endif()
