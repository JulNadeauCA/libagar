#	$Csoft: Makefile,v 1.6 2002/11/22 23:00:53 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	compat

CFLAGS+=-I.. ${SDL_CFLAGS}

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c
OBJS=	setenv.o strlcat.o strlcpy.o strsep.o asprintf.o vasprintf.o rwlock.o \
	pread.o pwrite.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
