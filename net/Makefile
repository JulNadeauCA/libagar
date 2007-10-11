TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_net
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"c2e73d69-e1f4-4b92-a54f-03b1016b6357"

LIB_LINKS=	ag_core

SRCS=		command.c server.c client.c fgetln.c
# MAN3+=	AGN.3

CFLAGS+=-I.. ${SDL_CFLAGS} -D_GNU_SOURCE

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
