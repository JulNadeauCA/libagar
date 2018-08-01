TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc

USE_LIBTOOL=	No

LIB=		ag_ada_core
LIB_CURRENT=	1
LIB_REVISION=	0
LIB_AGE=	0
LIB_SHARED=	Yes
LIB_INSTALL=	Yes
LIB_GUID=	"49d2fdbb-713c-11e8-b63b-ac9e17b8d45b"
LIB_LINKS=	${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_CORE_CFLAGS}
LIBS+=		${AGAR_CORE_LIBS}

SRCS_GENERATED= agar-data_source.ads \
		agar-dso.ads \
		agar-event.ads \
		agar-object.ads \
		agar-timer.ads

SRCS=		ag_data_source.c \
		ag_dso.c \
		ag_event.c \
		ag_object.c \
		agar.ads \
		agar-config.adb \
		agar-data_source.adb \
		agar-dso.adb \
		agar-error.adb \
		agar-event.adb \
		agar-init.adb \
		agar-object.adb \
		agar-timer.adb

CONFIGSCRIPTS=	agar-ada-core-config
PCMODULES=	agar-ada-core.pc

TYPEMAP=	agar-core.types
TYPESUBST=	${SH} ${TOP}/mk/typesubst.sh

DATAFILES=	${TYPEMAP} \
		LICENSE

CLEANFILES=	${SRCS_GENERATED}

include ${TOP}/mk/build.lib.mk

agar-object.ads: agar-object.ads.in ${TYPEMAP}
	${TYPESUBST} agar-object.ads.in ${TYPEMAP} > $@

agar-data_source.ads: agar-data_source.ads.in ${TYPEMAP}
	${TYPESUBST} agar-data_source.ads.in ${TYPEMAP} > $@

agar-dso.ads: agar-dso.ads.in ${TYPEMAP}
	${TYPESUBST} agar-dso.ads.in ${TYPEMAP} > $@

agar-event.ads: agar-event.ads.in ${TYPEMAP}
	${TYPESUBST} agar-event.ads.in ${TYPEMAP} > $@

agar-timer.ads: agar-timer.ads.in ${TYPEMAP}
	${TYPESUBST} agar-timer.ads.in ${TYPEMAP} > $@

agar-variable.ads: agar-variable.ads.in ${TYPEMAP}
	${TYPESUBST} agar-variable.ads.in ${TYPEMAP} > $@

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure
