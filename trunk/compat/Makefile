#	$Csoft: Makefile,v 1.8 2005/04/25 04:33:02 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		compat
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	gethostname.c snprintf.c arc4random.c dir.c md5.c

CFLAGS+=-I../engine ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
