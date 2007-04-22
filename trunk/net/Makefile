TOP=..
include ${TOP}/Makefile.config

LIB=		ag_net
LIB_SHARED=	Yes
LIB_STATIC=	No
CFLAGS+=	${AGAR_CFLAGS} -D_GNU_SOURCE

# MAN3+=	AGN.3
SRCS=		command.c server.c error.c client.c fgetln.c
INCLUDES=	error.h server.h client.h command.h

include .manlinks.mk
include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
