TOP=	..
include Makefile.config
include Makefile.proj
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/au/Makefile.inc

PROJECT=	"agartest"

PROG=		agartest
PROG_TYPE=	"GUI"
PROG_GUID=	"11d6c9ff-522e-43ed-b3eb-92a2c636cca7"
PROG_LINKS=	${AGMATH_LINKS} ${GUI_LINKS} ${CORE_LINKS}

CFLAGS+=	${AGAR_AU_CFLAGS} ${AGAR_MATH_CFLAGS} ${AGAR_CFLAGS}
LIBS+=		${AGAR_AU_LIBS} ${AGAR_MATH_LIBS} ${AGAR_LIBS}

SRCS=	agartest.c ${SRCS_AUDIO} ${SRCS_MATH} \
	buttons.c \
	charsets.c \
	checkbox.c \
	compositing.c \
	configsettings.c \
	console.c \
	customwidget.c \
	customwidget_mywidget.c \
	fixedres.c \
	focusing.c \
	fonts.c \
	fspaths.c \
	glview.c \
	imageloading.c \
	keyevents.c \
	loader.c \
	maximized.c \
	minimal.c \
	network.c \
	objsystem.c \
	objsystem_animal.c \
	objsystem_mammal.c \
	pane.c \
	palette.c \
	radio.c \
	rendertosurface.c \
	scrollbar.c \
	scrollview.c \
	sockets.c \
	table.c \
	textbox.c \
	textdlg.c \
	threads.c \
	timeouts.c \
	unitconv.c \
	user.c \
	widgets.c \
	windows.c

MAN1=	agartest.1

DATAFILES= agar.png agar-index.png agar.bmp agar-1.bmp agar-2.bmp agar-3.bmp \
	   agar-4.bmp axe.bmp axe.png helmet.png helmet.bmp helmet-socket.bmp \
	   loss.txt menubg.bmp pepe.jpg sword.bmp sword-socket.bmp sq-agar.bmp \
	   sq-agar.png

CLEANFILES+=	agar-index-save.png agar-save.png axe-save.png pepe-save.jpg

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk
