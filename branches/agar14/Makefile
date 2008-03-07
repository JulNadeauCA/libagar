TOP=	.
include ${TOP}/Makefile.config

PROJECT=	"Agar"
PROJECT_GUID=	"93733df2-c743-489e-bc9f-f22aee00d787"
PROJPREPKG=	pre-package
PROJPOSTPKG=	post-package

include ${TOP}/Makefile.proj

SUBDIR=	core agar-core-config \
	gui agar-config \
	vg agar-vg-config \
	rg agar-rg-config \
	map agar-map-config \
	sc agar-sc-config \
	dev agar-dev-config \
	net agar-net-config \
	po

all: all-subdir
clean: clean-config clean-subdir
cleandir: cleandir-config cleandir-subdir
install: install-subdir install-includes
deinstall: deinstall-subdir deinstall-includes
depend: depend-subdir
regress: regress-subdir

configure:
	cat configure.in | mkconfigure > configure
	chmod 755 configure

clean-config:
	rm -f configure.lua

cleandir-config:
	rm -fr config config.log Makefile.config .projfiles.out .projfiles2.out
	touch Makefile.config
	(cd agarpaint && ${MAKE} cleandir)
	(cd agarpaint && rm -f Makefile.config)
	(cd agarpaint && touch Makefile.config)
	(cd demos && ${MAKE} cleandir)
	(cd demos && rm -f */Makefile.config)
	find . -name premake.lua -exec rm -f {} \;
	find . -name configure.lua -exec rm -f {} \;

snapshot: cleandir
	sh mk/dist.sh snapshot

package: cleandir
	sh mk/dist.sh

release: cleandir
	sh mk/dist.sh commit

fastclean:
	find . -type f -and \( -name \*.o -or -name \*.lo -or \
	          -name \*.la -or -name \*.a -or \
	          -name libtool -or -name \*.out \
	          -name .depend -or -name config.log -or \
	          -name stdout.txt -or -name stderr.txt \) \
		  -exec rm -f {} \;
	find . -type d -and -name .libs -exec rm -fR {} \;

install-includes:
	${SUDO} ${INSTALL_INCL_DIR} ${INCLDIR}
	${SUDO} ${INSTALL_INCL_DIR} ${INCLDIR}/agar
	${SUDO} env \
	    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
	    INSTALL_INCL="${INSTALL_INCL}" \
	    ${FIND} . -type d \! -name .svn \
	    -exec ${SH} mk/install-includes.sh {} ${INCLDIR}/agar \;
	@if [ "${SRC}" != "" ]; then \
		(cd ${SRC} && ${SUDO} env \
		    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
		    INSTALL_INCL="${INSTALL_INCL}" \
		    ${FIND} . -type d \! -name .svn \
		    -exec ${SH} mk/install-includes.sh {} ${INCLDIR}/agar \;); \
		echo "${INSTALL_INCL} core_pub.h ${INCLDIR}/agar/core.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/core/core_pub.h \
		    ${INCLDIR}/agar/core.h; \
		echo "${INSTALL_INCL} gui_pub.h ${INCLDIR}/agar/gui.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/gui/gui_pub.h \
		    ${INCLDIR}/agar/gui.h; \
		echo "${INSTALL_INCL} vg/vg_pub.h ${INCLDIR}/agar/vg.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/vg/vg_pub.h \
		    ${INCLDIR}/agar/vg.h; \
		echo "${INSTALL_INCL} rg/rg_pub.h ${INCLDIR}/agar/rg.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/rg/rg_pub.h \
		   ${INCLDIR}/agar/rg.h; \
		echo "${INSTALL_INCL} net/net_pub.h ${INCLDIR}/agar/net.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/net/net_pub.h \
		   ${INCLDIR}/agar/net.h; \
		echo "${INSTALL_INCL} map/map_pub.h ${INCLDIR}/agar/map.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/map/map_pub.h \
		   ${INCLDIR}/agar/map.h; \
		echo "${INSTALL_INCL} sc/sc_pub.h ${INCLDIR}/agar/sc.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/sc/sc_pub.h \
		   ${INCLDIR}/agar/sc.h; \
		echo "${INSTALL_INCL} dev/dev_pub.h ${INCLDIR}/agar/dev.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/dev/dev_pub.h \
		   ${INCLDIR}/agar/dev.h; \
	else \
		echo "${INSTALL_INCL} core/core_pub.h \
		    ${INCLDIR}/agar/core.h"; \
		${SUDO} ${INSTALL_INCL} core/core_pub.h \
		    ${INCLDIR}/agar/core.h; \
		echo "${INSTALL_INCL} gui/gui_pub.h ${INCLDIR}/agar/gui.h"; \
		${SUDO} ${INSTALL_INCL} gui/gui_pub.h ${INCLDIR}/agar/gui.h; \
		echo "${INSTALL_INCL} vg/vg_pub.h ${INCLDIR}/agar/vg.h"; \
		${SUDO} ${INSTALL_INCL} vg/vg_pub.h ${INCLDIR}/agar/vg.h; \
		echo "${INSTALL_INCL} rg/rg_pub.h ${INCLDIR}/agar/rg.h"; \
		${SUDO} ${INSTALL_INCL} rg/rg_pub.h ${INCLDIR}/agar/rg.h; \
		echo "${INSTALL_INCL} net/net_pub.h ${INCLDIR}/agar/net.h"; \
		${SUDO} ${INSTALL_INCL} net/net_pub.h ${INCLDIR}/agar/net.h; \
		echo "${INSTALL_INCL} map/map_pub.h ${INCLDIR}/agar/map.h"; \
		${SUDO} ${INSTALL_INCL} map/map_pub.h ${INCLDIR}/agar/map.h; \
		echo "${INSTALL_INCL} sc/sc_pub.h ${INCLDIR}/agar/sc.h"; \
		${SUDO} ${INSTALL_INCL} sc/sc_pub.h ${INCLDIR}/agar/sc.h; \
		echo "${INSTALL_INCL} dev/dev_pub.h ${INCLDIR}/agar/dev.h"; \
		${SUDO} ${INSTALL_INCL} dev/dev_pub.h ${INCLDIR}/agar/dev.h; \
	fi

