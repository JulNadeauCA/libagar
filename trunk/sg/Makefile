TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	pe

LIB=		ag_sg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	sg.c sg_math.c \
	sg_vector.c sg_vector2_fpu.c sg_vector3_fpu.c sg_vector4_fpu.c \
	sg_vector3_sse.c sg_vector3_sse3.c \
	sg_matrix.c sg_matrix44_fpu.c sg_matrix44_sse.c \
	sg_view.c sg_camera.c sg_gui.c sg_primitive.c sg_quat.c sg_geom.c \
	sg_edit.c sg_dummy.c sg_sphere.c sg_point.c sg_light.c \
	sg_solid.c sg_planeobj.c sg_object.c sg_box.c sg_matview.c \
	sg_material.c sg_voxel.c sg_program.c sg_cg_program.c \
	sg_load_ply.c \
	sk.c sk_view.c sk_tool.c sk_edit.c sk_select.c sk_solve.c \
	sk_dummy.c sk_point.c sk_line.c sk_circle.c sk_arc.c sk_constraint.c \
	sk_annot.c sk_dimension.c sk_measure.c

MAN3=	SG.3 SG_Camera.3 SG_Light.3 SG_View.3
CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS} ${MATH_CFLAGS}
CFLAGS+=${ALTIVEC_CFLAGS} ${SSE_CFLAGS} ${SSE2_CFLAGS} ${SSE3_CFLAGS}

# For gcc warnings in sg_matrix44_sse.c.
CFLAGS+=${WNO_UNINITIALIZED}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
