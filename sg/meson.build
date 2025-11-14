# Agar-SG Library
# 3D Scene graph engine (requires Math, GUI, and OpenGL)

# Get all C source files
sg_sources = files(
  'sg.c',
  'sg_action.c',
  'sg_bsp.c',
  'sg_camera.c',
  'sg_cg_program.c',
  'sg_circle.c',
  'sg_dummy.c',
  'sg_edit.c',
  'sg_geom.c',
  'sg_gui.c',
  'sg_image.c',
  'sg_light.c',
  'sg_line.c',
  'sg_load_ply.c',
  'sg_node.c',
  'sg_object.c',
  'sg_plane.c',
  'sg_point.c',
  'sg_polyball.c',
  'sg_polybox.c',
  'sg_polygon.c',
  'sg_program.c',
  'sg_rectangle.c',
  'sg_script.c',
  'sg_sphere.c',
  'sg_texture.c',
  'sg_triangle.c',
  'sg_view.c',
  'sg_voxel.c',
  'sg_widget.c',
)

sg_deps = [
  libag_core_dep,
  libag_gui_dep,
  libag_math_dep,
  gl_dep,
]

if get_option('glu') and glu_dep.found()
  sg_deps += glu_dep
endif

sg_c_args = global_defs + warning_flags + ['-D_AGAR_SG_INTERNAL']
# Include local headers and config_inc for generated config headers
sg_inc = [include_directories('.', '../core', '../gui', '../math'), config_inc]

# Build library (respects parent project's default_library option)
libag_sg = library('ag_sg',
  sg_sources,
  c_args: sg_c_args,
  include_directories: sg_inc,
  dependencies: sg_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
)

libag_sg_dep = declare_dependency(
  link_with: libag_sg,
  include_directories: sg_inc,
  dependencies: sg_deps,
)

# Override dependency for wrap support
meson.override_dependency('agar-sg', libag_sg_dep)

# Install headers
install_headers(
  'begin.h',
  'close.h',
  'icons.h',
  'icons_data.h',
  'nullability.h',
  'sg.h',
  'sg_camera.h',
  'sg_cg_program.h',
  'sg_circle.h',
  'sg_dummy.h',
  'sg_geom.h',
  'sg_gui.h',
  'sg_image.h',
  'sg_light.h',
  'sg_line.h',
  'sg_load_ply.h',
  'sg_object.h',
  'sg_plane.h',
  'sg_point.h',
  'sg_polyball.h',
  'sg_polybox.h',
  'sg_polygon.h',
  'sg_program.h',
  'sg_pub.h',
  'sg_rectangle.h',
  'sg_script.h',
  'sg_sphere.h',
  'sg_texture.h',
  'sg_triangle.h',
  'sg_view.h',
  'sg_voxel.h',
  'sg_widget.h',
  subdir: 'agar/sg',
)
install_symlink(
  'sg.h',
  install_dir : 'include/agar',
  pointing_to: 'sg/sg_pub.h')

pkg.generate(
  libag_sg,
  name: 'agar-sg',
  description: 'Agar 3D scene graph library',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar-math', 'agar'],
)

# Install manpages
sg_man3 = files(
  'SG.3',
  'SG_Camera.3',
  'SG_CgProgram.3',
  'SG_Foo.3',
  'SG_Image.3',
  'SG_Intro.3',
  'SG_Light.3',
  'SG_Line.3',
  'SG_Node.3',
  'SG_Object.3',
  'SG_Plane.3',
  'SG_Point.3',
  'SG_PolyBall.3',
  'SG_Program.3',
  'SG_Texture.3',
  'SG_View.3',
  'SG_Voxel.3',
)
install_man(sg_man3)
