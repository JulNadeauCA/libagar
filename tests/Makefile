TOP=	..
include Makefile.config
include Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/au/Makefile.inc

PROJECT=	"agartest"
PROG_TYPE=	"GUI"
PROG_GUID=	"11D6C9FF-522E-43ed-B3EB-92A2C636CCA7"

PROG_LINKS=	${AGMATH_LINKS} ${DEV_LINKS} ${GUI_LINKS} ${CORE_LINKS}
CFLAGS+=	${AGAR_MATH_CFLAGS} ${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_MATH_LIBS} ${AGAR_DEV_LIBS} ${AGAR_LIBS}

DATAFILES=	loss.txt \
		agar.png agar-index.png agar.bmp \
		axe.bmp \
		helmet.png helmet.bmp \
		helmet-socket.bmp \
		menubg.bmp \
		pepe.jpg \
		sword-socket.bmp \
		sword.bmp \
		axe.png

PROG=	agartest
SRCS=	agartest.c ${SRCS_EXTRA} \
	charsets.c \
	compositing.c \
	configsettings.c \
	console.c \
	customwidget.c \
	customwidget_mywidget.c \
	fixedres.c \
	focusing.c \
	fontselector.c \
	fspaths.c \
	glview.c \
	imageloading.c \
	keyevents.c \
	loader.c \
	math.c \
	maximized.c \
	minimal.c \
	modalwindowhandler.c \
	network.c \
	objsystem.c \
	objsystem_animal.c \
	objsystem_mammal.c \
	pane.c \
	palette.c \
	plotting.c \
	rendertosurface.c \
	scrollbar.c \
	scrollview.c \
	sockets.c \
	string.c \
	table.c \
	textbox.c \
	textdlg.c \
	threads.c \
	timeouts.c \
	unitconv.c \
	widgets.c \
	windows.c

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
