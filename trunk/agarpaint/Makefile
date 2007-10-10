TOP=	..
include Makefile.config

PROJECT=	"AgarPaint"
PROG=		agarpaint
PROG_TYPE=	"GUI"
PROG_GUID=	"68DBFDA9-4965-4cca-8348-38409D8587A6"

SRCS=	agarpaint.c
SHARE=	agarpaint.bmp

CFLAGS+=${AGAR_CFLAGS} ${AGAR_MAP_CFLAGS} ${AGAR_RG_CFLAGS} ${AGAR_VG_CFLAGS} ${AGAR_DEV_CFLAGS} -I.
LIBS+=	${AGAR_LIBS} ${AGAR_MAP_LIBS} ${AGAR_RG_LIBS} ${AGAR_VG_LIBS} ${AGAR_DEV_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
