#
# Copyright (c) 2001-2016 Hypertriton, Inc. <http://hypertriton.com/>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# Preformat and install manual pages from mandoc sources.
#

MANDOC?=
PAGER?=more
MAN1?=
MAN2?=
MAN3?=
MAN4?=
MAN5?=
MAN6?=
MAN7?=
MAN8?=
MAN9?=
CATMAN1?=
CATMAN2?=
CATMAN3?=
CATMAN4?=
CATMAN5?=
CATMAN6?=
CATMAN7?=
CATMAN8?=
CATMAN9?=
MANS=${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8} ${MAN9}
MANLINKS?=
NOMAN?=
NOCATMAN?=
NOMANLINKS?=
CLEANFILES?=

all: all-subdir all-catman
install: all install-man-dirs install-man install-subdir
deinstall: deinstall-subdir
clean: clean-man clean-subdir
cleandir: clean-man clean-subdir cleandir-subdir
regress: regress-subdir
depend: depend-subdir

.SUFFIXES: .1 .2 .3 .4 .5 .6 .7 .8 .9 .cat1 .cat2 .cat3 .cat4 .cat5 .cat6 .cat7 .cat8 .cat9 .ps .pdf .html

.1.cat1 .2.cat2 .3.cat3 .4.cat4 .5.cat5 .6.cat6 .7.cat7 .8.cat8 .9.cat9:
	@echo "${MANDOC} -Tascii $< > $@"
	@(cat $< | \
	  sed 's,\$$SYSCONFDIR,${SYSCONFDIR},' | \
	  sed 's,\$$PREFIX,${PREFIX},' | \
	  sed 's,\$$DATADIR,${DATADIR},' | \
	  ${MANDOC} -Tascii > $@) || (rm -f $@; true)

.1.ps .2.ps .3.ps .4.ps .5.ps .6.ps .7.ps .8.ps .9.ps:
	@if [ "${HAVE_MANDOC}" != "yes" ]; then echo "Requires mandoc"; exit 1; fi
	@echo "${MANDOC} -Tps $< > $@"
	@(cat $< | \
	  sed 's,\$$SYSCONFDIR,${SYSCONFDIR},' | \
	  sed 's,\$$PREFIX,${PREFIX},' | \
	  sed 's,\$$DATADIR,${DATADIR},' | \
	  ${MANDOC} -Tps > $@) || (rm -f $@; true)

.1.pdf .2.pdf .3.pdf .4.pdf .5.pdf .6.pdf .7.pdf .8.pdf .9.pdf:
	@if [ "${HAVE_MANDOC}" != "yes" ]; then echo "Requires mandoc"; exit 1; fi
	@echo "${MANDOC} -Tpdf $< > $@"
	@(cat $< | \
	  sed 's,\$$SYSCONFDIR,${SYSCONFDIR},' | \
	  sed 's,\$$PREFIX,${PREFIX},' | \
	  sed 's,\$$DATADIR,${DATADIR},' | \
	  ${MANDOC} -Tpdf > $@) || (rm -f $@; true)

.1.html .2.html .3.html .4.html .5.html .6.html .7.html .8.html .9.html:
	@if [ "${HAVE_MANDOC}" != "yes" ]; then echo "Requires mandoc"; exit 1; fi
	@echo "${MANDOC} -Thtml $< > $@"
	@(cat $< | \
	  sed 's,\$$SYSCONFDIR,${SYSCONFDIR},' | \
	  sed 's,\$$PREFIX,${PREFIX},' | \
	  sed 's,\$$DATADIR,${DATADIR},' | \
	  ${MANDOC} -Thtml > $@) || (rm -f $@; true)

