#	$Csoft: Makefile,v 1.11 2003/12/31 01:55:15 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=	compat

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c gethostname.c snprintf.c arc4random.c

CFLAGS+=-I../engine ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
