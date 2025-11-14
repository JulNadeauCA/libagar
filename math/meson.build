# Agar-Math Library
# Math library with matrices, vectors, and advanced rendering

math_sources = files(
  'm_math.c',
  'm_complex.c',
  'm_quaternion.c',
  'm_vector.c',
  'm_vectorz.c',
  'm_vector_fpu.c',
  'm_vector2_fpu.c',
  'm_vector3_fpu.c',
  'm_vector4_fpu.c',
  'm_matrix.c',
  'm_matrix_fpu.c',
  'm_matrix44_fpu.c',
  'm_gui.c',
  'm_plotter.c',
  'm_matview.c',
  'm_line.c',
  'm_circle.c',
  'm_triangle.c',
  'm_rectangle.c',
  'm_polygon.c',
  'm_plane.c',
  'm_coordinates.c',
  'm_heapsort.c',
  'm_mergesort.c',
  'm_qsort.c',
  'm_radixsort.c',
  'm_point_set.c',
  'm_color.c',
  'm_sphere.c',
  'm_polyhedron.c',
  'm_matrix_sparse.c',
  'm_sparse_allocate.c',
  'm_sparse_build.c',
  'm_sparse_eda.c',
  'm_sparse_factor.c',
  'm_sparse_output.c',
  'm_sparse_solve.c',
  'm_sparse_utils.c',
  'm_bezier.c',
  'm_bezier_primitives.c',
)

# Add SSE-optimized sources if SSE is enabled
if get_option('sse') and have_sse
  math_sources += files(
    'm_vector3_sse.c',
    'm_matrix44_sse.c',
  )
endif

# Math library dependencies
math_deps = [
  libag_core_dep,
  libag_gui_dep,
  math_dep,
]

# Compile flags
math_c_args = global_defs + warning_flags + [
  '-D_AGAR_MATH_INTERNAL',
]

# Add SSE flags if enabled
if get_option('sse') and have_sse
  math_c_args += sse_args
endif

# Add AltiVec flags if enabled
if have_altivec
  math_c_args += altivec_args
endif

# Include directories
# Include local headers and config_inc for generated config headers
math_inc = [include_directories('.', '../core', '../gui'), config_inc]

# Build library (respects parent project's default_library option)
libag_math = library('ag_math',
  math_sources,
  c_args: math_c_args,
  include_directories: math_inc,
  dependencies: math_deps,
  version: agar_soversion,
  soversion: agar_soversion_major,
  install: true,
)

# Declare dependency
libag_math_dep = declare_dependency(
  link_with: libag_math,
  include_directories: math_inc,
  dependencies: math_deps,
)

# Override dependency for wrap support
meson.override_dependency('agar-math', libag_math_dep)

# Install headers
install_headers(
  'begin.h',
  'close.h',
  'gl_macros.h',
  'm.h',
  'm_math.h',
  'm_complex.h',
  'm_quaternion.h',
  'm_vector.h',
  'm_vectorz.h',
  'm_vector_fpu.h',
  'm_vector2_fpu.h',
  'm_vector3_fpu.h',
  'm_vector3_sse.h',
  'm_vector4_fpu.h',
  'm_matrix.h',
  'm_matrix_fpu.h',
  'm_matrix44_fpu.h',
  'm_matrix44_sse.h',
  'm_gui.h',
  'm_plotter.h',
  'm_matview.h',
  'm_line.h',
  'm_circle.h',
  'm_triangle.h',
  'm_rectangle.h',
  'm_polygon.h',
  'm_plane.h',
  'm_coordinates.h',
  'm_point_set.h',
  'm_color.h',
  'm_sphere.h',
  'm_polyhedron.h',
  'm_matrix_sparse.h',
  'm_sparse.h',
  'm_bezier.h',
  'm_bezier_primitives.h',
  'm_bitstring.h',
  'm_geometry.h',
  subdir: 'agar/math',
)

# Generate pkg-config file
pkg.generate(
  libag_math,
  name: 'agar-math',
  description: 'Agar math library',
  version: agar_version,
  subdirs: 'agar',
  requires: ['agar-core', 'agar'],
)

# Install manpages
math_man3 = files(
  'M_Circle.3',
  'M_Color.3',
  'M_Complex.3',
  'M_Geometry.3',
  'M_Line.3',
  'M_Matrix.3',
  'M_Matview.3',
  'M_Plane.3',
  'M_Plotter.3',
  'M_PointSet.3',
  'M_Polygon.3',
  'M_Quaternion.3',
  'M_Real.3',
  'M_Rectangle.3',
  'M_Sphere.3',
  'M_String.3',
  'M_Triangle.3',
  'M_Vector.3',
  'M_VectorZ.3',
)
install_man(math_man3)
