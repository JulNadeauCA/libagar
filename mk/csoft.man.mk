# $Csoft: csoft.man.mk,v 1.23 2003/08/26 03:22:33 vedge Exp $

# Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

MANS=	${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8} ${MAN9}
CATMANS=${CATMAN1} ${CATMAN2} ${CATMAN3} ${CATMAN4} ${CATMAN5} ${CATMAN6} ${CATMAN7} ${CATMAN8} ${CATMAN9}
PSMANS=	${PSMAN1} ${PSMAN2} ${PSMAN3} ${PSMAN4} ${PSMAN5} ${PSMAN6} ${PSMAN7} ${PSMAN8} ${PSMAN9}

.SUFFIXES: .1 .2 .3 .4 .5 .6 .7 .8 .9 .cat1 .cat2 .cat3 .cat4 .cat5 .cat6 .cat7 .cat8 .cat9 .ps1 .ps2 .ps3 .ps4 .ps5 .ps6 .ps7 .ps8 .ps9

.1.cat1 .2.cat2 .3.cat3 .4.cat4 .5.cat5 .6.cat6 .7.cat7 .8.cat8 .9.cat9:
	@echo "${NROFF} -Tascii -mandoc $< > $@"
	@${NROFF} -Tascii -mandoc $< > $@ || (rm -f $@; false)

.1.ps1 .2.ps2 .3.ps3 .4.ps4 .5.ps5 .6.ps6 .7.ps7 .8.ps8 .9.ps9:
	@echo "${NROFF} -Tps -mandoc $< > $@"
	@${NROFF} -Tps -mandoc $< > $@ || (rm -f $@; false)

preformat-man:
	@if [ "${MAN1}" != "" ]; then \
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
	@if [ "${MAN2}" != "" ]; then \
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
	@if [ "${MAN3}" != "" ]; then \
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
	@if [ "${MAN4}" != "" ]; then \
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
	@if [ "${MAN5}" != "" ]; then \
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
	@if [ "${MAN6}" != "" ]; then \
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
	@if [ "${MAN7}" != "" ]; then \
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
	@if [ "${MAN8}" != "" ]; then \
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
	@if [ "${MAN9}" != "" ]; then \
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

all: preformat-man

clean: clean-man

install: install-man

