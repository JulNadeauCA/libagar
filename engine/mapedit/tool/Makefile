#	$Csoft: Makefile,v 1.12 2003/06/21 06:50:24 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	eraser.c fill.c flip.c magnifier.c merge.c propedit.c \
	resize.c select.c shift.c stamp.c tool.c
OBJS=	eraser.o fill.o flip.o magnifier.o merge.o propedit.o \
	resize.o select.o shift.o stamp.o tool.o

DEN=		tool.den
DEN_DIR=	engine/mapedit/tool
DEN_NAME=	"Map editor tool icons"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2002, 2003 CubeSoft Communications, Inc."
XCF=		fill.xcf magnifier.xcf resize.xcf select.xcf

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
