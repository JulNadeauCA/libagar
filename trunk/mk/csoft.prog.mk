# $Csoft: csoft.prog.mk,v 1.19 2002/02/20 23:24:09 vedge Exp $

# Copyright (c) 2001 CubeSoft Communications, Inc.
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

CFLAGS?=	-Wall -g

PROG_INSTALL?=	Yes

CC?=		cc
CFLAGS?=	-O2
CPPFLAGS?=
CC_PICFLAGS?=	-fPIC -DPIC
GMONOUT?=	gmon.out

ASSEMBLER?=	nasm
ASMFLAGS?=	-g -w-orphan-labels
ASM_PICFLAGS?=	-DPIC

LEX?=		lex
LIBL?=		-ll
LFLAGS?=

YACC?=		yacc
YFLAGS?=	-d

SHARE?=

.SUFFIXES: .o .po .so .c .cc .C .cxx .y .s .S .asm .l

#
# C
#
.c.o:
	${CC} ${CFLAGS} ${CPPFLAGS} -c $<
.cc.o:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -c $<
.s.o .S.o:
	${CC} ${CFLAGS} ${CPPFLAGS} -c $<
.c.so:
	${CC} ${CC_PICFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
.cc.so:
	${CXX} ${CC_PICFLAGS} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.s.so .S.so:
	${CC} ${CC_PICFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
.cc.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.s.po .S.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

#
# Assembly
#
.asm.o:
	@echo "#ifdef __ELF__" > .elftest
	@echo "IS ELF" >> .elftest
	@echo "#endif" >> .elftest
	@if [ "`cat .elftest | cpp -P -`" = "IS ELF" ]; then \
	    echo "${ASSEMBLER} -f elf ${ASMFLAGS} ${CPPFLAGS} -o $@ $<"; \
	    ${ASSEMBLER} -f elf ${ASMFLAGS} ${CPPFLAGS} -o $@ $<; \
	else \
	    echo "${ASSEMBLER} -f aoutb ${ASMFLAGS} ${CPPFLAGS} -o $@ $<"; \
	    ${ASSEMBLER} -f aoutb ${ASMFLAGS} ${CPPFLAGS} -o $@ $<; \
	fi
	@rm -f .elftest

.asm.so:
	@echo "#ifdef __ELF__" > .elftest
	@echo "IS ELF" >> .elftest
	@echo "#endif" >> .elftest
	@if [ "`cat .elftest | cpp -P -`" = "IS ELF" ]; then \
	    echo "${ASSEMBLER} -f elf ${ASMFLAGS} ${ASM_PICFLAGS} ${CPPFLAGS} -o $@ $<"; \
	    ${ASSEMBLER} -f elf ${ASMFLAGS} ${ASM_PICFLAGS} ${CPPFLAGS} -o $@ $<; \
	else \
	    echo "${ASSEMBLER} -f aoutb ${ASMFLAGS} ${ASM_PICFLAGS} ${CPPFLAGS} -o $@ $<"; \
	    ${ASSEMBLER} -f aoutb ${ASMFLAGS} ${ASM_PICFLAGS} ${CPPFLAGS} -o $@ $<; \
	fi
	@rm -f .elftest

#
# Lex
#
.l:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $@.yy.c ${LIBL} ${LIBS}
	@rm -f $@.yy.c
.l.o:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	${CC} ${CFLAGS} -c $@.yy.c
	@mv -f $@.yy.o $@
	@rm -f $@.yy.c
.l.po:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	${CC} -pg -DPROF ${CFLAGS} -c $@.yy.c
	@mv -f $@.yy.o $@
	@rm -f $@.yy.c

#
# Yacc
#
.y:
	${YACC} ${YFLAGS} -b $@ $<
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $@.tab.c ${LIBS}
	@rm -f $@.tab.c
.y.o:
	${YACC} ${YFLAGS} -b $@ $<
	${CC} ${CFLAGS} -c $@.tab.c
	@mv -f $@.tab.o $@
	@rm -f $@.tab.c
.y.po:
	${YACC} ${YFLAGS} -b $@ $<
	${CC} -pg -DPROF ${CFLAGS} -c $@.tab.c
	@mv -f $@.tab.o $@
	@rm -f $@.tab.c

all:	all-subdir ${PROG}

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}

${GMONOUT}: ${OBJS}
	${CC} -pg -DPROF ${LDFLAGS} -o ${GMONOUT} ${OBJS} ${LIBS}

clean: clean-subdir
	@if [ "${PROG}" != "" ]; then \
	    echo "rm -f ${PROG} ${GMONOUT} ${OBJS}"; \
	    rm -f ${PROG} ${GMONOUT} ${OBJS}; \
	fi

cleandir:	cleandir-subdir clean-depend
	rm -f *.core *~

install: install-subdir ${PROG}
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${INSTALL_PROG} ${PROG} ${INST_BINDIR}"; \
	    ${INSTALL_PROG} ${PROG} ${INST_BINDIR}; \
	fi
	@if [ "${SHARE}" != "" ]; then \
	    if [ ! -d "${SHAREDIR}" ]; then \
	        echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
	        ${INSTALL_DATA_DIR} ${SHAREDIR}; \
	    fi; \
	    for F in ${SHARE}; do \
	        echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
	        ${INSTALL_DATA} $$F ${SHAREDIR}; \
	    done; \
	fi

deinstall: deinstall-subdir
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${DEINSTALL_PROG} ${INST_BINDIR}/${PROG}"; \
	    ${DEINSTALL_PROG} ${INST_BINDIR}/${PROG}; \
	fi
	@if [ "${SHARE}" != "" ]; then \
	    for F in ${SHARE}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${DEINSTALL_DATA} ${SHAREDIR}/$$F; \
	    done; \
	fi

regress: regress-subdir

depend: depend-subdir

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.dep.mk
include ${TOP}/mk/csoft.subdir.mk
