#	$Csoft: Makefile.inc,v 1.5 2002/02/18 03:53:04 vedge Exp $
#	ex:syn=make

CFLAGS?=
# XXX gcc
CFLAGS+=	-Wall -Werror -Wmissing-prototypes -Wunused
CFLAGS+=	-g -DDEBUG

SHAREDIR=	${PREFIX}/share/geggy
CFLAGS+=	-I${TOP} -DSHAREDIR=\"${SHAREDIR}\"

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/Makefile.config

