#	$Csoft: Makefile,v 1.4 2002/09/16 09:33:42 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	compat

CFLAGS+=-I.. ${SDL_CFLAGS}

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c vasprintf.c rwlock.c
OBJS=	setenv.o strlcat.o strlcpy.o strsep.o vasprintf.o rwlock.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
