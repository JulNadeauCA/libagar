#	$Csoft: Makefile,v 1.2 2002/08/23 08:59:03 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

CFLAGS+=-I.. ${SDL_CFLAGS}

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c vasprintf.c
OBJS=	setenv.o strlcat.o strlcpy.o strsep.o vasprintf.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
