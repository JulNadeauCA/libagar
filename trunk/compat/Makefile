TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_compat
LIB_SHARED=	Yes
LIB_STATIC=	Yes
LIB_GUID=	"70d7394b-aee9-4a95-9a8f-386b3406d0d9"

SRCS=	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c \
	gethostname.c snprintf.c arc4random.c dir.c md5.c sha1.c rmd160.c \
	file.c

CFLAGS+=-I.. ${SDL_CFLAGS}

include ${TOP}/mk/build.lib.mk
