TOP=	..
include ${TOP}/Makefile.config

PROG=	dencomp
SRCS=	dencomp.c
MAN1=	dencomp.1

CFLAGS+=`agar-config --cflags`
LIBS+=	`agar-config --libs`

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
