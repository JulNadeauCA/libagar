# $Csoft: csoft.doc.mk,v 1.1 2004/01/19 10:30:30 vedge Exp $

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

DOCPS?=		untitled.ps
DOCPDF?=	untitled.pdf
DOCSRC?=

ROFF?=		nroff
ROFFFLAGS?=
MACROS?=
EQN?=		eqn
EQNFLAGS?=
PIC?=		pic
PICFLAGS?=
TBL?=		tbl
TBLFLAGS?=
REFER?=		refer
REFERFLAGS?=
REFERDB?=
PS2PDF?=	ps2pdf13
PS2PDFFLAGS?=

all: all-subdir ${DOCPS} ${DOCPDF}
install: install-doc-dirs install-doc install-subdir
deinstall: deinstall-subdir
clean: clean-doc clean-subdir
cleandir: clean-doc clean-subdir cleandir-subdir
regress: regress-subdir
depend: depend-subdir

${DOCPS}: ${DOCSRC} ${REFERDB}
	@if [ "${REFERDB}" = "" ]; then \
		echo "cat ${DOCSRC} | ${PIC} ${PICFLAGS}| ${EQN} ${EQNFLAGS}|\
		    ${TBL} ${TBLFLAGS} | ${REFER} ${REFERFLAGS} |\
		    ${ROFF} ${ROFFFLAGS} -Tps ${MACROS} > $@"; \
		(cat ${DOCSRC} | ${PIC} ${PICFLAGS}| ${EQN} ${EQNFLAGS}|\
		    ${TBL} ${TBLFLAGS} | ${REFER} ${REFERFLAGS} |\
		    ${ROFF} ${ROFFFLAGS} -Tps ${MACROS} > $@) || \
		    (rm -f $@; false); \
	else \
		echo "cat ${DOCSRC} | ${PIC} ${PICFLAGS}| ${EQN} ${EQNFLAGS}|\
		    ${TBL} ${TBLFLAGS} | ${REFER} ${REFERFLAGS} -p${REFERDB} |\
		    ${ROFF} ${ROFFFLAGS} -Tps ${MACROS} > $@"; \
		(cat ${DOCSRC} | ${PIC} ${PICFLAGS}| ${EQN} ${EQNFLAGS}|\
		    ${TBL} ${TBLFLAGS} | ${REFER} ${REFERFLAGS} -p${REFERDB} |\
		    ${ROFF} ${ROFFFLAGS} -Tps ${MACROS} > $@) || \
		    (rm -f $@; false); \
	fi

${DOCPDF}: ${DOCPS}
	(${PS2PDF} ${PS2PDFFLAGS} ${DOCPS} > ${DOCPDF}) || (rm -f $@; false)

clean-doc:
	rm -f ${DOCPS} ${DOCPDF}

.PHONY: install deinstall clean cleandir regress depend
.PHONY: clean-doc

include ${TOP}/mk/csoft.subdir.mk
include ${TOP}/mk/csoft.common.mk
