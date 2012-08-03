TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc
include ${TOP}/au/Makefile.inc

PROJECT=	"agartest"
PROG_TYPE=	"GUI"
PROG_GUID=	"11D6C9FF-522E-43ed-B3EB-92A2C636CCA7"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${DEV_LINKS} ${AU_LINKS}

PROG=	agartest
SRCS=	agartest.c \
	audio.c \
	charsets.c \
	chinese.c \
	compositing.c \
	configsettings.c \
	console.c \
	customeventloop.c \
	customwidget.c \
	fixedres.c \
	focusing.c \
	glview.c \
	imageloading.c \
	keyevents.c \
	loader.c \
	maximized.c \
	minimal.c \
	modalwindowhandler.c \
	mywidget.c \
	network.c \
	objsystem.c \
	objsystem_animal.c \
	objsystem_mammal.c \
	pane.c \
	plotting.c \
	reinit.c \
	rendertosurface.c \
	scrollbar.c \
	scrollview.c \
	sdl.c \
	sockets.c \
	table.c \
	textbox.c \
	textdlg.c \
	themes.c \
	themes_mytheme.c \
	threads.c \
	timeouts.c \
	windows.c

CFLAGS+=${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
