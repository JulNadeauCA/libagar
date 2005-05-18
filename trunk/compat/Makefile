#	$Csoft: Makefile,v 1.10 2005/05/01 00:11:12 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		compat
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c rwlock.c \
	gethostname.c snprintf.c arc4random.c dir.c md5.c sha1.c rmd160.c \
	math.c

CFLAGS+=-I../engine ${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
