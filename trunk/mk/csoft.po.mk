# $Csoft: csoft.po.mk,v 1.5 2003/07/27 22:14:18 vedge Exp $

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
	@if [ "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "${MAKE} ${DOMAIN}.pot"; \
		${MAKE} ${DOMAIN}.pot; \
		echo "${MSGMERGE} $< ${DOMAIN}.pot -o $@.pox"; \
		${MSGMERGE} $< ${DOMAIN}.pot -o $@.pox; \
	fi

.po.mo:
	@if [ "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "${MSGFMT} -o $@ $<"; \
		${MSGFMT} -o $@ $<; \
	fi

${POTFILES}:
	@if [ "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "(cd ${SRC} && find . -name \*.c > ${POTFILES})"; \
		(cwd=`pwd`; cd ${SRC} && find . -name \*.c > \
		    $$cwd/${POTFILES}); \
	fi

${DOMAIN}.pot: ${POTFILES}
	@if [ "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "${XGETTEXT} --default-domain=${DOMAIN} \
		    --directory=${SRC} --add-comments \
		    --keyword=_ --keyword=N_ \
		    --files-from=${POTFILES} -o $@"; \
		${XGETTEXT} --default-domain=${DOMAIN} \
		    --directory=${SRC} --add-comments \
		    --keyword=_ --keyword=N_ \
		    --files-from=${POTFILES} -o $@; \
	fi

depend:
	# nothing

regress:
	# nothing

clean:
	if [ "${POTFILES}" != "" -o "${MOS}" != "" ]; then \
		echo "rm -f ${POTFILES} ${MOS}"; \
		rm -f ${POTFILES} ${MOS}; \
	fi

cleandir:
	# nothing

install: ${MOS}
	@export _mos="${MOS}"; \
	if [ "${HAVE_GETTEXT}" = "yes" -a "$$_mos" != "" ]; then \
            if [ ! -d "${LOCALEDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}"; \
                ${INSTALL_DATA_DIR} ${LOCALEDIR}; \
            fi; \
            for F in $$_mos; do \
	        _lang=`echo $$F | sed 's,\.mo,,'`; \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang"; \
                ${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang; \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang/LC_MESSAGES"; \
                ${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang/LC_MESSAGES; \
		cp -f $$F ${DOMAIN}.mo; \
                echo "${INSTALL_DATA} ${DOMAIN}.mo \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES"; \
                ${INSTALL_DATA} ${DOMAIN}.mo \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES; \
		rm -f ${DOMAIN}.mo; \
            done; \
	fi

deinstall:
	@export _mos="${MOS}"; \
        if [ "${HAVE_GETTEXT}" = "yes" -a "$$_mos" != "" ]; then \
            for F in $$_mos; do \
	        _lang=`echo $$F | sed 's,\.mo,,'`; \
                echo "${DEINSTALL_DATA} \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES/${DOMAIN}.mo"; \
                ${DEINSTALL_DATA} \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES/${DOMAIN}.mo; \
            done; \
	fi

.PHONY: ${POTFILES}

include ${TOP}/mk/csoft.common.mk
