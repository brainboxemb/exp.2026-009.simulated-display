"""Compile the desktop simulator's portable C renderer in Arduino PlatformIO.

One source of truth: native/src is shared by CMake host and embedded target.
No generated copies of the parser, fonts, sprite engine or renderer.
"""
Import("env")

from pathlib import Path
import re

project = Path(env.subst("$PROJECT_DIR")).resolve()
root = project.parent.parent
native = root / "native"
source = native / "src"

if not source.is_dir():
    raise RuntimeError(f"Missing shared C sources: {source}")

env.Append(CPPPATH=[
    str(native),
    str(native / "include"),
    str(source),
    str(project / "include"),
])

# Keep startup version identical to JavaFX/native simulator, derived from pom.
pom = (root / "pom.xml").read_text(encoding="utf-8")
version = re.search(
    r"<artifactId>simulated-display</artifactId>\s*<version>([^<]+)</version>",
    pom)
if version is None:
    raise RuntimeError("Cannot determine simulated-display version from pom.xml")
display_version = version.group(1).removesuffix("-SNAPSHOT")
env.Append(CPPDEFINES=[
    ("MATRIX_SOFTWARE_VERSION", f'\"v{display_version}\"'),
])

files = [
    "matrix_framebuffer.c",
    "matrix_pixel_font.c",
    "matrix_vbz_font.c",
    "matrix_numeric_font.c",
    "matrix_sprite.c",
    "matrix_status_icon.c",
    "matrix_lvgl.c",
    "matrix_font.c",
    "matrix_serial.c",
    "matrix_theme.c",
    "matrix_startup_screen.c",
    "matrix_race_screen.c",
]
env.BuildSources(
    "$BUILD_DIR/shared_native",
    str(source),
    src_filter=" ".join(["-<*>"] + [f"+<{name}>" for name in files]),
)
print(f"Arduino: sharing {len(files)} native C files, software version v{display_version}")
