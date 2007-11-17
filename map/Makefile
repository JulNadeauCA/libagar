TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_map
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"da8b3181-d485-4ef0-a68a-b747f08b5745"
LIB_LINKS=	ag_core ag_gui ag_rg ag_vg SDL opengl32 glu32

SRCS=	fill.c flip.c invert.c map.c mapedit.c mapview.c nodemask.c \
	nodesel.c refsel.c tool.c insert.c ginsert.c eraser.c actor.c
MAN3=	MAP.3 MAP_Actor.3 MAP_View.3

CFLAGS+=-I.. ${SDL_CFLAGS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
