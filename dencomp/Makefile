#	$Csoft: Makefile,v 1.4 2003/08/13 01:47:35 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

PROG=	dencomp
SRCS=	dencomp.c
MAN1=	dencomp.1

CFLAGS+=`agar-config --cflags`
LIBS+=	`agar-config --libs`

include ${TOP}/mk/csoft.prog.mk
include ${TOP}/mk/csoft.man.mk
