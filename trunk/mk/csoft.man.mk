# $Csoft: csoft.man.mk,v 1.14 2002/05/10 22:41:48 vedge Exp $

# Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
# <http://www.csoft.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistribution of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistribution in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of CubeSoft Communications, nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
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

NROFF=nroff

MANS=${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8} ${MAN9}
CATMANS=${CATMAN1} ${CATMAN2} ${CATMAN3} ${CATMAN4} ${CATMAN5} ${CATMAN6} ${CATMAN7} ${CATMAN8} ${CATMAN9}

.SUFFIXES: .1 .2 .3 .4 .5 .6 .7 .8 .9 .cat1 .cat2 .cat3 .cat4 .cat5 .cat6 .cat7 .cat8 .cat9

.1.cat1 .2.cat2 .3.cat3 .4.cat4 .5.cat5 .6.cat6 .7.cat7 .8.cat8 .9.cat9:
	@if [ -x "`which ${NROFF}`" ]; then \
	    echo "${NROFF} -Tascii -mandoc $< > $@"; \
	    ${NROFF} -Tascii -mandoc $< > $@; \
	fi

all: ${MANS} ${CATMANS}

clean: clean-man

install: install-man

deinstall: deinstall-man

clean-man:
	@if [ "${CATMANS}" != "       " ]; then \
	    echo "rm -f ${CATMANS}"; \
	    rm -f ${CATMANS}; \
	fi

install-man: ${MANS} ${CATMANS}
	@if [ "${MANS}" != "       " ]; then \
	    for F in ${MAN1}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man1"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man1; \
	    done; \
	    for F in ${MAN2}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man2"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man2; \
	    done; \
	    for F in ${MAN3}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man3"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man3; \
	    done; \
	    for F in ${MAN4}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man4"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man4; \
	    done; \
	    for F in ${MAN5}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man5"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man5; \
	    done; \
	    for F in ${MAN6}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man6"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man6; \
	    done; \
	    for F in ${MAN7}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man7"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man7; \
	    done; \
	    for F in ${MAN8}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man8"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man8; \
	    done; \
	    for F in ${MAN9}; do \
	        echo "${INSTALL_DATA} $$F ${INST_MANDIR}/man9"; \
	        ${INSTALL_DATA} $$F ${INST_MANDIR}/man9; \
	    done; \
	fi
	@if [ "${CATMANS}" != "       " ]; then \
	    if [ -d "${INST_MANDIR}/cat1" ]; then \
	        for F in ${CATMAN1}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat1"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat1; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat2" ]; then \
	        for F in ${CATMAN2}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat2"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat2; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat3" ]; then \
	        for F in ${CATMAN3}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat3"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat3; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat4" ]; then \
	        for F in ${CATMAN4}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat4"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat4; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat5" ]; then \
	        for F in ${CATMAN5}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat5"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat5; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat6" ]; then \
	        for F in ${CATMAN6}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat6"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat6; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat7" ]; then \
	        for F in ${CATMAN7}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat7"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat7; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat8" ]; then \
	        for F in ${CATMAN8}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat8"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat8; \
	        done; \
	    fi; \
	    if [ -d "${INST_MANDIR}/cat9" ]; then \
	        for F in ${CATMAN9}; do \
	             echo "${INSTALL_DATA} $$F ${INST_MANDIR}/cat9"; \
	             ${INSTALL_DATA} $$F ${INST_MANDIR}/cat9; \
	        done; \
	    fi; \
	fi
	
deinstall-man:
	@if [ "${MANS}" != "       " ]; then \
	    for F in ${MAN1}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man1/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man1/$$F; \
	    done; \
	    for F in ${MAN2}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man2/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man2/$$F; \
	    done; \
	    for F in ${MAN3}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man3/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man3/$$F; \
	    done; \
	    for F in ${MAN4}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man4/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man4/$$F; \
	    done; \
	    for F in ${MAN5}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man5/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man5/$$F; \
	    done; \
	    for F in ${MAN6}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man6/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man6/$$F; \
	    done; \
	    for F in ${MAN7}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man7/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man7/$$F; \
	    done; \
	    for F in ${MAN8}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man8/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man8/$$F; \
	    done; \
	    for F in ${MAN9}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/man9/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/man9/$$F; \
	    done; \
	fi
	@if [ "${CATMANS}" != "       " ]; then \
	    for F in ${CATMAN1}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat1/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat1/$$F; \
	    done; \
	    for F in ${CATMAN2}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat2/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat2/$$F; \
	    done; \
	    for F in ${CATMAN3}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat3/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat3/$$F; \
	    done; \
	    for F in ${CATMAN4}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat4/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat4/$$F; \
	    done; \
	    for F in ${CATMAN5}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat5/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat5/$$F; \
	    done; \
	    for F in ${CATMAN6}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat6/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat6/$$F; \
	    done; \
	    for F in ${CATMAN7}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat7/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat7/$$F; \
	    done; \
	    for F in ${CATMAN8}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat8/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat8/$$F; \
	    done; \
	    for F in ${CATMAN9}; do \
	        echo "${DEINSTALL_DATA} ${INST_MANDIR}/cat9/$$F"; \
	        ${DEINSTALL_DATA} ${INST_MANDIR}/cat9/$$F; \
	    done; \
	fi

include ${TOP}/mk/csoft.common.mk
