#	$Csoft: Makefile,v 1.8 2003/02/20 04:57:28 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c propedit.c select.c \
	shift.c merge.c fill.c flip.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o propedit.o select.o \
	shift.o merge.o fill.o flip.o

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
