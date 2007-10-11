TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_intl
LIB_GUID=	"450f5d7c-148c-4934-91d5-833fa30d8d89"

SRCS=	gettext.c textdomain.c
#MAN3=	gettext.3

CFLAGS+=-I.. ${SDL_CFLAGS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
