#	$Csoft: Makefile,v 1.4 2003/08/13 01:47:35 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

PROG=	dencomp
SRCS=	dencomp.c
MAN1=	dencomp.1

CFLAGS+=${SDL_CFLAGS}
LIBS+=	${TOP}/engine/loader/libloader.a \
	${TOP}/engine/error/liberror.a \
	${TOP}/compat/libcompat.a \
	${SDL_LIBS}

include ${TOP}/mk/csoft.prog.mk
include ${TOP}/mk/csoft.man.mk
