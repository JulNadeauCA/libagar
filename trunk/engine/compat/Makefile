#	$Csoft: Makefile,v 1.5 2002/09/19 20:57:09 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	compat

CFLAGS+=-I.. ${SDL_CFLAGS}

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c
OBJS=	setenv.o strlcat.o strlcpy.o strsep.o asprintf.o vasprintf.o rwlock.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
