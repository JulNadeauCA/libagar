# $Csoft: csoft.man.mk,v 1.36 2004/03/22 01:13:17 vedge Exp $

# Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
# <http://www.csoft.org>
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

NROFF?=	nroff
MAN1?=""
MAN2?=""
MAN3?=""
MAN4?=""
MAN5?=""
MAN6?=""
MAN7?=""
MAN8?=""
MAN9?=""
MANS=	${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8} ${MAN9}
NOMAN?=

all: all-subdir preformat-man
install: install-man-dirs install-man install-subdir
deinstall: deinstall-subdir
clean: clean-man clean-subdir
cleandir: clean-man clean-subdir cleandir-subdir
regress: regress-subdir
depend: depend-subdir

.SUFFIXES: .1 .2 .3 .4 .5 .6 .7 .8 .9 .cat1 .cat2 .cat3 .cat4 .cat5 .cat6 .cat7 .cat8 .cat9 .ps1 .ps2 .ps3 .ps4 .ps5 .ps6 .ps7 .ps8 .ps9

.1.cat1 .2.cat2 .3.cat3 .4.cat4 .5.cat5 .6.cat6 .7.cat7 .8.cat8 .9.cat9:
	@echo "${NROFF} -Tascii -mandoc $< > $@"
	@(cat $< | \
	  sed 's,\$$SYSCONFDIR,${SYSCONFDIR},' | \
	  sed 's,\$$PREFIX,${PREFIX},' | \
	  sed 's,\$$SHAREDIR,${SHAREDIR},' | \
	  ${NROFF} -Tascii -mandoc > $@) || (rm -f $@; true)

.1.ps1 .2.ps2 .3.ps3 .4.ps4 .5.ps5 .6.ps6 .7.ps7 .8.ps8 .9.ps9:
	@echo "${NROFF} -Tps -mandoc $< > $@"
	@(cat $< | \
	  sed 's,\$$SYSCONFDIR,${SYSCONFDIR},' | \
	  sed 's,\$$PREFIX,${PREFIX},' | \
	  sed 's,\$$SHAREDIR,${SHAREDIR},' | \
	  ${NROFF} -Tps -mandoc > $@) || (rm -f $@; true)

preformat-man:
	@if [ "${MAN1}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN1}" = "" ]; then \
	        for F in ${MAN1}; do \
	            CAT=`echo $$F | sed 's/.1$$/.cat1/'`; \
	            PS=`echo $$F | sed 's/.1$$/.ps1/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN1} ${PSMAN1}; \
	    fi; \
	fi
	@if [ "${MAN2}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN2}" = "" ]; then \
	        for F in ${MAN2}; do \
	            CAT=`echo $$F | sed 's/.2$$/.cat2/'`; \
	            PS=`echo $$F | sed 's/.2$$/.ps2/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN2} ${PSMAN2}; \
	    fi; \
	fi
	@if [ "${MAN3}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN3}" = "" ]; then \
	        for F in ${MAN3}; do \
	            CAT=`echo $$F | sed 's/.3$$/.cat3/'`; \
	            PS=`echo $$F | sed 's/.3$$/.ps3/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN3} ${PSMAN3}; \
	    fi; \
	fi
	@if [ "${MAN4}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN4}" = "" ]; then \
	        for F in ${MAN4}; do \
	            CAT=`echo $$F | sed 's/.4$$/.cat4/'`; \
	            PS=`echo $$F | sed 's/.4$$/.ps4/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN4} ${PSMAN4}; \
	    fi; \
	fi
	@if [ "${MAN5}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN5}" = "" ]; then \
	        for F in ${MAN5}; do \
	            CAT=`echo $$F | sed 's/.5$$/.cat5/'`; \
	            PS=`echo $$F | sed 's/.5$$/.ps5/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN5} ${PSMAN5}; \
	    fi; \
	fi
	@if [ "${MAN6}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN6}" = "" ]; then \
	        for F in ${MAN6}; do \
	            CAT=`echo $$F | sed 's/.6$$/.cat6/'`; \
	            PS=`echo $$F | sed 's/.6$$/.ps6/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN6} ${PSMAN6}; \
	    fi; \
	fi
	@if [ "${MAN7}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN7}" = "" ]; then \
	        for F in ${MAN7}; do \
	            CAT=`echo $$F | sed 's/.7$$/.cat7/'`; \
	            PS=`echo $$F | sed 's/.7$$/.ps7/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN7} ${PSMAN7}; \
	    fi; \
	fi
	@if [ "${MAN8}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN8}" = "" ]; then \
	        for F in ${MAN8}; do \
	            CAT=`echo $$F | sed 's/.8$$/.cat8/'`; \
	            PS=`echo $$F | sed 's/.8$$/.ps8/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN8} ${PSMAN8}; \
	    fi; \
	fi
	@if [ "${MAN9}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN9}" = "" ]; then \
	        for F in ${MAN9}; do \
	            CAT=`echo $$F | sed 's/.9$$/.cat9/'`; \
	            PS=`echo $$F | sed 's/.9$$/.ps9/'`; \
	            ${MAKE} $$CAT $$PS; \
	        done; \
	    else \
	        ${MAKE} ${CATMAN9} ${PSMAN9}; \
	    fi; \
	fi

