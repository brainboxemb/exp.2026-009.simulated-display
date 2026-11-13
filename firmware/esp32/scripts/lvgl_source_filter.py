"""Limit upstream LVGL 9.6 to the units used by the native C matrix renderer.

Without this filter PlatformIO compiles 551 LVGL sources (including
unrelated graphics backends). The exact tag v9.6.0 is pinned in
platformio.ini. Fail qualification if the source budget drifts.
"""
Import("env")

if env.IsIntegrationDump():
    Return()

KEEP_DIRS = (
    "core/",
    "display/",
    "draw/sw/",
    "font/fmt_txt/",
    "indev/",
    "logging/",
    "misc/",
    "stdlib/clib/",
    "tick/",
    "widgets/label/",
)
KEEP_FILES = (
    "lv_init.c",
    "draw/lv_draw.c",
    "draw/lv_draw_3d.c",
    "draw/lv_draw_arc.c",
    "draw/lv_draw_blur.c",
    "draw/lv_draw_buf.c",
    "draw/lv_draw_image.c",
    "draw/lv_draw_label.c",
    "draw/lv_draw_line.c",
    "draw/lv_draw_mask.c",
    "draw/lv_draw_rect.c",
    "draw/lv_draw_triangle.c",
    "draw/lv_draw_utils.c",
    "draw/lv_draw_vector.c",
    "draw/convert/lv_draw_buf_convert.c",
    "font/lv_font.c",
    "font/lv_font_unscii_8.c",
    "font/lv_font_unscii_16.c",
    "fs/lv_fs.c",
    "image/lv_bin_decoder.c",
    "image/lv_image_decoder.c",
    "layouts/lv_layout.c",
    "osal/lv_os.c",
    "osal/lv_os_none.c",
    "stdlib/lv_mem.c",
    "themes/lv_theme.c",
)

def filter_lvgl_sources(build_env, node):
    path = node.srcnode().get_abspath().replace("\\", "/")
    marker = "/lvgl/src/"
    if marker not in path:
        return node
    relative = path.rsplit(marker, 1)[1]
    if relative in KEEP_FILES or relative.startswith(KEEP_DIRS):
        return node
    return None

env.AddBuildMiddleware(filter_lvgl_sources)
print("LVGL v9.6.0 source allowlist enabled for shared native matrix display")
