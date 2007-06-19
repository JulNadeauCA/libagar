TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	import

LIB=		ag_sg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	sg.c sg_edit.c sg_math.c sg_vector.c sg_matrix.c sg_view.c \
	sg_camera.c sg_gui.c sg_primitive.c sg_quat.c sg_plane.c \
	sg_spherical.c sg_dummy.c sg_sphere.c sg_point.c sg_light.c \
	sg_solid.c sg_planeobj.c sg_object.c sg_box.c sg_matview.c \
	sg_material.c sg_voxel.c  \
	sk.c sk_view.c sk_tool.c sk_dummy.c sk_edit.c sk_point.c \
	sk_line.c sk_circle.c sk_arc.c

MAN3=	SG.3 SG_Camera.3 SG_Light.3 SG_View.3
CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
