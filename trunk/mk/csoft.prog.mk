# $Csoft: csoft.prog.mk,v 1.30 2003/08/13 03:57:04 vedge Exp $

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

CFLAGS?=	-Wall -g

PROG_INSTALL?=	Yes

CC?=		cc
CFLAGS?=	-O2
CPPFLAGS?=
GMONOUT?=	gmon.out

ASM?=		nasm
ASMFLAGS?=	-g -w-orphan-labels

LEX?=		lex
LIBL?=		-ll
LFLAGS?=

YACC?=		yacc
YFLAGS?=	-d

SHARE?=

.SUFFIXES: .o .po .c .cc .asm .l .y

# C
.c.o:
	${CC} ${CFLAGS} ${CPPFLAGS} -c $<
.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

# C++
.cc.o:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -c $<
.cc.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<

# Assembly
.asm.o:
	${ASM} ${ASMFLAGS} ${CPPFLAGS} -o $@ $<

# Lex
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

# Yacc
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

all: all-subdir ${PROG}

_prog_objs:
	@if [ "${PROG}" != "" -a "${OBJS}" = "" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	        ${MAKE} $$F; \
	    done; \
	fi

_prog_pobjs:
	@if [ "${GMONOUT}" != "" -a "${POBJS}" = "" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[cly]$$/.po/'`; \
	        F=`echo $$F | sed 's/.cc$$/.po/'`; \
	        F=`echo $$F | sed 's/.asm$$/.po/'`; \
	        ${MAKE} $$F; \
	    done; \
	fi

${PROG}: _prog_objs ${OBJS}
	@if [ "${PROG}" != "" ]; then \
	    if [ "${OBJS}" = "" ]; then \
	        export _objs=""; \
                for F in ${SRCS}; do \
	            F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    _objs="$$_objs $$F"; \
                done; \
	        echo "${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} $$_objs ${LIBS}"; \
	        ${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} $$_objs ${LIBS}; \
	    else \
	        echo "${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}"; \
	        ${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}; \
	    fi; \
	fi

${GMONOUT}: _prog_pobjs ${POBJS}
	if [ "${GMONOUT}" != "" ]; then \
	    if [ "${OBJS}" = "" ]; then \
	        export _pobjs=""; \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[cly]$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.po/'`; \
	    	    _pobjs="$$_pobjs $$F"; \
                done; \
	        echo "${CC} -pg -DPROF ${LDFLAGS} -o ${GMONOUT} $$_pobjs \
		    ${LIBS}"; \
	        ${CC} -pg -DPROF ${LDFLAGS} -o ${GMONOUT} $$_pobjs ${LIBS}; \
	    else \
	        echo "${CC} -pg -DPROF ${LDFLAGS} -o ${GMONOUT} ${POBJS} \
		    ${LIBS}"; \
	        ${CC} -pg -DPROF ${LDFLAGS} -o ${GMONOUT} ${POBJS} ${LIBS}; \
	    fi; \
	fi

clean: clean-subdir
	@if [ "${PROG}" != "" ]; then \
	    if [ "${OBJS}" = "" ]; then \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    echo "rm -f $$F"; \
	    	    rm -f $$F; \
                done; \
	    else \
	        echo "rm -f ${OBJS}"; \
	        rm -f ${OBJS}; \
	    fi; \
	    if [ "${POBJS}" = "" ]; then \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[cly]$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.po/'`; \
	    	    echo "rm -f $$F"; \
	    	    rm -f $$F; \
                done; \
	    else \
	        echo "rm -f ${POBJS}"; \
	        rm -f ${POBJS}; \
	    fi; \
	    echo "rm -f ${PROG} ${GMONOUT}"; \
	    rm -f ${PROG} ${GMONOUT}; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi

cleandir: clean cleandir-subdir clean-depend
	rm -f *.core *~

install: install-subdir ${PROG}
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${INSTALL_PROG} ${PROG} ${INST_BINDIR}"; \
	    ${INSTALL_PROG} ${PROG} ${INST_BINDIR}; \
	fi
	@export _share="${SHARE}"; \
        if [ "$$_share" != "" ]; then \
            if [ ! -d "${SHAREDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
                ${INSTALL_DATA_DIR} ${SHAREDIR}; \
            fi; \
            for F in $$_share; do \
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

.PHONY: clean cleandir install deinstall regress depend _prog_objs _prog_pobjs

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.dep.mk
include ${TOP}/mk/csoft.subdir.mk
