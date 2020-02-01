TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/sg/Makefile.inc

LIB=		ag_sg
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"ea398f34-6e0e-4436-aae6-08f47550f52d"
LIB_LINKS=	${AGMATH_LINKS_STATIC} ${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC} \
		${SG_LIBS}

MAN3=	SG.3 SG_Camera.3 SG_CgProgram.3 SG_Light.3 SG_Texture.3 SG_Node.3 \
	SG_Object.3 SG_Plane.3 SG_Point.3 SG_Program.3 SG_PolyBall.3 \
	SG_View.3 SG_Voxel.3 SG_Image.3 SG_Intro.3 SG_Line.3

SRCS=	sg.c sg_node.c sg_view.c sg_camera.c sg_gui.c sg_edit.c sg_dummy.c \
        sg_point.c sg_light.c sg_plane.c sg_object.c sg_texture.c \
	sg_voxel.c sg_program.c sg_load_ply.c sg_image.c sg_script.c \
	sg_line.c sg_geom.c sg_circle.c sg_polygon.c sg_triangle.c \
	sg_rectangle.c sg_sphere.c sg_polyball.c sg_polybox.c sg_action.c \
	sg_widget.c sg_palette.c sg_palette_view.c

#SRCS+=	sg_cg_program.c

CFLAGS+=${AGMATH_CFLAGS} \
	${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${SG_CFLAGS} -D_AGAR_SG_INTERNAL

LIBS=	-L../math -lag_math ${AGMATH_LIBS} \
	-L../gui  -lag_gui  ${GUI_LIBS} \
	-L../core -lag_core ${CORE_LIBS} \
	${SG_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
