TOP=	..
include ${TOP}/Makefile.config

PROG=	denuncomp
SRCS=	denuncomp.c
MAN1=	denuncomp.1

CFLAGS+=`agar-config --cflags`
LIBS+=	`agar-config --libs`

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
