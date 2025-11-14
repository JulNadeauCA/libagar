# Agar-VG Library
# Simple 2D vector graphics library

vg_sources = files(
  'vg.c',
  'vg_arc.c',
  'vg_arc_tool.c',
  'vg_circle.c',
  'vg_circle_tool.c',
  'vg_line.c',
  'vg_line_tool.c',
  'vg_point.c',
  'vg_point_tool.c',
  'vg_polygon.c',
  'vg_polygon_tool.c',
  'vg_proximity_tool.c',
  'vg_select_tool.c',
  'vg_snap.c',
  'vg_tables.c',
  'vg_text.c',
  'vg_text_tool.c',
  'vg_tool.c',
  'vg_view.c',
)

vg_deps = [
  libag_core_dep,
  libag_gui_dep,
  math_dep,
]

vg_c_args = global_defs + warning_flags + ['-D_AGAR_VG_INTERNAL']
# Include local headers and config_inc for generated config headers
vg_inc = [include_directories('.', '../core', '../gui'), config_inc]

# Build library (respects parent project's default_library option)
libag_vg = library('ag_vg',
  vg_sources,
  c_args: vg_c_args,
  include_directories: vg_inc,
  dependencies: vg_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
)

libag_vg_dep = declare_dependency(
  link_with: libag_vg,
  include_directories: vg_inc,
  dependencies: vg_deps,
)

# Override dependency for wrap support
meson.override_dependency('agar-vg', libag_vg_dep)

install_headers(
  'begin.h',
  'close.h',
  'icons.h',
  'icons_data.h',
  'nullability.h',
  'vg.h',
  'vg_arc.h',
  'vg_circle.h',
  'vg_line.h',
  'vg_math.h',
  'vg_point.h',
  'vg_polygon.h',
  'vg_pub.h',
  'vg_snap.h',
  'vg_text.h',
  'vg_tool.h',
  'vg_tools.h',
  'vg_view.h',
  subdir: 'agar/vg',
)
install_symlink(
  'vg.h',
  install_dir : 'include/agar',
  pointing_to: 'vg/vg_pub.h')

pkg.generate(
  libag_vg,
  name: 'agar-vg',
  description: 'Agar vector graphics library',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar'],
)

# Install manpages
vg_man3 = files(
  'VG.3',
  'VG_Arc.3',
  'VG_Circle.3',
  'VG_Line.3',
  'VG_Point.3',
  'VG_Polygon.3',
  'VG_Text.3',
  'VG_View.3',
)
install_man(vg_man3)
