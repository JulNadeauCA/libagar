#	$Csoft$

TOP=	../..
include ${TOP}/Makefile.config

LIB=	widget
SRCS=	widget.c window.c text.c
OBJS=	widget.o window.o text.o
SHARE=	LICENSE.ttf \
	dignity.ttf kingrich.ttf larabie.ttf pakenham.ttf suigener.ttf \
	zekton.ttf zerothre.ttf zerotwos.ttf zrnic.ttf

CFLAGS+=	${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/libfobj/Makefile.inc