deinstall: deinstall-man

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
	    if [ ! -d "${INST_MANDIR}" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man1" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man1"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man1; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man2" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man2"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man2; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man3" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man3"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man3; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man4" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man4"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man4; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man5" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man5"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man5; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man6" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man6"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man6; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man7" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man7"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man7; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man8" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man8"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man8; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/man9" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/man9"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/man9; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat1" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat1"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat1; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat2" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat2"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat2; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat3" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat3"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat3; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat4" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat4"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat4; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat5" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat5"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat5; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat6" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat6"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat6; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat7" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat7"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat7; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat8" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat8"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat8; \
	    fi; \
	    if [ ! -d "${INST_MANDIR}/cat9" ]; then \
	        echo "${INSTALL_MAN_DIR} ${INST_MANDIR}/cat9"; \
	        ${INSTALL_MAN_DIR} ${INST_MANDIR}/cat9; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps1" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps1"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps1; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps2" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps2"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps2; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps3" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps3"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps3; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps4" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps4"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps4; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps5" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps5"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps5; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps6" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps6"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps6; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps7" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps7"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps7; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps8" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps8"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps8; \
	    fi; \
	    if [ ! -d "${INST_PSDIR}/ps9" ]; then \
	        echo "${INSTALL_PS_DIR} ${INST_PSDIR}/ps9"; \
	        ${INSTALL_PS_DIR} ${INST_PSDIR}/ps9; \
	    fi; \
	fi

install-man: preformat-man install-man-dirs
	@if [ "${MAN1}" != "" ]; then \
	    if [ "${CATMAN1}" = "" ]; then \
	        for F in ${MAN1}; do \
	            CAT=`echo $$F | sed 's/.1$$/.cat1/'`; \
	            PS=`echo $$F | sed 's/.1$$/.ps1/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man1"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man1; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat1"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat1; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps1"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps1; \
	        done; \
	    else \
	        for F in ${MAN1}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man1"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man1; \
		done; \
	        for F in ${CATMAN1}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat1"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat1; \
		done; \
	        for F in ${PSMAN1}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps1"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps1; \
		done; \
	    fi; \
	fi
	@if [ "${MAN2}" != "" ]; then \
	    if [ "${CATMAN2}" = "" ]; then \
	        for F in ${MAN2}; do \
	            CAT=`echo $$F | sed 's/.2$$/.cat2/'`; \
	            PS=`echo $$F | sed 's/.2$$/.ps2/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man2"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man2; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat2"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat2; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps2"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps2; \
	        done; \
	    else \
	        for F in ${MAN2}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man2"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man2; \
		done; \
	        for F in ${CATMAN2}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat2"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat2; \
		done; \
	        for F in ${PSMAN2}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps2"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps2; \
		done; \
	    fi; \
	fi
	@if [ "${MAN3}" != "" ]; then \
	    if [ "${CATMAN3}" = "" ]; then \
	        for F in ${MAN3}; do \
	            CAT=`echo $$F | sed 's/.3$$/.cat3/'`; \
	            PS=`echo $$F | sed 's/.3$$/.ps3/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man3"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man3; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat3"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat3; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps3"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps3; \
	        done; \
	    else \
	        for F in ${MAN3}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man3"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man3; \
		done; \
	        for F in ${CATMAN3}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat3"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat3; \
		done; \
	        for F in ${PSMAN3}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps3"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps3; \
		done; \
	    fi; \
	fi
	@if [ "${MAN4}" != "" ]; then \
	    if [ "${CATMAN4}" = "" ]; then \
	        for F in ${MAN4}; do \
	            CAT=`echo $$F | sed 's/.4$$/.cat4/'`; \
	            PS=`echo $$F | sed 's/.4$$/.ps4/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man4"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man4; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat4"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat4; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps4"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps4; \
	        done; \
	    else \
	        for F in ${MAN4}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man4"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man4; \
		done; \
	        for F in ${CATMAN4}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat4"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat4; \
		done; \
	        for F in ${PSMAN4}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps4"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps4; \
		done; \
	    fi; \
	fi
	@if [ "${MAN5}" != "" ]; then \
	    if [ "${CATMAN5}" = "" ]; then \
	        for F in ${MAN5}; do \
	            CAT=`echo $$F | sed 's/.5$$/.cat5/'`; \
	            PS=`echo $$F | sed 's/.5$$/.ps5/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man5"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man5; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat5"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat5; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps5"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps5; \
	        done; \
	    else \
	        for F in ${MAN5}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man5"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man5; \
		done; \
	        for F in ${CATMAN5}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat5"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat5; \
		done; \
	        for F in ${PSMAN5}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps5"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps5; \
		done; \
	    fi; \
	fi
	@if [ "${MAN6}" != "" ]; then \
	    if [ "${CATMAN6}" = "" ]; then \
	        for F in ${MAN6}; do \
	            CAT=`echo $$F | sed 's/.6$$/.cat6/'`; \
	            PS=`echo $$F | sed 's/.6$$/.ps6/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man6"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man6; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat6"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat6; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps6"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps6; \
	        done; \
	    else \
	        for F in ${MAN6}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man6"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man6; \
		done; \
	        for F in ${CATMAN6}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat6"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat6; \
		done; \
	        for F in ${PSMAN6}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps6"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps6; \
		done; \
	    fi; \
	fi
	@if [ "${MAN7}" != "" ]; then \
	    if [ "${CATMAN7}" = "" ]; then \
	        for F in ${MAN7}; do \
	            CAT=`echo $$F | sed 's/.7$$/.cat7/'`; \
	            PS=`echo $$F | sed 's/.7$$/.ps7/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man7"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man7; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat7"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat7; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps7"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps7; \
	        done; \
	    else \
	        for F in ${MAN7}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man7"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man7; \
		done; \
	        for F in ${CATMAN7}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat7"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat7; \
		done; \
	        for F in ${PSMAN7}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps7"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps7; \
		done; \
	    fi; \
	fi
	@if [ "${MAN8}" != "" ]; then \
	    if [ "${CATMAN8}" = "" ]; then \
	        for F in ${MAN8}; do \
	            CAT=`echo $$F | sed 's/.8$$/.cat8/'`; \
	            PS=`echo $$F | sed 's/.8$$/.ps8/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man8"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man8; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat8"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat8; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps8"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps8; \
	        done; \
	    else \
	        for F in ${MAN8}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man8"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man8; \
		done; \
	        for F in ${CATMAN8}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat8"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat8; \
		done; \
	        for F in ${PSMAN8}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps8"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps8; \
		done; \
	    fi; \
	fi
	@if [ "${MAN9}" != "" ]; then \
	    if [ "${CATMAN9}" = "" ]; then \
	        for F in ${MAN9}; do \
	            CAT=`echo $$F | sed 's/.9$$/.cat9/'`; \
	            PS=`echo $$F | sed 's/.9$$/.ps9/'`; \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man9"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man9; \
	            echo "${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat9"; \
	            ${INSTALL_DATA} $$CAT ${INST_MANDIR}/cat9; \
	            echo "${INSTALL_DATA} $$PS ${INST_PSDIR}/ps9"; \
	            ${INSTALL_DATA} $$PS ${INST_PSDIR}/ps9; \
	        done; \
	    else \
	        for F in ${MAN9}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man9"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/man9; \
		done; \
	        for F in ${CATMAN9}; do \
	            echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat9"; \
	            ${INSTALL_DATA} $$F ${INST_MANDIR}/cat9; \
		done; \
	        for F in ${PSMAN9}; do \
	            echo "${INSTALL_DATA} $$F ${INST_PSDIR}/ps9"; \
	            ${INSTALL_DATA} $$F ${INST_PSDIR}/ps9; \
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

.PHONY: clean install deinstall preformat-man clean-man install-man install-man-dirs deinstall-man man

include ${TOP}/mk/csoft.common.mk