clean-man:
	@if [ "${MAN1}" != "" ]; then \
	    if [ "${CATMAN1}" = "" ]; then \
	        for F in ${MAN1}; do \
	            CAT=`echo $$F | sed 's/.1$$/.cat1/'`; \
	            PS=`echo $$F | sed 's/.1$$/.ps1/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN1} ${PSMAN1}; \
	     fi; \
	fi
	@if [ "${MAN2}" != "" ]; then \
	    if [ "${CATMAN2}" = "" ]; then \
	        for F in ${MAN2}; do \
	            CAT=`echo $$F | sed 's/.2$$/.cat2/'`; \
	            PS=`echo $$F | sed 's/.2$$/.ps2/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN2} ${PSMAN2}; \
	     fi; \
	fi
	@if [ "${MAN3}" != "" ]; then \
	    if [ "${CATMAN3}" = "" ]; then \
	        for F in ${MAN3}; do \
	            CAT=`echo $$F | sed 's/.3$$/.cat3/'`; \
	            PS=`echo $$F | sed 's/.3$$/.ps3/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN3} ${PSMAN3}; \
	     fi; \
	fi
	@if [ "${MAN4}" != "" ]; then \
	    if [ "${CATMAN4}" = "" ]; then \
	        for F in ${MAN4}; do \
	            CAT=`echo $$F | sed 's/.4$$/.cat4/'`; \
	            PS=`echo $$F | sed 's/.4$$/.ps4/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN4} ${PSMAN4}; \
	     fi; \
	fi
	@if [ "${MAN5}" != "" ]; then \
	    if [ "${CATMAN5}" = "" ]; then \
	        for F in ${MAN5}; do \
	            CAT=`echo $$F | sed 's/.5$$/.cat5/'`; \
	            PS=`echo $$F | sed 's/.5$$/.ps5/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN5} ${PSMAN5}; \
	     fi; \
	fi
	@if [ "${MAN6}" != "" ]; then \
	    if [ "${CATMAN6}" = "" ]; then \
	        for F in ${MAN6}; do \
	            CAT=`echo $$F | sed 's/.6$$/.cat6/'`; \
	            PS=`echo $$F | sed 's/.6$$/.ps6/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN6} ${PSMAN6}; \
	     fi; \
	fi
	@if [ "${MAN7}" != "" ]; then \
	    if [ "${CATMAN7}" = "" ]; then \
	        for F in ${MAN7}; do \
	            CAT=`echo $$F | sed 's/.7$$/.cat7/'`; \
	            PS=`echo $$F | sed 's/.7$$/.ps7/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN7} ${PSMAN7}; \
	     fi; \
	fi
	@if [ "${MAN8}" != "" ]; then \
	    if [ "${CATMAN8}" = "" ]; then \
	        for F in ${MAN8}; do \
	            CAT=`echo $$F | sed 's/.8$$/.cat8/'`; \
	            PS=`echo $$F | sed 's/.8$$/.ps8/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN8} ${PSMAN8}; \
	     fi; \
	fi
	@if [ "${MAN9}" != "" ]; then \
	    if [ "${CATMAN9}" = "" ]; then \
	        for F in ${MAN9}; do \
	            CAT=`echo $$F | sed 's/.9$$/.cat9/'`; \
	            PS=`echo $$F | sed 's/.9$$/.ps9/'`; \
	            rm -f $$CAT $$PS; \
	        done; \
	     else \
	         rm -f ${CATMAN9} ${PSMAN9}; \
	     fi; \
	fi

