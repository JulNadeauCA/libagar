#	$Csoft: Makefile,v 1.16 2003/08/13 01:47:35 vedge Exp $

TOP=	../../..
include ${TOP}/Makefile.config

LIB=	tool
SRCS=	eraser.c fill.c flip.c magnifier.c merge.c position.c propedit.c \
	resize.c select.c shift.c stamp.c tool.c

XCF=		fill.xcf magnifier.xcf resize.xcf select.xcf
DEN=		tool.den
DEN_DIR=	engine/mapedit/tool
DEN_NAME=	"Map edition tool cursors"
DEN_AUTHOR=	"vedge@"
DEN_COPYRIGHT=	"2002, 2003 CubeSoft Communications, Inc."

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/dencomp/Makefile.inc
