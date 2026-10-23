cmake_minimum_required(VERSION 3.20)
project(simulated_display_native C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(legacy-display
    src/display_framebuffer.c
    src/custom_bitmap.c
    src/custom_character_map.c
    src/serial_lcd.c
)
target_include_directories(legacy-display PUBLIC include)

add_library(matrix-display
    src/matrix_framebuffer.c
)
target_include_directories(matrix-display PUBLIC include)

if(MSVC)
    target_compile_options(legacy-display PRIVATE /W4)
    target_compile_options(matrix-display PRIVATE /W4)
else()
    target_compile_options(legacy-display PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(matrix-display PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(display-host src/host_main.c)
target_link_libraries(display-host PRIVATE legacy-display)

include(CTest)
if(BUILD_TESTING)
    add_executable(test-serial-lcd tests/test_serial_lcd.c)
    target_link_libraries(test-serial-lcd PRIVATE legacy-display)
    add_test(NAME serial-lcd COMMAND test-serial-lcd)

    add_executable(test-matrix-framebuffer tests/test_matrix_framebuffer.c)
    target_link_libraries(test-matrix-framebuffer PRIVATE matrix-display)
    add_test(NAME matrix-framebuffer COMMAND test-matrix-framebuffer)
endif()
