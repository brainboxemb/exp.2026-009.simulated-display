set(MATRIX_NATIVE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../native")
idf_component_register(
    SRCS
        "${MATRIX_NATIVE_ROOT}/src/matrix_framebuffer.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_pixel_font.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_vbz_font.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_numeric_font.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_sprite.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_status_icon.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_lvgl.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_font.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_serial.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_theme.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_startup_screen.c"
        "${MATRIX_NATIVE_ROOT}/src/matrix_race_screen.c"
    INCLUDE_DIRS "${MATRIX_NATIVE_ROOT}/include"
    PRIV_INCLUDE_DIRS "${MATRIX_NATIVE_ROOT}/src"
    REQUIRES lvgl
)
