TOP=	../..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc

PROG=		dencomp
PROG_TYPE=	"CLI"
PROG_GUID=	"792EDA21-89A8-4a8e-A226-C28F2810D689"
PROG_LINKS=	${CORE_LINKS}

SRCS=	dencomp.c
MAN1=	dencomp.1

CFLAGS+=`agar-core-config --cflags`
LIBS+=	`agar-core-config --libs`

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
