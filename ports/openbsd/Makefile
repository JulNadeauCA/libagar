# $OpenBSD$

COMMENT=	"GUI toolkit for SDL/OpenGL"

VERSION=	10162005
DISTNAME=	agar-${VERSION}
CATEGORIES=	devel

HOMEPAGE=	http://agar.csoft.org/
MASTER_SITES=	http://beta.csoft.org/agar/

MAINTAINER=	Wilbern Cobb <wcobb@openbsd.org>

# BSD
PERMIT_PACKAGE_CDROM=   Yes
PERMIT_PACKAGE_FTP=     Yes
PERMIT_DISTFILES_CDROM= Yes
PERMIT_DISTFILES_FTP=   Yes
WANTLIB=		X11 Xext c m usbhid

LIB_DEPENDS=		SDL::devel/sdl
USE_X11=		Yes

FLAVORS=	threads net gl debug
FLAVOR?=

CONFIGURE_STYLE= simple
CONFIGURE_ARGS+= ${CONFIGURE_SHARED} \
		 --without-smpeg \
		 --with-freetype

.if ${FLAVOR:L:Mthreads}
CONFIGURE_ARGS+= --enable-threads
WANTLIB+= pthread
.else
CONFIGURE_ARGS+= --disable-threads
.endif

.if ${FLAVOR:L:Mnet}
CONFIGURE_ARGS+= --enable-network
.else
CONFIGURE_ARGS+= --disable-network
.endif

.if ${FLAVOR:L:Mgl}
CONFIGURE_ARGS+= --with-gl
.else
CONFIGURE_ARGS+= --without-gl
.endif

.if ${FLAVOR:L:Mdebug}
CONFIGURE_ARGS+= --enable-debug --enable-warnings
.endif

NO_REGRESS=	Yes

.include <bsd.port.mk>
