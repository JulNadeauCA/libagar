#	$Csoft: Makefile,v 1.3 2002/07/12 02:53:12 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c propedit.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o propedit.o

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
