#	$Csoft: Makefile,v 1.7 2003/03/13 22:43:54 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	compat

CFLAGS+=-I.. ${SDL_CFLAGS}

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c gethostname.c snprintf.c
OBJS=	setenv.o strlcat.o strlcpy.o strsep.o asprintf.o vasprintf.o rwlock.o \
	pread.o pwrite.o gethostname.o snprintf.o

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
