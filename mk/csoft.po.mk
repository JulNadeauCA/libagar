# $Csoft: csoft.po.mk,v 1.2 2003/06/21 07:26:19 vedge Exp $

# Copyright (c) 2003 CubeSoft Communications, Inc.
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

XGETTEXT?=	xgettext
MSGMERGE?=	msgmerge
MSGFMT?=	msgfmt
DOMAIN?=	untitled
POTFILES?=	POTFILES
SRC?=		..
POS?=
MOS?=

.SUFFIXES: .c .po .pox .mo

all: ${DOMAIN}.pot ${MOS}

.po.pox:
	${MAKE} ${DOMAIN}.pot
	${MSGMERGE} $< ${DOMAIN}.pot -o $*.pox

.po.mo:
	${MSGFMT} -o $@ $<

${POTFILES}:
	(cwd=`pwd`; cd ${SRC} && find . -name \*.c > $$cwd/${POTFILES})

${DOMAIN}.pot: ${POTFILES}
	${XGETTEXT} --default-domain=${DOMAIN} --directory=${SRC} \
	    --add-comments --keyword=_ --keyword=N_ \
	    --files-from=${POTFILES} -o $@

depend:
	# nothing

regress:
	# nothing

clean:
	rm -f ${POTFILES}

cleandir:
	# nothing

install: ${MOS}
	@export _mos="${MOS}"; \
        if [ "$$_mos" != "" ]; then \
            if [ ! -d "${LOCALEDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}"; \
                ${INSTALL_DATA_DIR} ${LOCALEDIR}; \
            fi; \
            for F in $$_mos; do \
                echo "${INSTALL_DATA} $$F ${LOCALEDIR}"; \
                ${INSTALL_DATA} $$F ${LOCALEDIR}; \
            done; \
	fi

deinstall:
	@if [ "${MOS}" != "" ]; then \
	    for F in ${MOS}; do \
	        echo "${DEINSTALL_DATA} ${LOCALEDIR}/$$F"; \
	        ${DEINSTALL_DATA} ${LOCALEDIR}/$$F; \
	    done; \
	fi

.PHONY: ${POTFILES}

include ${TOP}/mk/csoft.common.mk
