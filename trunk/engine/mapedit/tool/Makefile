#	$Csoft: Makefile,v 1.9 2003/03/13 06:19:31 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c propedit.c select.c \
	shift.c merge.c fill.c flip.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o propedit.o select.o \
	shift.o merge.o fill.o flip.o

XCF=	fill.xcf magnifier.xcf resize.xcf select.xcf
FOBJ=	tool.fob
FOFFS=	tool_offs.h

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
