# $Csoft: csoft.lib.mk,v 1.27 2003/08/12 23:19:21 vedge Exp $

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

CFLAGS?=	-Wall -O2 -g
SH?=		sh
CC?=		cc
AR?=		ar
RANLIB?=	ranlib

LIB_INSTALL=	No

ASM?=		nasm
ASMFLAGS?=	-g -w-orphan-labels

LIBTOOL?=	libtool
LTCONFIG?=	./ltconfig
LTMAIN_SH?=	./ltmain.sh
LTCONFIG_GUESS?=./config.guess
LTCONFIG_SUB?=	./config.sub
LTCONFIG_LOG?=	./config.log

BINMODE?=	755

STATIC?=	Yes
SHARED?=	No
SOVERSION?=	1:0:0

.SUFFIXES: .o .po .lo .c .cc .asm .l .y

CFLAGS+=    ${COPTS}

SHARE?=

# C
.c.o:
	${CC} ${CFLAGS} -c $<
.c.lo:
	${LIBTOOL} ${CC} ${CFLAGS} -c $<
.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

# C++
.cc.o:
	${CXX} ${CXXFLAGS} -c $<
.cc.lo:
	${LIBTOOL} ${CXX} ${CXXFLAGS} -c $<
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

all: all-subdir lib${LIB}.a lib${LIB}.la

_lib_objs:
	@if [ "${LIB}" != "" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	        ${MAKE} $$F; \
            done; \
	fi

_lib_shobjs:
	@if [ "${LIB}" != "" -a "${SHARED}" = "Yes" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[cly]$$/.so/'`; \
	        F=`echo $$F | sed 's/.cc$$/.so/'`; \
	        F=`echo $$F | sed 's/.asm$$/.so/'`; \
	        ${MAKE} $$F; \
            done; \
	fi

lib${LIB}.a: _lib_objs
	@if [ "${LIB}" != "" ]; then \
	    export _objs=""; \
	    for F in ${SRCS}; do \
	    	F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	    	F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	_objs="$$_objs $$F"; \
            done; \
	    echo "${AR} -cru lib${LIB}.a $$_objs"; \
	    ${AR} -cru lib${LIB}.a $$_objs; \
	    echo "${RANLIB} lib${LIB}.a"; \
	    (${RANLIB} lib${LIB}.a || exit 0); \
	fi

lib${LIB}.la: ${LIBTOOL} _lib_shobjs
	@if [ "${LIB}" != "" -a "${SHARED}" = "Yes" ]; then \
	    export _shobjs=""; \
	    for F in ${SRCS}; do \
	    	F=`echo $$F | sed 's/.[cly]$$/.so/'`; \
	    	F=`echo $$F | sed 's/.cc$$/.so/'`; \
	    	F=`echo $$F | sed 's/.asm$$/.so/'`; \
	    	_shobjs="$$_objs $$F"; \
            done; \
	    echo "${LIBTOOL} ${CC} -o lib${LIB}.la -rpath ${PREFIX}/lib \
	     -shared -version-info ${SOVERSION} ${LDFLAGS} $$_shobjs ${LIBS}"; \
	    ${LIBTOOL} ${CC} -o lib${LIB}.la -rpath ${PREFIX}/lib -shared \
		-version-info ${SOVERSION} ${LDFLAGS} $$_shobjs ${LIBS}; \
	fi

clean: clean-subdir
	@if [ "${LIB}" != "" ]; then \
            for F in ${SRCS}; do \
	    	F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	    	F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	echo "rm -f $$F"; \
	    	rm -f $$F; \
            done; \
	    echo "rm -f lib${LIB}.a"; \
	    rm -f lib${LIB}.a; \
	    if [ "${SHARED}" = "Yes" ]; then \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[cly]$$/.so/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.so/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.so/'`; \
	    	    echo "rm -f $$F"; \
	    	    rm -f $$F; \
                done; \
		echo "rm -f lib${LIB}.la ${LIBTOOL} ${LTCONFIG_LOG}"; \
		rm -f lib${LIB}.la ${LIBTOOL} ${LTCONFIG_LOG}; \
	    fi; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi

cleandir: clean cleandir-subdir clean-depend
	rm -fR .libs

install: install-subdir lib${LIB}.a lib${LIB}.la
	@if [ "${LIB}" != "" -a "${LIB_INSTALL}" != "No" ]; then \
	    echo "${INSTALL_LIB} lib${LIB}.a ${INST_LIBDIR}"; \
	    ${INSTALL_LIB} lib${LIB}.a ${INST_LIBDIR}; \
	    if [ "${SHARED}" = "Yes" ]; then \
	        echo "${LIBTOOL} --mode=install \
	            ${INSTALL_LIB} lib${LIB}.la ${INST_LIBDIR}"; \
	        ${LIBTOOL} --mode=install \
	            ${INSTALL_LIB} lib${LIB}.la ${INST_LIBDIR}; \
	    fi; \
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
	@if [ "${LIB}" != "" -a "${LIB_INSTALL}" != "No" ]; then \
	    echo "${DEINSTALL_LIB} ${PREFIX}/lib/lib${LIB}.a"; \
	    ${DEINSTALL_LIB} ${PREFIX}/lib/lib${LIB}.a; \
	    if [ "${SHARED}" == "Yes" ]; then \
	        echo "${LIBTOOL} --mode=uninstall \
		    rm -f ${PREFIX}/lib/lib${LIB}.la"; \
	        ${LIBTOOL} --mode=uninstall \
		    rm -f ${PREFIX}/lib/lib${LIB}.la; \
	    fi; \
	fi
	@if [ "${SHARE}" != "" ]; then \
	    for F in ${SHARE}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${DEINSTALL_DATA} ${SHAREDIR}/$$F; \
	    done; \
	fi

${LIBTOOL}: ${LTCONFIG} ${LTMAIN_SH} ${LTCONFIG_GUESS} ${LTCONFIG_SUB}
	@if [ "${LIB}" != "" -a "${SHARED}" = "Yes" ]; then \
	    echo "${SH} ${LTCONFIG} ${LTMAIN_SH}"; \
	    ${SH} ${LTCONFIG} ${LTMAIN_SH}; \
	fi

${LTCONFIG} ${LTCONFIG_GUESS} ${LTCONFIG_SUB} ${LTMAIN_SH}:

depend: depend-subdir

regress: regress-subdir

.PHONY:	clean cleandir install deinstall depend regress _lib_objs _lib_shobjs

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.dep.mk
include ${TOP}/mk/csoft.subdir.mk
