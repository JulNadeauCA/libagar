#	$Csoft: Makefile,v 1.2 2003/06/22 06:38:00 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

PROG=	denex
SRCS=	denex.c
OBJS=	denex.o

MAN1=	denex.1
CATMAN1=denex.cat1
PSMAN1=	denex.ps1

CFLAGS+=${SDL_CFLAGS}
LIBS+=	${TOP}/engine/loader/libloader.a \
	${TOP}/engine/error/liberror.a \
	${TOP}/engine/compat/libcompat.a \
	${SDL_LIBS}

include ${TOP}/mk/csoft.prog.mk
include ${TOP}/mk/csoft.man.mk
