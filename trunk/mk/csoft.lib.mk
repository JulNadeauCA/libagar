# $Csoft: csoft.lib.mk,v 1.31 2003/10/05 01:41:41 vedge Exp $

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
CFLAGS+=    ${COPTS}
SHARE?=
LFLAGS?=
YFLAGS?=

all: all-subdir lib${LIB}.a lib${LIB}.la
install: install-lib install-subdir
deinstall: deinstall-lib deinstall-subdir
clean: clean-lib clean-subdir
cleandir: cleandir-lib cleandir-subdir
regress: regress-subdir
depend: depend-subdir

.SUFFIXES: .o .po .lo .c .cc .asm .l .y

# Compile C code into an object file
.c.o:
	${CC} ${CFLAGS} -c $<
.c.lo:
	${LIBTOOL} ${CC} ${CFLAGS} -c $<
.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile C++ code into an object file
.cc.o:
	${CXX} ${CXXFLAGS} -c $<
.cc.lo:
	${LIBTOOL} ${CXX} ${CXXFLAGS} -c $<
.cc.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile assembly code into an object file
.asm.o:
	${ASM} ${ASMFLAGS} ${CPPFLAGS} -o $@ $<

# Compile a Lex lexer into an object file
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

# Compile a Yacc parser into an object file
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