install-man-dirs:
	@if [ "${MANS}" != "       " ]; then \
	    if [ ! -d "${MANDIR}" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}; \
	    fi; \
	    if [ ! -d "${MANDIR}/man1" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man1"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man1; \
	    fi; \
	    if [ ! -d "${MANDIR}/man2" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man2"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man2; \
	    fi; \
	    if [ ! -d "${MANDIR}/man3" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man3"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man3; \
	    fi; \
	    if [ ! -d "${MANDIR}/man4" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man4"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man4; \
	    fi; \
	    if [ ! -d "${MANDIR}/man5" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man5"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man5; \
	    fi; \
	    if [ ! -d "${MANDIR}/man6" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man6"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man6; \
	    fi; \
	    if [ ! -d "${MANDIR}/man7" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man7"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man7; \
	    fi; \
	    if [ ! -d "${MANDIR}/man8" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man8"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man8; \
	    fi; \
	    if [ ! -d "${MANDIR}/man9" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/man9"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/man9; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat1" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat1"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat1; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat2" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat2"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat2; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat3" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat3"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat3; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat4" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat4"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat4; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat5" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat5"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat5; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat6" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat6"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat6; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat7" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat7"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat7; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat8" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat8"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat8; \
	    fi; \
	    if [ ! -d "${MANDIR}/cat9" ]; then \
	        echo "${INSTALL_MAN_DIR} ${MANDIR}/cat9"; \
	        ${SUDO} ${INSTALL_MAN_DIR} ${MANDIR}/cat9; \
	    fi; \
	    if [ ! -d "${PSDIR}" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps1" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps1"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps1; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps2" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps2"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps2; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps3" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps3"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps3; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps4" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps4"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps4; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps5" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps5"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps5; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps6" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps6"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps6; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps7" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps7"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps7; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps8" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps8"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps8; \
	    fi; \
	    if [ ! -d "${PSDIR}/ps9" ]; then \
	        echo "${INSTALL_PS_DIR} ${PSDIR}/ps9"; \
	        ${SUDO} ${INSTALL_PS_DIR} ${PSDIR}/ps9; \
	    fi; \
	fi

install-man:
	@if [ "${MAN1}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN1}" = "" ]; then \
	        for F in ${MAN1}; do \
	            CAT=`echo $$F | sed 's/.1$$/.cat1/'`; \
	            PS=`echo $$F | sed 's/.1$$/.ps1/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man1"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man1; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat1"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat1; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps1"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps1; \
	        done; \
	    else \
	        for F in ${MAN1}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man1"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man1; \
		done; \
	        for F in ${CATMAN1}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat1"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat1; \
		done; \
	        for F in ${PSMAN1}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps1"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps1; \
		done; \
	    fi; \
	fi
	@if [ "${MAN2}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN2}" = "" ]; then \
	        for F in ${MAN2}; do \
	            CAT=`echo $$F | sed 's/.2$$/.cat2/'`; \
	            PS=`echo $$F | sed 's/.2$$/.ps2/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man2"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man2; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat2"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat2; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps2"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps2; \
	        done; \
	    else \
	        for F in ${MAN2}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man2"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man2; \
		done; \
	        for F in ${CATMAN2}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat2"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat2; \
		done; \
	        for F in ${PSMAN2}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps2"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps2; \
		done; \
	    fi; \
	fi
	@if [ "${MAN3}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN3}" = "" ]; then \
	        for F in ${MAN3}; do \
	            CAT=`echo $$F | sed 's/.3$$/.cat3/'`; \
	            PS=`echo $$F | sed 's/.3$$/.ps3/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man3"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man3; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat3"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat3; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps3"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps3; \
	        done; \
	    else \
	        for F in ${MAN3}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man3"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man3; \
		done; \
	        for F in ${CATMAN3}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat3"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat3; \
		done; \
	        for F in ${PSMAN3}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps3"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps3; \
		done; \
	    fi; \
	fi
	@if [ "${MAN4}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN4}" = "" ]; then \
	        for F in ${MAN4}; do \
	            CAT=`echo $$F | sed 's/.4$$/.cat4/'`; \
	            PS=`echo $$F | sed 's/.4$$/.ps4/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man4"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man4; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat4"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat4; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps4"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps4; \
	        done; \
	    else \
	        for F in ${MAN4}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man4"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man4; \
		done; \
	        for F in ${CATMAN4}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat4"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat4; \
		done; \
	        for F in ${PSMAN4}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps4"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps4; \
		done; \
	    fi; \
	fi
	@if [ "${MAN5}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN5}" = "" ]; then \
	        for F in ${MAN5}; do \
	            CAT=`echo $$F | sed 's/.5$$/.cat5/'`; \
	            PS=`echo $$F | sed 's/.5$$/.ps5/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man5"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man5; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat5"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat5; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps5"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps5; \
	        done; \
	    else \
	        for F in ${MAN5}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man5"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man5; \
		done; \
	        for F in ${CATMAN5}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat5"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat5; \
		done; \
	        for F in ${PSMAN5}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps5"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps5; \
		done; \
	    fi; \
	fi
	@if [ "${MAN6}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN6}" = "" ]; then \
	        for F in ${MAN6}; do \
	            CAT=`echo $$F | sed 's/.6$$/.cat6/'`; \
	            PS=`echo $$F | sed 's/.6$$/.ps6/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man6"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man6; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat6"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat6; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps6"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps6; \
	        done; \
	    else \
	        for F in ${MAN6}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man6"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man6; \
		done; \
	        for F in ${CATMAN6}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat6"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat6; \
		done; \
	        for F in ${PSMAN6}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps6"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps6; \
		done; \
	    fi; \
	fi
	@if [ "${MAN7}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN7}" = "" ]; then \
	        for F in ${MAN7}; do \
	            CAT=`echo $$F | sed 's/.7$$/.cat7/'`; \
	            PS=`echo $$F | sed 's/.7$$/.ps7/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man7"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man7; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat7"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat7; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps7"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps7; \
	        done; \
	    else \
	        for F in ${MAN7}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man7"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man7; \
		done; \
	        for F in ${CATMAN7}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat7"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat7; \
		done; \
	        for F in ${PSMAN7}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps7"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps7; \
		done; \
	    fi; \
	fi
	@if [ "${MAN8}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN8}" = "" ]; then \
	        for F in ${MAN8}; do \
	            CAT=`echo $$F | sed 's/.8$$/.cat8/'`; \
	            PS=`echo $$F | sed 's/.8$$/.ps8/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man8"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man8; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat8"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat8; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps8"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps8; \
	        done; \
	    else \
	        for F in ${MAN8}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man8"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man8; \
		done; \
	        for F in ${CATMAN8}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat8"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat8; \
		done; \
	        for F in ${PSMAN8}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps8"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps8; \
		done; \
	    fi; \
	fi
	@if [ "${MAN9}" != "" -a "${NOMAN}" != "yes" ]; then \
	    if [ "${CATMAN9}" = "" ]; then \
	        for F in ${MAN9}; do \
	            CAT=`echo $$F | sed 's/.9$$/.cat9/'`; \
	            PS=`echo $$F | sed 's/.9$$/.ps9/'`; \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man9"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man9; \
	            echo "${INSTALL_DATA} $$CAT ${MANDIR}/cat9"; \
	            ${SUDO} ${INSTALL_DATA} $$CAT ${MANDIR}/cat9; \
	            echo "${INSTALL_DATA} $$PS ${PSDIR}/ps9"; \
	            ${SUDO} ${INSTALL_DATA} $$PS ${PSDIR}/ps9; \
	        done; \
	    else \
	        for F in ${MAN9}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/man9"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/man9; \
		done; \
	        for F in ${CATMAN9}; do \
	            echo "${INSTALL_DATA} $$F ${MANDIR}/cat9"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${MANDIR}/cat9; \
		done; \
	        for F in ${PSMAN9}; do \
	            echo "${INSTALL_DATA} $$F ${PSDIR}/ps9"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${PSDIR}/ps9; \
		done; \
	    fi; \
	fi

man:
	@if [ "${MAN}" != "" ]; then \
		echo "${NROFF} -Tascii -mandoc ${MAN} | ${PAGER}"; \
		${NROFF} -Tascii -mandoc ${MAN} | ${PAGER}; \
	else \
		echo "Usage: ${MAKE} read MAN=(manpage)"; \
		exit 1; \
	fi

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-man clean-man
.PHONY: man performat-man install-man-dirs

include ${TOP}/mk/csoft.common.mk
