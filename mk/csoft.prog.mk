# $Csoft: csoft.prog.mk,v 1.8 2002/01/26 00:20:35 vedge Exp $

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

PREFIX?=	/usr/local
CFLAGS?=	-Wall -g
INSTALL?=	install

BINMODE=	755

CC?=		cc
CFLAGS?=	-O2
CPPFLAGS?=
CC_PICFLAGS?=	-fPIC -DPIC
GMONOUT?=	gmon.out

ASM?=		nasm
ASMOUT?=	aoutb
ASMFLAGS?=	-f ${ASMOUT} -g -w-orphan-labels
ASM_PICFLAGS?=	-DPIC

LEX?=		lex
LIBL?=		-ll
LFLAGS?=

YACC?=		yacc
YFLAGS?=	-d

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
	${ASM} ${ASMFLAGS} ${CPPFLAGS} -o $@ $< 
.asm.so:
	${ASM} ${ASM_PICFLAGS} ${ASMFLAGS} ${CPPFLAGS} -o $@ $< 

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
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}

${GMONOUT}: ${OBJS}
	${CC} -pg -DPROF ${LDFLAGS} -o ${GMONOUT} ${OBJS} ${LIBS}

clean: clean-subdir
	rm -f ${PROG} ${GMONOUT} ${OBJS}

install: install-subdir ${PROG}
	@if [ "${PROG}" != "" ]; then \
	    echo "${INSTALL} ${INSTALL_COPY} ${INSTALL_STRIP} \
	    -m ${BINMODE} ${PROG} ${PREFIX}/bin"; \
	    ${INSTALL} ${INSTALL_COPY} ${INSTALL_STRIP} \
	    -m ${BINMODE} ${PROG} ${PREFIX}/bin; \
	fi
	
uninstall: uninstall-subdir
	@if [ "${PROG}" != "" ]; then \
	    echo "rm -f ${PROG} ${PREFIX}/bin"; \
	    rm -f ${PROG} ${PREFIX}/bin; \
	fi

regress: regress-subdir

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
