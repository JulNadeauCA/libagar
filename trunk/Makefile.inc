#	$Csoft: Makefile.inc,v 1.1.1.1 2002/01/25 09:50:02 vedge Exp $

VERSION?=	0.2

CFLAGS+=	-I${TOP}
CFLAGS+=	-DVERSION=\"${VERSION}\"
CFLAGS+=	-Wall -Werror -Wmissing-prototypes -Wunused -g -DDEBUG

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/Makefile.config
