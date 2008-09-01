TOP=	..
include ${TOP}/Makefile.config

SUBDIR=		SPARSE
LIBS=		SPARSE/*.lo

LIB=		ag_math
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"ba35a553-bd88-4e5e-8c4b-d17f60ae7439"
LIB_LINKS=	ag_core_static ag_gui_static \
		pthreads SDL opengl freetype

SRCS=	m_math.c m_complex.c m_quaternion.c \
	m_int_vector.c m_vector.c m_vector_fpu.c \
	m_vector2_fpu.c m_vector3_fpu.c m_vector4_fpu.c \
	m_vector3_sse.c m_vector3_sse3.c \
	m_matrix.c m_matrix_fpu.c m_matrix44_fpu.c m_matrix44_sse.c \
	m_gaussj.c m_lu.c \
	m_gui.c m_plotter.c m_matview.c \
	m_line.c m_circle.c m_triangle.c m_rectangle.c m_polygon.c m_plane.c \
	m_coordinates.c m_heapsort.c m_mergesort.c m_qsort.c m_radixsort.c \
	m_point_set.c m_color.c \
	m_matrix_sparse.c

LIB_XOBJS=SPARSE

MAN3=	M_Matrix.3

CFLAGS+=${SDL_CFLAGS} ${PTHREADS_XOPEN_CFLAGS} ${ALTIVEC_CFLAGS} ${SSE_CFLAGS} \
	${SSE2_CFLAGS} ${SSE3_CFLAGS} ${GLU_CFLAGS} ${MATH_CFLAGS} \
	-D_M_INTERNAL

# For gcc warnings in m_matrix44_sse.c.
CFLAGS+=${WNO_UNINITIALIZED}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
