TOP=	.
include ${TOP}/Makefile.config

PROJECT=	"Agar"
PROJECT_GUID=	"93733df2-c743-489e-bc9f-f22aee00d787"
PROJECT_FLAVORS=gl threads freetype

SUBDIR=	 agar-config \
	 core \
	 gui \
	 vg agar-vg-config \
	 rg agar-rg-config \
	 map agar-map-config \
	 sc agar-sc-config \
	 net agar-net-config \
	 dev agar-dev-config \
	 libintl \
	 po

all: all-subdir
clean: clean-subdir
cleandir: cleandir-config cleandir-subdir
install: install-subdir install-includes
deinstall: deinstall-subdir deinstall-includes
depend: depend-subdir
regress: regress-subdir

configure:
	cat configure.in | mkconfigure > configure
	chmod 755 configure

configure.windows:
	cat configure.in | mkconfigure --emul-os=windows > configure.windows
	chmod 755 configure.windows

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
	    -exec ${SH} mk/install-includes.sh "{}" "${INCLDIR}/agar/{}" \;
	@if [ "${SRC}" != "" ]; then \
		(cd ${SRC} && ${SUDO} env \
		    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
		    INSTALL_INCL="${INSTALL_INCL}" \
		    ${FIND} . -type d \! -name .svn \
		    -exec ${SH} mk/install-includes.sh "{}" \
		    "${INCLDIR}/agar/{}" \;); \
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

.PHONY: clean cleandir install deinstall depend regress
.PHONY: configure cleandir-config package snapshot release fastclean
.PHONY: install-includes deinstall-includes

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
include ${TOP}/mk/build.man.mk
include ${TOP}/mk/build.proj.mk
