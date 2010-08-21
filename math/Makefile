TOP=	..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

LIB=		ag_math
LIB_MAJOR=	4
LIB_MINOR=	0
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"ba35a553-bd88-4e5e-8c4b-d17f60ae7439"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC}

SRCS=	m_math.c m_complex.c m_quaternion.c \
	m_vector.c m_vectorz.c m_vector_fpu.c \
	m_vector2_fpu.c m_vector3_fpu.c m_vector4_fpu.c \
	m_vector3_sse.c m_vector3_sse3.c \
	m_matrix.c m_matrix_fpu.c m_matrix44_fpu.c m_matrix44_sse.c \
	m_gaussj.c m_lu.c \
	m_gui.c m_plotter.c m_matview.c \
	m_line.c m_circle.c m_triangle.c m_rectangle.c m_polygon.c m_plane.c \
	m_coordinates.c m_heapsort.c m_mergesort.c m_qsort.c m_radixsort.c \
	m_point_set.c m_color.c m_sphere.c \
	m_matrix_sparse.c \
	m_sparse_allocate.c \
	m_sparse_build.c \
	m_sparse_eda.c \
	m_sparse_factor.c \
	m_sparse_output.c \
	m_sparse_solve.c \
	m_sparse_utils.c

MAN3=	M_Matrix.3 M_Circle.3 M_Color.3 M_Complex.3 M_Geometry.3 M_Line.3 \
	M_Plane.3 M_Polygon.3 M_Rectangle.3 M_Sphere.3 M_Triangle.3 \
	M_Matview.3 M_Real.3 M_Plotter.3 M_Quaternion.3 M_Vector.3 \
	M_Label.3

CFLAGS+=-D_AGAR_MATH_INTERNAL ${SDL_CFLAGS} ${PTHREADS_XOPEN_CFLAGS} \
	${ALTIVEC_CFLAGS} ${SSE_CFLAGS} ${SSE2_CFLAGS} ${SSE3_CFLAGS} \
	${MATH_CFLAGS}

# For gcc warnings in m_matrix44_sse.c.
CFLAGS+=${WNO_UNINITIALIZED}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
