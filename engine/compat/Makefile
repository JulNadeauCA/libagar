#	$Csoft: Makefile,v 1.10 2003/08/13 01:47:34 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	compat

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c gethostname.c snprintf.c arc4random.c

CFLAGS+=-I.. ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
