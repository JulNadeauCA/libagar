#	$Csoft: Makefile,v 1.64 2004/04/11 03:29:19 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

SUBDIR=	tool \
	mapview

LIB=		mapedit
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	layedit.c mapedit.c mapview.c mediasel.c objedit.c tool.c
MAN3=	mapview.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