all-catman:
	@if [ "${HAVE_MANDOC}" != "yes" ]; then exit 0; fi
	@if [ "${MAN1}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN1}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN1}; do \
	            CAT=`echo $$F | sed 's/.1$$/.cat1/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN1}; \
	    fi; \
	fi
	@if [ "${MAN2}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN2}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN2}; do \
	            CAT=`echo $$F | sed 's/.2$$/.cat2/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN2}; \
	    fi; \
	fi
	@if [ "${MAN3}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN3}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN3}; do \
	            CAT=`echo $$F | sed 's/.3$$/.cat3/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN3}; \
	    fi; \
	fi
	@if [ "${MAN4}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN4}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN4}; do \
	            CAT=`echo $$F | sed 's/.4$$/.cat4/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN4}; \
	    fi; \
	fi
	@if [ "${MAN5}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN5}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN5}; do \
	            CAT=`echo $$F | sed 's/.5$$/.cat5/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN5}; \
	    fi; \
	fi
	@if [ "${MAN6}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN6}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN6}; do \
	            CAT=`echo $$F | sed 's/.6$$/.cat6/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN6}; \
	    fi; \
	fi
	@if [ "${MAN7}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN7}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN7}; do \
	            CAT=`echo $$F | sed 's/.7$$/.cat7/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN7}; \
	    fi; \
	fi
	@if [ "${MAN8}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN8}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN8}; do \
	            CAT=`echo $$F | sed 's/.8$$/.cat8/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN8}; \
	    fi; \
	fi
	@if [ "${MAN9}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN9}" = "" ]; then \
	        CATLIST=""; \
	        for F in ${MAN9}; do \
	            CAT=`echo $$F | sed 's/.9$$/.cat9/'`; \
		    CATLIST="$$CATLIST $$CAT"; \
	        done; \
	        ${MAKE} $$CATLIST; \
	    else \
	        ${MAKE} ${CATMAN9}; \
	    fi; \
	fi

clean-man:
	@if [ "${MAN1}" != "" ]; then \
	    if [ "${CATMAN1}" = "" ]; then \
	        for F in ${MAN1}; do \
	            CAT=`echo $$F | sed 's/.1$$/.cat1/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN1}; \
	     fi; \
	fi
	@if [ "${MAN2}" != "" ]; then \
	    if [ "${CATMAN2}" = "" ]; then \
	        for F in ${MAN2}; do \
	            CAT=`echo $$F | sed 's/.2$$/.cat2/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN2}; \
	     fi; \
	fi
	@if [ "${MAN3}" != "" ]; then \
	    if [ "${CATMAN3}" = "" ]; then \
	        for F in ${MAN3}; do \
	            CAT=`echo $$F | sed 's/.3$$/.cat3/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN3}; \
	     fi; \
	fi
	@if [ "${MAN4}" != "" ]; then \
	    if [ "${CATMAN4}" = "" ]; then \
	        for F in ${MAN4}; do \
	            CAT=`echo $$F | sed 's/.4$$/.cat4/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN4}; \
	     fi; \
	fi
	@if [ "${MAN5}" != "" ]; then \
	    if [ "${CATMAN5}" = "" ]; then \
	        for F in ${MAN5}; do \
	            CAT=`echo $$F | sed 's/.5$$/.cat5/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN5}; \
	     fi; \
	fi
	@if [ "${MAN6}" != "" ]; then \
	    if [ "${CATMAN6}" = "" ]; then \
	        for F in ${MAN6}; do \
	            CAT=`echo $$F | sed 's/.6$$/.cat6/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN6}; \
	     fi; \
	fi
	@if [ "${MAN7}" != "" ]; then \
	    if [ "${CATMAN7}" = "" ]; then \
	        for F in ${MAN7}; do \
	            CAT=`echo $$F | sed 's/.7$$/.cat7/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN7}; \
	     fi; \
	fi
	@if [ "${MAN8}" != "" ]; then \
	    if [ "${CATMAN8}" = "" ]; then \
	        for F in ${MAN8}; do \
	            CAT=`echo $$F | sed 's/.8$$/.cat8/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN8}; \
	     fi; \
	fi
	@if [ "${MAN9}" != "" ]; then \
	    if [ "${CATMAN9}" = "" ]; then \
	        for F in ${MAN9}; do \
	            CAT=`echo $$F | sed 's/.9$$/.cat9/'`; \
	            rm -f $$CAT; \
	        done; \
	     else \
	         rm -f ${CATMAN9}; \
	     fi; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi

install-man-dirs:
	@if [ "${MANS}" != "       " ]; then \
	    if [ ! -d "${DESTDIR}${MANDIR}" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man1" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man1"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man1; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man2" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man2"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man2; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man3" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man3"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man3; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man4" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man4"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man4; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man5" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man5"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man5; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man6" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man6"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man6; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man7" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man7"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man7; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man8" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man8"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man8; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/man9" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man9"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man9; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat1" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat1"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat1; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat2" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat2"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat2; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat3" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat3"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat3; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat4" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat4"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat4; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat5" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat5"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat5; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat6" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat6"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat6; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat7" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat7"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat7; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat8" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat8"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat8; \
	    fi; \
	    if [ ! -d "${DESTDIR}${MANDIR}/cat9" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat9"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/cat9; \
	    fi; \
	fi

install-man:
	@if [ "${MAN1}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN1}" CATMAN="${CATMAN1}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man1" \
	        CATMANDIR="${MANDIR}/cat1" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN2}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN2}" CATMAN="${CATMAN2}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man2" \
	        CATMANDIR="${MANDIR}/cat2" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN3}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN3}" CATMAN="${CATMAN3}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man3" \
	        CATMANDIR="${MANDIR}/cat3" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN4}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN4}" CATMAN="${CATMAN4}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man4" \
	        CATMANDIR="${MANDIR}/cat4" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN5}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN5}" CATMAN="${CATMAN5}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man5" \
	        CATMANDIR="${MANDIR}/cat5" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN6}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN6}" CATMAN="${CATMAN6}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man6" \
	        CATMANDIR="${MANDIR}/cat6" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN7}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN7}" CATMAN="${CATMAN7}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man7" \
	        CATMANDIR="${MANDIR}/cat7" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN8}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN8}" CATMAN="${CATMAN8}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man8" \
	        CATMANDIR="${MANDIR}/cat8" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${MAN9}" != "" -a "${NOMAN}" != "yes" ]; then \
	    ${SUDO} env MAN="${MAN9}" CATMAN="${CATMAN9}" NOCATMAN="${NOCATMAN}" \
	        DESTDIR="${DESTDIR}" \
	        INSTALL_DATA="${INSTALL_DATA}" \
		MANDIR="${MANDIR}/man9" \
	        CATMANDIR="${MANDIR}/cat9" \
	        ${SH} ${TOP}/mk/install-manpages.sh; \
	    if [ $$? != 0 ]; then \
	    	echo "install-manpages.sh failed"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${NOMANLINKS}" != "yes" -a "${MANLINKS}" != "" ]; then \
		echo "${MAKE} install-manlinks"; \
		${MAKE} install-manlinks; \
	fi

install-manlinks:
	@(cd ${DESTDIR}${MANDIR} && \
	     for L in ${MANLINKS}; do \
	        MPG=`echo $$L | sed 's/:.*//'`; \
	        MLNK=`echo $$L | sed 's/.*://'`; \
		MS=`echo $$L | sed 's/.*\.//'`; \
		echo "ln -fs $$MPG man$$MS/$$MLNK"; \
		${SUDO} ln -fs $$MPG man$$MS/$$MLNK; \
	    done)
	@if [ "${NOCATMAN}" != "yes" ]; then \
	    (cd ${DESTDIR}${MANDIR} && \
	     for L in ${CATLINKS}; do \
	        MPG=`echo $$L | sed 's/:.*//'`; \
	        MLNK=`echo $$L | sed 's/.*://'`; \
		MS=`echo $$L | sed 's/.*\.//'`; \
		echo "ln -fs $$MPG $$MS/$$MLNK"; \
		${SUDO} ln -fs $$MPG $$MS/$$MLNK; \
	    done); \
	fi

man:
	@if [ "${MAN}" != "" ]; then \
		echo "${MANDOC} -Tascii ${MAN} | ${PAGER}"; \
		${MANDOC} -Tascii ${MAN} | ${PAGER}; \
	else \
		echo "Usage: ${MAKE} man MAN=(manpage)"; \
		exit 1; \
	fi

manlinks: Makefile
	echo > .manlinks.mk
	@if [ "${MAN2}" != "" ]; then \
		for F in ${MAN2}; do \
			echo "cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk"; \
			cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk; \
		done; \
	fi
	@if [ "${MAN3}" != "" ]; then \
		for F in ${MAN3}; do \
			echo "cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk"; \
			cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk; \
		done; \
	fi
	@if [ "${MAN9}" != "" ]; then \
		for F in ${MAN9}; do \
			echo "cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk"; \
			cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk; \
		done; \
	fi

all-manlinks:
	@if [ "${SRC}" != "" ]; then \
		(cd ${SRC} && \
		 for DIR in `find . -name .manlinks.mk |sed 's/\/\.manlinks\.mk//'`; do \
			echo "(cd $$DIR && ${MAKE} manlinks)"; \
			(cd $$DIR && ${MAKE} manlinks); \
		done); \
	else \
		for DIR in `find . -name .manlinks.mk |sed 's/\/\.manlinks\.mk//'`; do \
			echo "(cd $$DIR && ${MAKE} manlinks)"; \
			(cd $$DIR && ${MAKE} manlinks); \
		done; \
	fi

man2wiki: Makefile
	@if [ "${MAN1}" != "" ]; then \
		for F in ${MAN1}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN2}" != "" ]; then \
		for F in ${MAN2}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN3}" != "" ]; then \
		for F in ${MAN3}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN4}" != "" ]; then \
		for F in ${MAN4}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN5}" != "" ]; then \
		for F in ${MAN5}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN6}" != "" ]; then \
		for F in ${MAN6}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN7}" != "" ]; then \
		for F in ${MAN7}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN8}" != "" ]; then \
		for F in ${MAN8}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi
	@if [ "${MAN9}" != "" ]; then \
		for F in ${MAN9}; do \
			echo "cat $$F |perl ${TOP}/mk/man2wiki.pl $$F"; \
			cat $$F |perl ${TOP}/mk/man2wiki.pl $$F; \
		done; \
	fi

lint:
	@if [ "${HAVE_MANDOC}" != "yes" ]; then \
		echo "Cannot find mandoc (re-run ./configure?)"; \
		exit 1; \
	fi
	@if [ "${MAN1}" != "" ]; then for F in ${MAN1}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN2}" != "" ]; then for F in ${MAN2}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN3}" != "" ]; then for F in ${MAN3}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN4}" != "" ]; then for F in ${MAN4}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN5}" != "" ]; then for F in ${MAN5}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN6}" != "" ]; then for F in ${MAN6}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN7}" != "" ]; then for F in ${MAN7}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN8}" != "" ]; then for F in ${MAN8}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN9}" != "" ]; then for F in ${MAN9}; do ${MANDOC} -Tlint $$F; done; fi

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-man install-manlinks clean-man 
.PHONY: man all-catman install-man-dirs manlinks all-manlinks man2wiki

include ${TOP}/mk/build.common.mk
