#	$Csoft: Makefile,v 1.9 2003/05/19 09:35:37 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	compat

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c gethostname.c snprintf.c

CFLAGS+=-I.. ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
