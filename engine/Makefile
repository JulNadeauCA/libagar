#	$Csoft: Makefile,v 1.2 2002/01/26 03:37:54 vedge Exp $

TOP=	..
SUBDIR=	mapedit

.PATH:	mapedit

LIB=	engine
SRCS=	event.c object.c char.c world.c map.c xcf.c view.c audio.c video.c \
	mapedit.c
OBJS=	event.o object.o char.o world.o map.o xcf.o view.o audio.o video.o \
	mapedit.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/inc.smpeg.mk
include ${TOP}/mk/inc.x11.mk
include ${TOP}/Makefile.inc
