#	$Csoft$

VERSION?=	0.2

CFLAGS+=	-I${TOP}
CFLAGS+=	-DVERSION=\"${VERSION}\"
CFLAGS+=	-Wall -Werror -Wmissing-prototypes -Wunused -g -DDEBUG

include ${TOP}/libfobj/Makefile.inc
include ${TOP}/mk/inc.pthread.mk
include ${TOP}/mk/inc.sdl.mk
include ${TOP}/mk/inc.glib.mk
