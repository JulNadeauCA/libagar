# $Csoft: csoft.po.mk,v 1.35 2004/04/25 05:23:00 vedge Exp $

# Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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
XGETTEXT_FLAGS?=--no-location
MSGMERGE?=	msgmerge
MSGMERGE_FLAGS?=--no-location
MSGFMT?=	msgfmt
DOMAIN?=	untitled
POTFILES?=	POTFILES
SRC?=		..
POS?=
MOS?=

.SUFFIXES: .c .po .pox .mo

all: all-subdir ${DOMAIN}.pot ${MOS}
install: install-po install-subdir
deinstall: deinstall-po deinstall-subdir
clean: clean-po clean-subdir
cleandir: clean-po clean-subdir cleandir-subdir
regress: regress-subdir
depend: depend-subdir

.po.pox:
	@if [ "${ENABLE_NLS}" = "yes" -a "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "${MAKE} ${DOMAIN}.pot"; \
		${MAKE} ${DOMAIN}.pot; \
		echo "${MSGMERGE} ${MSGMERGE_FLAGS} $< ${DOMAIN}.pot -o $@"; \
		${MSGMERGE} ${MSGMERGE_FLAGS} $< ${DOMAIN}.pot -o $@; \
	else \
		echo "skipping $@ (no gettext)"; \
	fi

.po.mo:
	@if [ "${ENABLE_NLS}" = "yes" -a "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "${MSGFMT} -o $@ $<"; \
		${MSGFMT} -o $@ $<; \
	else \
		echo "skipping $@ (no gettext)"; \
	fi

${POTFILES}:
	@if [ "${ENABLE_NLS}" = "yes" -a "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "(cd ${SRC} && find . -name \*.c > ${POTFILES})"; \
		(cwd=`pwd`; cd ${SRC} && find . -name \*.c > \
		    $$cwd/${POTFILES}); \
	else \
		echo "skipping $@ (no gettext)"; \
	fi

${DOMAIN}.pot: ${POTFILES}
	@if [ "${ENABLE_NLS}" = "yes" -a "${HAVE_GETTEXT}" = "yes" ]; then \
		echo "${XGETTEXT} --default-domain=${DOMAIN} \
		    --directory=${SRC} --add-comments \
		    --keyword=_ --keyword=N_ \
		    --files-from=${POTFILES} ${XGETTEXT_FLAGS} -o $@"; \
		${XGETTEXT} --default-domain=${DOMAIN} \
		    --directory=${SRC} --add-comments \
		    --keyword=_ --keyword=N_ \
		    --files-from=${POTFILES} ${XGETTEXT_FLAGS} --from-code=UTF-8 -o $@; \
	else \
		echo "skipping $@ (no gettext)"; \
	fi

clean-po:
	if [ "${POTFILES}" != "" -o "${MOS}" != "" ]; then \
		echo "rm -f ${POTFILES} ${MOS}"; \
		rm -f ${POTFILES} ${MOS}; \
	fi

install-po:
	@export _mos="${MOS}"; \
	if [ "${ENABLE_NLS}" = "yes" -a "${HAVE_GETTEXT}" = "yes" \
	     -a "$$_mos" != "" ]; then \
            if [ ! -d "${LOCALEDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${LOCALEDIR}; \
            fi; \
            for F in $$_mos; do \
	        _lang=`echo $$F | sed 's,\.mo,,'`; \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang; \
                echo "${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang/LC_MESSAGES"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${LOCALEDIR}/$$_lang/LC_MESSAGES; \
		cp -f $$F ${DOMAIN}.mo; \
                echo "${INSTALL_DATA} ${DOMAIN}.mo \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES"; \
                ${SUDO} ${INSTALL_DATA} ${DOMAIN}.mo \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES; \
		rm -f ${DOMAIN}.mo; \
            done; \
	else \
		echo "skipping $@ (no gettext)"; \
	fi

deinstall-po:
	@export _mos="${MOS}"; \
        if [ "${HAVE_GETTEXT}" = "yes" -a "$$_mos" != "" ]; then \
            for F in $$_mos; do \
	        _lang=`echo $$F | sed 's,\.mo,,'`; \
                echo "${DEINSTALL_DATA} \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES/${DOMAIN}.mo"; \
                ${SUDO} ${DEINSTALL_DATA} \
		    ${LOCALEDIR}/$$_lang/LC_MESSAGES/${DOMAIN}.mo; \
            done; \
	fi

count:
	@for F in ${POS}; do \
		echo -n "$$F: "; \
		cat $$F |grep "^msgid " |cut -c 7- |\
		    perl -pe '!s/^\"(.*)\"/$$1/' | \
		    perl -pe '!s/<.+>//g' | \
		    perl -pe '!s/\\n//g' | \
		    perl -pe '!s/&(lt|gt|nbsp|copy);//g' | \
		    perl -pe '!s/`%[diouxXDOUeEfgGcspn]'\''/$$1/g' | \
		    perl -pe '!s/%[diouxXDOUeEfgGcspn]/$$1/g' | \
		    wc -w; \
	done

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-po deinstall-po clean-po
.PHONY: ${POTFILES} count

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
