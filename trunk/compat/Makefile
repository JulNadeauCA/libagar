#	$Csoft: Makefile,v 1.3 2004/02/26 09:19:37 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		compat
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	pread.c pwrite.c gethostname.c snprintf.c arc4random.c

CFLAGS+=-I../engine ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