# Build the library's object files.
_lib_objs:
	@if [ "${LIB}" != "" -a "${OBJS}" = "" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	        ${MAKE} $$F; \
		if [ $$? != 0 ]; then \
			echo "${MAKE}: failure"; \
			exit 1; \
		fi; \
            done; \
	fi

# Build PIC versions of the library's object files.
_lib_shobjs:
	@if [ "${LIB}" != "" -a "${SHOBJS}" = "" \
	      -a "${SHARED}" = "Yes" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[cly]$$/.so/'`; \
	        F=`echo $$F | sed 's/.cc$$/.so/'`; \
	        F=`echo $$F | sed 's/.asm$$/.so/'`; \
	        ${MAKE} $$F; \
		if [ $$? != 0 ]; then \
			echo "${MAKE}: failure"; \
			exit 1; \
		fi; \
            done; \
	fi

# Build a static version of the library.
lib${LIB}.a: _lib_objs ${OBJS}
	@if [ "${LIB}" != "" ]; then \
	    if [ "${OBJS}" = "" ]; then \
	        export _objs=""; \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[cly]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    _objs="$$_objs $$F"; \
                done; \
	        echo "${AR} -cru lib${LIB}.a $$_objs"; \
	        ${AR} -cru lib${LIB}.a $$_objs; \
	    else \
	        echo "${AR} -cru lib${LIB}.a ${OBJS}"; \
	        ${AR} -cru lib${LIB}.a ${OBJS}; \
	    fi; \
	    echo "${RANLIB} lib${LIB}.a"; \
	    (${RANLIB} lib${LIB}.a || exit 0); \
	fi

# Build a Libtool version of the library.
lib${LIB}.la: ${LIBTOOL} _lib_shobjs
	@if [ "${LIB}" != "" -a "${SHARED}" = "Yes" ]; then \
	    if [ "${SHOBJS}" = "" ]; then \
	        export _shobjs=""; \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[cly]$$/.so/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.so/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.so/'`; \
	    	    _shobjs="$$_shobjs $$F"; \
                done; \
	        echo "${LIBTOOL} ${CC} -o lib${LIB}.la -rpath ${PREFIX}/lib \
	            -shared -version-info ${SOVERSION} ${LDFLAGS} $$_shobjs \
		    ${LIBS}"; \
	        ${LIBTOOL} ${CC} -o lib${LIB}.la -rpath ${PREFIX}/lib -shared \
		    -version-info ${SOVERSION} ${LDFLAGS} $$_shobjs ${LIBS}; \
	    else \
	        echo "${LIBTOOL} ${CC} -o lib${LIB}.la -rpath ${PREFIX}/lib \
	            -shared -version-info ${SOVERSION} ${LDFLAGS} ${SHOBJS} \
		    ${LIBS}"; \
	        ${LIBTOOL} ${CC} -o lib${LIB}.la -rpath ${PREFIX}/lib -shared \
		    -version-info ${SOVERSION} ${LDFLAGS} ${SHOBJS} ${LIBS}; \
	    fi; \
	fi

clean-lib:
	@if [ "${LIB}" != "" ]; then \
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
	    echo "rm -f lib${LIB}.a"; \
	    rm -f lib${LIB}.a; \
	    if [ "${SHARED}" = "Yes" ]; then \
	        if [ "${OBJS}" != "" ]; then \
                    for F in ${SRCS}; do \
	    	        F=`echo $$F | sed 's/.[cly]$$/.so/'`; \
	    	        F=`echo $$F | sed 's/.cc$$/.so/'`; \
	    	        F=`echo $$F | sed 's/.asm$$/.so/'`; \
	    	        echo "rm -f $$F"; \
	    	        rm -f $$F; \
                    done; \
		else \
		    rm -f ${OBJS}; \
		    echo "rm -f ${OBJS}"; \
		fi; \
		echo "rm -f lib${LIB}.la ${LIBTOOL} ${LTCONFIG_LOG}"; \
		rm -f lib${LIB}.la ${LIBTOOL} ${LTCONFIG_LOG}; \
	    fi; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi

cleandir-lib:
	rm -fR .libs

install-lib:
	@if [ "${LIB}" != "" -a "${LIB_INSTALL}" != "No" ]; then \
	    echo "${INSTALL_LIB} lib${LIB}.a ${INST_LIBDIR}"; \
	    ${SUDO} ${INSTALL_LIB} lib${LIB}.a ${INST_LIBDIR}; \
	    if [ "${SHARED}" = "Yes" ]; then \
	        echo "${LIBTOOL} --mode=install \
	            ${INSTALL_LIB} lib${LIB}.la ${INST_LIBDIR}"; \
	        ${SUDO} ${LIBTOOL} --mode=install \
	            ${INSTALL_LIB} lib${LIB}.la ${INST_LIBDIR}; \
	    fi; \
	fi
	@export _share="${SHARE}"; \
        if [ "$$_share" != "" ]; then \
            if [ ! -d "${SHAREDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${SHAREDIR}; \
            fi; \
            for F in $$_share; do \
                echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA} $$F ${SHAREDIR}; \
            done; \
	fi

deinstall-lib:
	@if [ "${LIB}" != "" -a "${LIB_INSTALL}" != "No" ]; then \
	    echo "${DEINSTALL_LIB} ${PREFIX}/lib/lib${LIB}.a"; \
	    ${SUDO} ${DEINSTALL_LIB} ${PREFIX}/lib/lib${LIB}.a; \
	    if [ "${SHARED}" == "Yes" ]; then \
	        echo "${LIBTOOL} --mode=uninstall \
		    rm -f ${PREFIX}/lib/lib${LIB}.la"; \
	        ${SUDO} ${LIBTOOL} --mode=uninstall \
		    rm -f ${PREFIX}/lib/lib${LIB}.la; \
	    fi; \
	fi
	@if [ "${SHARE}" != "" ]; then \
	    for F in ${SHARE}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${SHAREDIR}/$$F; \
	    done; \
	fi

${LIBTOOL}: ${LTCONFIG} ${LTMAIN_SH} ${LTCONFIG_GUESS} ${LTCONFIG_SUB}
	@if [ "${LIB}" != "" -a "${SHARED}" = "Yes" ]; then \
	    echo "${SH} ${LTCONFIG} ${LTMAIN_SH}"; \
	    ${SH} ${LTCONFIG} ${LTMAIN_SH}; \
	fi

${LTCONFIG} ${LTCONFIG_GUESS} ${LTCONFIG_SUB} ${LTMAIN_SH}:

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-lib deinstall-lib clean-lib cleandir-lib
.PHONY: _lib_objs _lib_shobjs

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.dep.mk
include ${TOP}/mk/csoft.subdir.mk
