#	$Csoft: Makefile,v 1.2 2003/06/21 07:14:17 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

PROG=	dencomp
SRCS=	dencomp.c
OBJS=	dencomp.o

MAN1=	dencomp.1
CATMAN1=dencomp.cat1
PSMAN1=	dencomp.ps1

CFLAGS+=${SDL_CFLAGS}
LIBS+=	${TOP}/engine/loader/libloader.a \
	${TOP}/engine/error/liberror.a \
	${TOP}/engine/compat/libcompat.a \
	${SDL_LIBS}

include ${TOP}/mk/csoft.prog.mk
include ${TOP}/mk/csoft.man.mk
