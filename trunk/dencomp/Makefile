TOP=	..
include ${TOP}/Makefile.config

PROG=		dencomp
PROG_TYPE=	"CLI"
PROG_GUID=	"792EDA21-89A8-4a8e-A226-C28F2810D689"
PROG_LINKS=	ag_core SDL opengl32 glu32

SRCS=	dencomp.c
MAN1=	dencomp.1

CFLAGS+=`agar-config --cflags`
LIBS+=	`agar-config --libs`

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
