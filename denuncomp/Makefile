TOP=	..
include ${TOP}/Makefile.config

PROG=		denuncomp
PROG_TYPE=	"CLI"
PROG_GUID=	"3B69DB91-66D5-4d11-B77E-D83A26372A7D"

SRCS=	denuncomp.c
MAN1=	denuncomp.1

CFLAGS+=`agar-config --cflags`
LIBS+=	`agar-config --libs`

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
