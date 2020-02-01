TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc

LIB=		ag_net
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"d486c040-24ad-11ea-b600-ac9e17b8d45b"
LIB_LINKS=	${CORE_LINKS_STATIC} ${NET_LIBS}

MAN3=	AG_Net.3 AG_Web.3

SRCS=	net.c net_dummy.c ${SRCS_NET}

CFLAGS+=${CORE_CFLAGS} ${NET_CFLAGS} -D_AGAR_NET_INTERNAL

LIBS=	-L../core -lag_core ${CORE_LIBS} \
	${NET_LIBS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
