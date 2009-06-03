TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/rg/Makefile.inc
include ${TOP}/vg/Makefile.inc
include ${TOP}/dev/Makefile.inc

PROJECT=	"agarpaint"
#SUBDIR=	po

PROG=		agarpaint
PROG_TYPE=	"GUI"
PROG_GUID=	"68DBFDA9-4965-4cca-8348-38409D8587A6"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${RG_LINKS} ${VG_LINKS} ${DEV_LINKS}

SRCS=	agarpaint.c
SHARE=	agarpaint.bmp

CFLAGS+=${AGAR_CFLAGS} ${AGAR_RG_CFLAGS} ${AGAR_VG_CFLAGS} \
	${AGAR_DEV_CFLAGS} ${GETTEXT_CFLAGS}
LIBS+=	${AGAR_LIBS} ${AGAR_RG_LIBS} ${AGAR_VG_LIBS} \
	${AGAR_DEV_LIBS} ${GETTEXT_CFLAGS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
