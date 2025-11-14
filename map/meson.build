# Agar-MAP Library
# 2D tile engine (requires GUI)

# Get all C source files from the map directory
map_sources = files(
  'eraser.c',
  'fill.c',
  'flip.c',
  'insert.c',
  'insert_obj.c',
  'invert.c',
  'map.c',
  'map_img.c',
  'map_link.c',
  'map_object.c',
  'map_tile.c',
  'mapview.c',
  'nodesel.c',
  'rg_feature.c',
  'rg_fill.c',
  'rg_pixmap.c',
  'rg_prim.c',
#  'rg_sketch.c',
#  'rg_sketch_circle.c',
#  'rg_sketch_line.c',
#  'rg_sketch_polygon.c',
#  'rg_sketchproj.c',
  'rg_texture.c',
  'rg_tile.c',
  'rg_tileset.c',
  'rg_tileview.c',
  'rg_transform.c',
  'tool.c',
)

map_deps = [
  libag_core_dep,
  libag_gui_dep,
  libag_math_dep,
  math_dep,
]

map_c_args = global_defs + warning_flags + [ '-D_AGAR_MAP_INTERNAL', '-D_AGAR_RG_INTERNAL' ]
# Include local headers and config_inc for generated config headers
map_inc = [include_directories('.', '../core', '../gui'), config_inc]

if get_option('static')
  static_pic = get_option('static_pic') and get_option('shared')

  libag_map_static = static_library('ag_map',
    map_sources,
    c_args: map_c_args,
    include_directories: map_inc,
    dependencies: map_deps,
    install: true,
    pic: static_pic,
  )
endif

if get_option('shared')
  libag_map_shared = shared_library('ag_map',
    map_sources,
    c_args: map_c_args,
    include_directories: map_inc,
    dependencies: map_deps,
    version: agar_soversion,
    soversion: agar_soversion_major,
    install: true,
  )
  libag_map = libag_map_shared
else
  libag_map = libag_map_static
endif

libag_map_dep = declare_dependency(
  link_with: libag_map,
  include_directories: map_inc,
  dependencies: map_deps,
)

install_headers(
  'begin.h',
  'close.h',
  'icons.h',
  'icons_data.h',
  'insert.h',
  'map.h',
  'map_img.h',
  'map_link.h',
  'map_math.h',
  'map_object.h',
  'map_pub.h',
  'map_tile.h',
  'mapview.h',
  'nodesel.h',
  'nullability.h',
  'rg.h',
  'rg_feature.h',
  'rg_fill.h',
  'rg_icons.h',
  'rg_icons_data.h',
  'rg_math.h',
  'rg_pixmap.h',
  'rg_prim.h',
  'rg_sketch.h',
  'rg_sketchproj.h',
  'rg_texture.h',
  'rg_tile.h',
  'rg_tileset.h',
  'rg_tileview.h',
  'rg_transform.h',
  'tool.h',
  'tools.h',
  subdir: 'agar/map',
)
install_symlink(
  'map.h',
  install_dir : 'include/agar',
  pointing_to: 'map/map_pub.h')

pkg.generate(
  libag_map,
  name: 'agar-map',
  description: 'Agar 2D tile engine',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar-math', 'agar'],
)

# Install manpages
map_man3 = files(
  'MAP.3',
  'MAP_View.3',
  'RG_Feature.3',
  'RG_Pixmap.3',
  'RG_Tile.3',
  'RG_Tileset.3',
  'RG_Tileview.3',
)
install_man(map_man3)
