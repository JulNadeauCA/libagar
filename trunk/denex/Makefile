#	$Csoft: Makefile,v 1.3 2003/06/25 00:30:55 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

PROG=	denex
SRCS=	denex.c
MAN1=	denex.1

CFLAGS+=${SDL_CFLAGS}
LIBS+=	${TOP}/engine/loader/libloader.a \
	${TOP}/engine/error/liberror.a \
	${TOP}/engine/compat/libcompat.a \
	${SDL_LIBS}

include ${TOP}/mk/csoft.prog.mk
include ${TOP}/mk/csoft.man.mk