deinstall-includes:
	${FIND} . -type f -name '*.h' -print \
	    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
	    | ${SUDO} ${SH}
	@if [ "${SRC}" != "" ]; then \
		echo "${FIND} ${SRC} -type f -name '*.h' -print \
		    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
		    | ${SUDO} ${SH}"; \
		(cd ${SRC} && ${FIND} . -type f -name '*.h' -print \
		    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
		    | ${SUDO} ${SH}); \
	fi

pre-package:
	@if [ "${PKG_OS}" = "windows" ]; then \
		cp -f ${TOP}/mk/install-sdk/install-sdk.exe .; \
		echo '<meta http-equiv="refresh" content="1;url=http://libagar.org/docs/compile-msvc.html" />' > VisualC.html; \
		echo "install-sdk.exe" >> ${PROJFILELIST}; \
		echo "VisualC.html" >> ${PROJFILELIST}; \
		if [ -e "`which unix2dos 2>/dev/null`" ]; then \
			V=`perl mk/get-version.pl`; \
			unix2dos -n README README.txt; \
			unix2dos -n INSTALL INSTALL.txt; \
			unix2dos -n ChangeLogs/Release-$$V RELEASE-$$V.txt; \
			unix2dos -n mk/LICENSE.txt LICENSE.txt; \
			unix2dos -n gui/fonts/Vera-Copyright.txt \
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
		cp ChangeLogs/Release-$$V RELEASE-$$V; \
		cp ChangeLogs/ChangeLog-$$V ChangeLog-$$V; \
		cp mk/LICENSE.txt LICENSE; \
		cp gui/fonts/Vera-Copyright.txt LICENSE-Vera; \
		cp mk/agar-logo.png Logo.png; \
	fi

post-package:
	@if [ "${PKG_OS}" = "windows" ]; then \
		rm -f install-sdk.exe README.txt INSTALL.txt VisualC.html; \
		rm -f RELEASE-*.txt LICENSE.txt License-Vera.txt Logo.png; \
	else \
		rm -f Release-* ChangeLog-* LICENSE LICENSE-Vera Logo.png; \
	fi

.PHONY: clean cleandir install deinstall depend regress
.PHONY: configure cleandir-config package snapshot release fastclean
.PHONY: install-includes deinstall-includes pre-package post-package

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
include ${TOP}/mk/build.man.mk
include ${TOP}/mk/build.proj.mk
