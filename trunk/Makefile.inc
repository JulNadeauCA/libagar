#	$Csoft: Makefile.inc,v 1.3 2002/01/30 17:54:04 vedge Exp $

CFLAGS+=	-I${TOP}
CFLAGS+=	-Wall -Werror -Wmissing-prototypes -Wunused -g -DDEBUG

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/Makefile.config

LIB_SHAREDIR=	${PREFIX}/share/geggy
PROG_SHAREDIR=	${LIB_SHAREDIR}

CFLAGS+=	-DSHAREDIR=\"${LIB_SHAREDIR}\"

