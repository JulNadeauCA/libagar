#	$Csoft: Makefile.inc,v 1.4 2002/02/18 02:01:01 vedge Exp $

CFLAGS+=	-I${TOP}
CFLAGS+=	-Wall -Werror -Wmissing-prototypes -Wunused -g -DDEBUG

SHAREDIR=	${PREFIX}/share/geggy
CFLAGS+=	-DSHAREDIR=\"${SHAREDIR}\"

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/Makefile.config

