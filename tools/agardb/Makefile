TOP=	../..
include Makefile.config

PROJECT=	"agardb"
#SUBDIR=	po

PROG=		agardb
PROG_TYPE=	"GUI"
PROG_GUID=	"883e7d87-8ab0-11e9-b0bb-ac9e17b8d45b"
PROG_LINKS=	ag_gui ag_core

SRCS=		agardb.cpp
#DATAFILES_SRC=	agardb.bmp
MAN1=		agardb.1

CFLAGS+=	${AGAR_CFLAGS} ${LLDB_CFLAGS} ${LLDB_UTILITY_CFLAGS}
CXXFLAGS+=	${AGAR_CFLAGS} ${LLDB_CFLAGS} ${LLDB_UTILITY_CFLAGS}
LIBS+=		${AGAR_LIBS} ${LLDB_LIBS} ${LLDB_UTILITY_LIBS}

all: all-subdir ${PROG}

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
