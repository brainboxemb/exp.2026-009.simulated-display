cmake_minimum_required(VERSION 3.20)
project(simulated_display_native C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(display-core
    src/display_framebuffer.c
    src/custom_bitmap.c
    src/custom_character_map.c
    src/serial_lcd.c
)
target_include_directories(display-core PUBLIC include)

if(MSVC)
    target_compile_options(display-core PRIVATE /W4)
else()
    target_compile_options(display-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(display-host src/host_main.c)
target_link_libraries(display-host PRIVATE display-core)

include(CTest)
if(BUILD_TESTING)
    add_executable(test-serial-lcd tests/test_serial_lcd.c)
    target_link_libraries(test-serial-lcd PRIVATE display-core)
    add_test(NAME serial-lcd COMMAND test-serial-lcd)
endif()
