TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_net
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=		command.c server.c error.c client.c fgetln.c
# MAN3+=	AGN.3

CFLAGS+=${SDL_CFLAGS} -D_GNU_SOURCE

include .manlinks.mk
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
