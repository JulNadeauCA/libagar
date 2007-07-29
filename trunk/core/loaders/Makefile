TOP=	../..
include ${TOP}/Makefile.config

LIB=		ag_core_loaders
LIB_SHARED=	Yes
LIB_STATIC=	Yes

SRCS=	color.c den.c integral.c netbuf.c real.c string.c surface.c \
	version.c xcf.c

MAN3=	AG_Version.3
MAN5=	AG_XCF.5 AG_DEN.5

CFLAGS+=${SDL_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
