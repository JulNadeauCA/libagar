#	$Csoft: Makefile,v 1.6 2003/01/25 06:29:30 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c propedit.c select.c \
	shift.c merge.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o propedit.o select.o \
	shift.o merge.o

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
