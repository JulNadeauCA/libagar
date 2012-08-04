TOP=	..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc
include ${TOP}/math/Makefile.inc

PROJECT=	"agartest"
PROG_TYPE=	"GUI"
PROG_GUID=	"11D6C9FF-522E-43ed-B3EB-92A2C636CCA7"
PROG_LINKS=	${CORE_LINKS} ${MATH_LINKS} ${GUI_LINKS} ${DEV_LINKS}

PROG=	agartest
SRCS=	agartest.c ${SRCS_EXTRA} \
	charsets.c \
	chinese.c \
	compositing.c \
	configsettings.c \
	console.c \
	customwidget.c \
	customwidget_mywidget.c \
	fixedres.c \
	focusing.c \
	glview.c \
	imageloading.c \
	keyevents.c \
	loader.c \
	maximized.c \
	minimal.c \
	modalwindowhandler.c \
	network.c \
	objsystem.c \
	objsystem_animal.c \
	objsystem_mammal.c \
	pane.c \
	plotting.c \
	rendertosurface.c \
	scrollbar.c \
	scrollview.c \
	sockets.c \
	table.c \
	textbox.c \
	textdlg.c \
	themes.c \
	themes_mytheme.c \
	threads.c \
	timeouts.c \
	unitconv.c \
	windows.c

CFLAGS+=${AGAR_AU_CFLAGS} ${AGAR_MATH_CFLAGS} ${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS}
LIBS+=	${AGAR_AU_LIBS} ${AGAR_MATH_LIBS} ${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
