TOP=	.
include ${TOP}/Makefile.config

PROJECT=	"Agar"
PROJECT_GUID=	"93733df2-c743-489e-bc9f-f22aee00d787"
PROJCONFIGDIR=	include/agar/config
PROJINCLUDES=	${SRCDIR}/configure.lua

include ${TOP}/Makefile.proj

SUBDIR=		core agar-core-config \
		gui agar-config \
		vg agar-vg-config \
		rg agar-rg-config \
		math agar-math-config \
		dev agar-dev-config

SUBDIR_CONFIG=	agar-core-config \
		agar-config \
		agar-vg-config \
		agar-rg-config \
		agar-math-config \
		agar-dev-config

INCDIRS=core \
	gui \
	vg \
	rg \
	math \
	dev

INCPUB=	core gui vg rg math dev

all: all-subdir
clean: clean-config clean-subdir
cleandir: cleandir-config cleandir-subdir
install: install-subdir install-includes
deinstall: deinstall-subdir deinstall-includes
depend: depend-subdir
regress: regress-subdir

includes:
	if [ ! -e "include" ]; then mkdir include; fi
	if [ "${SRCDIR}" != "${BLDDIR}" ]; then \
		(cd "${SRCDIR}" && \
		 perl mk/gen-includes.pl "${BLDDIR}/include/agar"); \
	else \
		perl mk/gen-includes.pl include/agar; \
	fi

configure:
	cat configure.in | mkconfigure > configure
	chmod 755 configure

clean-config:
	rm -f configure.lua
	rm -fR include

cleandir-config: clean-config
	rm -fr config config.log Makefile.config .projfiles.out .projfiles2.out
	touch Makefile.config
	-(cd agarpaint && ${MAKE} cleandir)
	-(cd agarrcsd && ${MAKE} cleandir)
	-(cd demos && ${MAKE} cleandir)
	-rm -fR demos/.cache
	find . -name premake.lua -exec rm -f {} \;
	find . -name configure.lua -exec rm -f {} \;

beta:
	sh mk/dist.sh beta

package:
	-${MAKE} all-manlinks
	env NOUPLOAD=Yes sh mk/dist.sh beta

release:
	-${MAKE} all-manlinks
	sh mk/dist.sh stable

install-includes:
	${SUDO} ${INSTALL_INCL_DIR} ${INCLDIR}
	${SUDO} ${INSTALL_INCL_DIR} ${INCLDIR}/agar
	@(cd include/agar && for DIR in ${INCDIRS}; do \
	    echo "mk/install-includes.sh $$DIR ${INCLDIR}/agar"; \
	    ${SUDO} env \
	      INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
	      INSTALL_INCL="${INSTALL_INCL}" \
	      ${SH} ${SRCDIR}/mk/install-includes.sh \
	        $$DIR ${INCLDIR}/agar; \
	done)
	@echo "mk/install-includes.sh config ${INCLDIR}/agar"
	@${SUDO} env \
	    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
	    INSTALL_INCL="${INSTALL_INCL}" \
	    ${SH} ${SRCDIR}/mk/install-includes.sh config ${INCLDIR}/agar
	@for INC in ${INCPUB}; do \
		echo "${INSTALL_INCL} include/agar/$$INC/$${INC}_pub.h \
		    ${INCLDIR}/agar/$${INC}.h"; \
		${SUDO} ${INSTALL_INCL} include/agar/$$INC/$${INC}_pub.h \
		    ${INCLDIR}/agar/$${INC}.h; \
	done

deinstall-includes:
	@echo "rm -fR ${INCLDIR}/agar"
	@${SUDO} rm -fR ${INCLDIR}/agar

pre-package:
	@if [ "${PKG_OS}" = "windows" ]; then \
		cp -f ${TOP}/mk/install-sdk/install-sdk.exe .; \
		echo '<meta http-equiv="refresh" content="1;url=http://libagar.org/docs/compile-msvc.html" />' > VisualC.html; \
		echo "install-sdk.exe" >> ${PROJFILELIST}; \
		echo "VisualC.html" >> ${PROJFILELIST}; \
		if [ -e "`which unix2dos 2>/dev/null`" ]; then \
			V=`perl mk/get-version.pl`; \
			cat README |unix2dos >README.txt; \
			cat INSTALL |unix2dos >INSTALL.txt; \
			cat ChangeLogs/Release-$$V.txt | \
			    unix2dos >RELEASE-$$V.txt; \
			cat mk/LICENSE.txt |unix2dos >LICENSE.txt; \
			cat gui/fonts/Vera-Copyright.txt |unix2dos > \
			    LICENSE-Vera.txt; \
			cp -f mk/agar-logo.png Logo.png; \
			echo "README.txt" >> ${PROJFILELIST}; \
			echo "INSTALL.txt" >> ${PROJFILELIST}; \
			echo "RELEASE-$$V.txt" >> ${PROJFILELIST}; \
			echo "LICENSE.txt" >> ${PROJFILELIST}; \
			echo "LICENSE-Vera.txt" >> ${PROJFILELIST}; \
			echo "Logo.png" >> ${PROJFILELIST}; \
		fi; \
	else \
		V=`perl mk/get-version.pl`; \
		cp ChangeLogs/Release-$$V.txt RELEASE-$$V; \
		cp mk/LICENSE.txt LICENSE; \
		cp gui/fonts/Vera-Copyright.txt LICENSE-Vera; \
		cp mk/agar-logo.png Logo.png; \
	fi

post-package:
	@if [ "${PKG_OS}" = "windows" ]; then \
		rm -f install-sdk.exe README.txt INSTALL.txt VisualC.html; \
		rm -f RELEASE-*.txt LICENSE.txt LICENSE-Vera.txt Logo.png; \
	else \
		rm -f Release-* ChangeLog-* LICENSE LICENSE-Vera Logo.png; \
	fi

.PHONY: clean cleandir install deinstall depend regress includes
.PHONY: configure cleandir-config package beta release
.PHONY: install-includes deinstall-includes pre-package post-package

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
include ${TOP}/mk/build.man.mk
include ${TOP}/mk/build.proj.mk
