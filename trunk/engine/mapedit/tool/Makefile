#	$Csoft: Makefile,v 1.4 2002/07/30 22:18:46 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c propedit.c select.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o propedit.o select.o

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
