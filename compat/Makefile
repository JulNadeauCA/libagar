#	$Csoft: Makefile,v 1.5 2004/04/23 10:50:52 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		compat
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c gethostname.c snprintf.c arc4random.c dir.c litoa.c

CFLAGS+=-I../engine ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
