# $Csoft: csoft.den.mk,v 1.2 2004/03/17 03:49:16 vedge Exp $
# ex:syn=make

# Copyright (c) 2004 CubeSoft Communications, Inc.
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

DENCOMP?=	dencomp
DENCOMP_FLAGS?=

XCF?=
WAV?=
OGG?=
MPEG?=
DEN_INPUT=	${XCF} ${WAV} ${OGG} ${MPEG}

DEN?=
DEN_LANG?=	en
DEN_NAME?=	${DEN}
DEN_AUTHOR?=	"Auguste Pocuste"
DEN_COPYRIGHT?=	"None"
DEN_DESCR?=	""
DEN_KEYWORDS?=	""

all: all-subdir ${DEN}
install: install-den install-subdir
deinstall: deinstall-den deinstall-subdir
clean: clean-den clean-subdir
cleandir: cleandir-subdir
regress: regress-subdir
depend: depend-subdir

${DEN}: ${DEN_INPUT}
	@(if [ "${XCF}" != "" ]; then \
		_den_hint="gfx"; \
	 elif [ "${WAV}" != "" ]; then \
		_den_hint="audio"; \
	 elif [ "${OGG}" != "" ]; then \
		_den_hint="audio"; \
	 elif [ "${MPEG}" != "" ]; then \
		_den_hint="video"; \
	 fi; \
	 ${DENCOMP} ${DENCOMP_FLAGS} -o ${DEN} -h $$_den_hint \
	   -l ${DEN_LANG} -n ${DEN_NAME} -a ${DEN_AUTHOR} \
	   -c ${DEN_COPYRIGHT} -d ${DEN_DESCR} -k ${DEN_KEYWORDS} \
	   ${DEN_INPUT})

clean-den:
	if [ "${DEN}" != "" ]; then \
		rm -f ${DEN}; \
	fi

install-den:
	if [ "${DEN}" != "" ]; then \
		if [ "${DEN_DIR}" = "" ]; then \
			echo "Makefile is missing DEN_DIR"; \
			exit 1; \
		fi; \
		if [ ! -d "${SHAREDIR}/${DEN_DIR}" ]; then \
			echo "${INSTALL_DATA_DIR} ${SHAREDIR}/${DEN_DIR}"; \
			${INSTALL_DATA_DIR} ${SHAREDIR}/${DEN_DIR}; \
		fi; \
		${INSTALL_DATA} ${DEN} ${SHAREDIR}/${DEN_DIR}; \
	fi

deinstall-den:
	if [ "${DEN}" != "" ]; then \
		echo "${DEINSTALL_DATA} ${SHAREDIR}/${DEN_DIR}/${DEN}"; \
		${DEINSTALL_DATA} ${SHAREDIR}/${DEN_DIR}/${DEN}; \
	fi

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-prog deinstall-prog clean-prog cleandir-prog

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.dep.mk
include ${TOP}/mk/csoft.subdir.mk
