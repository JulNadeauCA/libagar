#	$Csoft: Makefile,v 1.13 2005/06/01 09:08:45 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_sg
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	sg.c sg_math.c sg_vector.c sg_matrix.c
#MAN3=	sg.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
