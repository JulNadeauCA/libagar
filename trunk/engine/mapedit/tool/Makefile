#	$Csoft: Makefile,v 1.2 2002/07/09 08:24:48 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
