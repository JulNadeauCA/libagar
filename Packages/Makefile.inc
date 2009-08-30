#
# Public domain
# ex:syn=make
#

PROJNAME=	agar
PORTNAME?=	${OS}-${ARCH}${FLAVOR}
PKGNAME?=	${PROJNAME}-${VERSION}-${PORTNAME}
#SRCDIR=	/path/to/source

VERSION=	`(cd ${SRCDIR} && perl mk/get-version.pl)`
RELEASE=	`(cd ${SRCDIR} && perl mk/get-release.pl)`
BUILDDIR=	${HOME}/tmp/w-${PKGNAME}
FAKEDIR=	${HOME}/tmp/pkg-${PKGNAME}
RELENGDIR=	${SRCDIR}/Packages/${PORTNAME}
PLATFORM?=	unix

CONFIGURE_ARGS?=
CONFIGURE_STD=	--enable-threads \
		--enable-network \
		--with-gl \
		--with-freetype \
		--without-jpeg

# NOTE: Sync with ${CONFIG_PROGS} in install-unix.sh.
CONFIG_PROGS=	agar-config \
		agar-core-config \
		agar-dev-config \
		agar-math-config \
		agar-rg-config \
		agar-vg-config

all: pre-build do-build package

pre-build:
	@if [ "${SRCDIR}" = "" ]; then \
		echo "*"; \
		echo "* Please set SRCDIR to the source directory path."; \
		echo "*"; \
		exit 1; \
	fi
	mkdir ${BUILDDIR}
	mkdir ${FAKEDIR}
	mkdir ${FAKEDIR}/${PKGNAME}

do-build:
	(cd ${BUILDDIR} && ${SRCDIR}/configure \
	    --srcdir=${SRCDIR} --prefix=${FAKEDIR}/${PKGNAME} \
	    ${CONFIGURE_ARGS} && \
	    make depend all install)

package:
	(cd ${FAKEDIR} && \
	 cp -f ${SRCDIR}/README ${PKGNAME} && \
	 cp -f ${SRCDIR}/mk/LICENSE.txt ${PKGNAME}/LICENSE && \
	 cp -f ${SRCDIR}/gui/fonts/Vera-Copyright.txt ${PKGNAME}/LICENSE-Vera && \
	 cp -f ${SRCDIR}/mk/agar-logo.png ${PKGNAME} && \
	 cp -f ${RELENGDIR}/README.* ${PKGNAME} && \
	 cat ${RELENGDIR}/../install-${PLATFORM}.sh | \
	     perl ${RELENGDIR}/../install-subst.pl "${SRCDIR}" "${PROJNAME}" \
	     "${OS}" "${ARCH}" "${FLAVOR}" > ${PKGNAME}/install.sh && \
	 chmod 755 ${PKGNAME}/install.sh && \
	 for F in ${CONFIG_PROGS}; do \
	 	cp -f ${RELENGDIR}/$$F.sh ${PKGNAME}; \
	 done; \
	 cp -f ${RELENGDIR}/../config.guess ${PKGNAME} && \
	 tar -f ${PKGNAME}.tar -c ${PKGNAME} && \
	 gzip -f ${PKGNAME}.tar && \
	 cp -f ${PKGNAME}.tar.gz ${RELENGDIR})

sign:
	 gpg -ab ${PKGNAME}.tar.gz

clean:
	rm -fR ${BUILDDIR}
	rm -fR ${FAKEDIR}

.PHONY: pre-build package sign do-build clean
