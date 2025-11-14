# Agar-SK Library
# Sketching engine with constraint solver (requires Math, GUI, and OpenGL)

sk_sources = files(
  'sk.c',
  'sk_annot.c',
  'sk_arc.c',
  'sk_circle.c',
  'sk_dimension.c',
  'sk_dummy.c',
  'sk_edit.c',
  'sk_geometry.c',
  'sk_group.c',
  'sk_gui.c',
  'sk_intersect.c',
  'sk_line.c',
  'sk_measure.c',
  'sk_pixmap.c',
  'sk_placement.c',
  'sk_point.c',
  'sk_polygon.c',
  'sk_select.c',
  'sk_solve.c',
  'sk_tool.c',
  'sk_view.c',
)

sk_deps = [
  libag_core_dep,
  libag_gui_dep,
  libag_math_dep,
  gl_dep,
]

sk_c_args = global_defs + warning_flags + ['-D_AGAR_SK_INTERNAL']
# Include local headers and config_inc for generated config headers
sk_inc = [include_directories('.', '../core', '../gui', '../math'), config_inc]

# Build library (respects parent project's default_library option)
libag_sk = library('ag_sk',
  sk_sources,
  c_args: sk_c_args,
  include_directories: sk_inc,
  dependencies: sk_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
)

libag_sk_dep = declare_dependency(
  link_with: libag_sk,
  include_directories: sk_inc,
  dependencies: sk_deps,
)

# Override dependency for wrap support
meson.override_dependency('agar-sk', libag_sk_dep)

install_headers(
  'begin.h',
  'close.h',
  'nullability.h',
  'sk.h',
  'sk_annot.h',
  'sk_arc.h',
  'sk_circle.h',
  'sk_dimension.h',
  'sk_dummy.h',
  'sk_group.h',
  'sk_gui.h',
  'sk_line.h',
  'sk_pixmap.h',
  'sk_placement.h',
  'sk_point.h',
  'sk_polygon.h',
  'sk_pub.h',
  'sk_tool.h',
  'sk_view.h',
  subdir: 'agar/sk',
)
install_symlink(
  'sk.h',
  install_dir : 'include/agar',
  pointing_to: 'sk/sk_pub.h')

pkg.generate(
  libag_sk,
  name: 'agar-sk',
  description: 'Agar sketch engine library',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar-math', 'agar'],
)

# Install manpages
sk_man3 = files(
  'SK.3',
  'SK_View.3',
)
install_man(sk_man3)
