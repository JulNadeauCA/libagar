#	$Csoft: Makefile,v 1.11 2003/06/06 02:18:04 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	tool.c stamp.c eraser.c magnifier.c resize.c propedit.c select.c \
	shift.c merge.c fill.c flip.c
OBJS=	tool.o stamp.o eraser.o magnifier.o resize.o propedit.o select.o \
	shift.o merge.o fill.o flip.o

DEN=		tool.den
DEN_DIR=	engine/mapedit/tool
DEN_NAME=	"Map editor tool icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2002, 2003 CubeSoft Communications, Inc."
XCF=		fill.xcf magnifier.xcf resize.xcf select.xcf

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
