#	$Csoft: Makefile.inc,v 1.2 2002/01/26 20:17:34 vedge Exp $

CFLAGS+=	-I${TOP}
CFLAGS+=	-Wall -Werror -Wmissing-prototypes -Wunused -g -DDEBUG

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/Makefile.config
