#
# Copyright (c) 2001-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#
# Build static and shared libraries from source.
#

LIB?=
WINRES?=

CC?=		cc
ASM?=		nasm
LEX?=		lex
YACC?=		yacc
SH?=		sh
AR?=		ar
RANLIB?=	ranlib

CFLAGS?=
CPPFLAGS?=
CXXFLAGS?=
OBJCFLAGS?=	${CFLAGS}
ASMFLAGS?=	-g -w-orphan-labels
LFLAGS?=
LIBL?=		-ll
YFLAGS?=	-d

LIB_INSTALL?=	No
LIB_SHARED?=	No
LIB_MAJOR?=	1
LIB_MINOR?=	0
LIB_XOBJS?=
LIB_GUID?=

USE_LIBTOOL?=	Yes
LIBTOOL?=	${TOP}/mk/libtool/libtool
LIBTOOL_COOKIE?=${TOP}/mk/libtool.ok
LTCONFIG?=	${TOP}/mk/libtool/ltconfig
LTCONFIG_GUESS?=${TOP}/mk/libtool/config.guess
LTCONFIG_SUB?=	${TOP}/mk/libtool/config.sub
LTMAIN_SH?=	${TOP}/mk/libtool/ltmain.sh
LTCONFIG_LOG?=	./config.log
LIBTOOLFLAGS?=

SHARE?=none
SHARESRC?=none
SRCS?=none
OBJS?=none
SHOBJS?=none
CONF?=none
CONF_OVERWRITE?=No
INCL?=none
INCLDIR?=
CLEANFILES?=

CTAGS?=
CTAGSFLAGS?=
DPADD+=lib-tags

all: all-subdir lib${LIB}.a lib${LIB}.la
install: install-lib install-subdir
deinstall: deinstall-lib deinstall-subdir
clean: clean-lib clean-subdir
cleandir: clean-lib clean-subdir cleandir-lib cleandir-subdir
regress: regress-subdir
depend: depend-subdir

.SUFFIXES: .o .po .lo .c .cc .cpp .asm .l .y .m

# Compile C code into an object file
.c.o:
	${CC} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
.c.lo: ${LIBTOOL}
	${LIBTOOL} --mode=compile ${CC} ${LIBTOOLFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile Objective-C code into an object file
.m.o:
	${CC} ${OBJCFLAGS} ${CPPFLAGS} -o $@ -c $<
.m.lo: ${LIBTOOL}
	${LIBTOOL} --mode=compile ${CC} ${LIBTOOLFLAGS} ${OBJCFLAGS} ${CPPFLAGS} -o $@ -c $<
.m.po:
	${CC} -pg -DPROF ${OBJCFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile C++ code into an object file
.cc.o:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cc.lo: ${LIBTOOL}
	${LIBTOOL} --mode=compile ${CXX} ${LIBTOOLFLAGS} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cc.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cpp.o:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cpp.lo: ${LIBTOOL}
	${LIBTOOL} --mode=compile ${CXX} ${LIBTOOLFLAGS} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cpp.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile assembly code into an object file
.asm.o:
	${ASM} ${ASMFLAGS} ${CPPFLAGS} -o $@ $<

# Compile a Lex lexer into an object file
.l:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} -o $@ $@.yy.c ${LIBL} ${LIBS}
	@rm -f $@.yy.c
.l.o:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	${CC} ${CFLAGS} ${CPPFLAGS} -o $@ -c $@.yy.c
	@mv -f $@.yy.o $@
	@rm -f $@.yy.c
.l.po:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $@.yy.c
	@mv -f $@.yy.o $@
	@rm -f $@.yy.c

# Compile a Yacc parser into an object file
.y:
	${YACC} ${YFLAGS} -b $@ $<
	${CC} ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} -o $@ $@.tab.c ${LIBS}
	@rm -f $@.tab.c
.y.o:
	${YACC} ${YFLAGS} -b $@ $<
	${CC} ${CFLAGS} ${CPPFLAGS} -o $@ -c $@.tab.c
	@mv -f $@.tab.o $@
	@rm -f $@.tab.c
.y.po:
	${YACC} ${YFLAGS} -b $@ $<
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $@.tab.c
	@mv -f $@.tab.o $@
	@rm -f $@.tab.c

# Build the library's object files.
_lib_objs:
	@if [ "${LIB}" != "" -a "${OBJS}" = "none" -a "${SRCS}" != "none" \
	      -a "${USE_LIBTOOL}" = "No" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	        F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	        ${MAKE} $$F; \
		if [ $$? != 0 ]; then \
			echo "${MAKE}: failure"; \
			exit 1; \
		fi; \
            done; \
	fi
	@if [ "${WINRES}" != "" -a "${WINDRES}" != "" ]; then \
		echo "${WINDRES} -o ${WINRES}.o ${WINRES}"; \
		${WINDRES} -o ${WINRES}.o ${WINRES}; \
	fi

# Build PIC versions of the library's object files.
_lib_shobjs:
	@if [ "${LIB}" != "" -a "${SHOBJS}" = "none" -a "${SRCS}" != "none" \
	      -a "${USE_LIBTOOL}" = "Yes" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[clym]$$/.lo/'`; \
	        F=`echo $$F | sed 's/.cc$$/.lo/'`; \
	        F=`echo $$F | sed 's/.cpp$$/.lo/'`; \
	        F=`echo $$F | sed 's/.asm$$/.lo/'`; \
	        ${MAKE} $$F; \
		if [ $$? != 0 ]; then \
			echo "${MAKE}: failure"; \
			exit 1; \
		fi; \
            done; \
	fi

# Build a non-libtool version of the library.
lib${LIB}.a: _lib_objs ${OBJS}
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "No" \
	      -a "${SRCS}" != "none" ]; then \
	    if [ "${OBJS}" = "none" ]; then \
	        export _objs=""; \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    _objs="$$_objs $$F"; \
                done; \
	        echo "${AR} -cru lib${LIB}.a $$_objs ${LIB_XOBJS}"; \
	        ${AR} -cru lib${LIB}.a $$_objs ${LIB_XOBJS}; \
	    else \
	        echo "${AR} -cru lib${LIB}.a ${OBJS} ${LIB_XOBJS}"; \
	        ${AR} -cru lib${LIB}.a ${OBJS} ${LIB_XOBJS}; \
	    fi; \
	    echo "${RANLIB} lib${LIB}.a"; \
	    (${RANLIB} lib${LIB}.a || exit 0); \
	fi

# Build a Libtool version of the library.
_lib_shobjs ${SHOBJS}: ${LIBTOOL_COOKIE}

lib${LIB}.la: _lib_shobjs ${SHOBJS}
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "Yes" \
	      -a "${SRCS}" != "none" ]; then \
	    if [ "${SHOBJS}" = "none" ]; then \
	        export _shobjs=""; \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[clym]$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.lo/'`; \
	    	    _shobjs="$$_shobjs $$F"; \
                done; \
	    	if [ "${LIB_SHARED}" = "Yes" ]; then \
	            echo "${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -rpath ${PREFIX}/lib \
	                -version-info ${LIB_MAJOR}:${LIB_MINOR}:0 \
		        ${LDFLAGS} $$_shobjs \
		        ${LIBS} ${LIB_XOBJS}"; \
	            ${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -rpath ${PREFIX}/lib \
		        -version-info ${LIB_MAJOR}:${LIB_MINOR}:0 \
		        ${LDFLAGS} $$_shobjs \
			${LIBS} ${LIB_XOBJS}; \
		else \
	            echo "${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
	                -static \
		        ${LDFLAGS} $$_shobjs \
		        ${LIBS} ${LIB_XOBJS}"; \
	            ${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -static \
		        ${LDFLAGS} $$_shobjs \
			${LIBS} ${LIB_XOBJS}; \
		fi; \
	    else \
	    	if [ "${LIB_SHARED}" = "Yes" ]; then \
	            echo "${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -rpath ${PREFIX}/lib \
	                -version-info ${LIB_MAJOR}:${LIB_MINOR}:0 \
		        ${LDFLAGS} ${SHOBJS} \
		        ${LIBS} ${LIB_XOBJS}"; \
	            ${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -rpath ${PREFIX}/lib \
		        -version-info ${LIB_MAJOR}:${LIB_MINOR}:0 \
		        ${LDFLAGS} ${SHOBJS} \
			${LIBS} ${LIB_XOBJS}; \
	        else \
	            echo "${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -static \
		        ${LDFLAGS} ${SHOBJS} \
		        ${LIBS} ${LIB_XOBJS}"; \
	            ${LIBTOOL} --mode=link ${CC} -o lib${LIB}.la \
		        -static \
		        ${LDFLAGS} ${SHOBJS} \
			${LIBS} ${LIB_XOBJS}; \
		fi; \
	    fi; \
	fi

clean-lib:
	@if [ "${LIB}" != "" -a "${SRCS}" != "none" ]; then \
	    if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	        if [ "${SHOBJS}" = "none" ]; then \
		    export _objs=""; \
                    for F in ${SRCS}; do \
	    	        F=`echo $$F | sed 's/.[clym]$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.cc$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.cpp$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.asm$$/.lo/'`; \
			_objs="$$_objs $$F"; \
                    done; \
	    	    echo "rm -f $$_objs"; \
	    	    rm -f $$_objs; \
		    export _objs=""; \
                    for F in ${SRCS}; do \
	    	        F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
			_objs="$$_objs $$F"; \
                    done; \
	    	    echo "rm -f $$_objs"; \
	    	    rm -f $$_objs; \
		else \
		    rm -f ${SHOBJS}; \
		    echo "rm -f ${SHOBJS}"; \
		    export _objs=""; \
                    for F in ${SHOBJS}; do \
	    	        F=`echo $$F | sed 's/.lo$$/.o/'`; \
			_objs="$$_objs $$F"; \
                    done; \
	    	    echo "rm -f $$_objs"; \
	    	    rm -f $$_objs; \
		fi; \
		echo "rm -fR lib${LIB}.la .libs"; \
		rm -fR lib${LIB}.la .libs; \
	    else \
	        if [ "${OBJS}" = "none" ]; then \
		    export _objs=""; \
                    for F in ${SRCS}; do \
	   	        F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	    	F=`echo $$F | sed 's/.asm$$/.o/'`; \
			_objs="$$_objs $$F"; \
                    done; \
	    	    echo "rm -f $$_objs"; \
	    	    rm -f $$_objs; \
	    	else \
	            echo "rm -f ${OBJS}"; \
		    rm -f ${OBJS}; \
	    	fi; \
	   	echo "rm -f lib${LIB}.a"; \
		rm -f lib${LIB}.a; \
	    fi; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi
	@if [ -e ".depend" ]; then \
		echo "echo >.depend"; \
		echo >.depend; \
	fi

cleandir-lib:
	rm -f ${LIBTOOL} ${LIBTOOL_COOKIE} ${LTCONFIG_LOG} config.log tags
	if [ -e "./config/prefix.h" ]; then rm -fr ./config; fi
	if [ -e "Makefile.config" ]; then echo >Makefile.config; fi

install-lib: ${LIBTOOL_COOKIE}
	@if [ "${INCL}" != "none" -a "${INCL}" != "none" ]; then \
	    if [ ! -d "${INCLDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${INCLDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${INCLDIR}; \
	    fi; \
	    for F in ${INCL}; do \
	        echo "${INSTALL_DATA} $$F ${INCLDIR}"; \
	        ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${INCLDIR}; \
	    done; \
	fi
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "Yes" -a \
	      "${LIB_INSTALL}" = "Yes" ]; then \
	    if [ ! -d "${LIBDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${LIBDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${LIBDIR}; \
	    fi; \
	    if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	        echo "${LIBTOOL} --mode=install \
	            ${INSTALL_LIB} lib${LIB}.la ${LIBDIR}"; \
	        ${SUDO} ${LIBTOOL} --mode=install \
	            ${INSTALL_LIB} lib${LIB}.la ${DESTDIR}${LIBDIR}; \
	        echo "${LIBTOOL} --finish ${LIBDIR}"; \
	        ${SUDO} ${LIBTOOL} --finish ${DESTDIR}${LIBDIR}; \
	    else \
	        echo "${INSTALL_LIB} lib${LIB}.a ${LIBDIR}"; \
	        ${SUDO} ${INSTALL_LIB} lib${LIB}.a ${DESTDIR}${LIBDIR}; \
	    fi; \
	fi
	@if [ "${SHARE}" != "none" ]; then \
            if [ ! -d "${SHAREDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${SHAREDIR}; \
            fi; \
            for F in ${SHARE}; do \
                echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${SHAREDIR}; \
            done; \
	fi
	@if [ "${SHARESRC}" != "none" ]; then \
            if [ ! -d "${SHAREDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${SHAREDIR}; \
            fi; \
	    if [ "${SRC}" != "" ]; then \
                for F in ${SHARESRC}; do \
                    echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                    ${SUDO} ${INSTALL_DATA} ${SRC}/${BUILDREL}/$$F \
		    ${DESTDIR}${SHAREDIR}; \
                done; \
	    else \
                for F in ${SHARESRC}; do \
                    echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                    ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${SHAREDIR}; \
                done; \
	    fi; \
	fi
	@if [ "${CONF}" != "none" ]; then \
            if [ ! -d "${SYSCONFDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SYSCONFDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${SYSCONFDIR}; \
            fi; \
	    if [ "${CONF_OVERWRITE}" != "Yes" ]; then \
	        echo "+----------------"; \
	        echo "| The following configuration files exist and "; \
	        echo "| will not be overwritten:"; \
	        echo "|"; \
	        for F in ${CONF}; do \
	            if [ -e "${DESTDIR}${SYSCONFDIR}/$$F" ]; then \
	                echo "| - $$F"; \
	            else \
	                ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${SYSCONFDIR}; \
	            fi; \
	        done; \
	        echo "+----------------"; \
	    else \
	        for F in ${CONF}; do \
	            echo "${INSTALL_DATA} $$F ${SYSCONFDIR}"; \
	            ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${SYSCONFDIR}; \
	        done; \
	    fi; \
	fi

deinstall-lib: ${LIBTOOL_COOKIE}
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "Yes" ]; then \
	    if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	        echo "${LIBTOOL} --mode=uninstall \
	            rm -f ${LIBDIR}/lib${LIB}.la"; \
	        ${SUDO} ${LIBTOOL} --mode=uninstall \
	            rm -f ${DESTDIR}${LIBDIR}/lib${LIB}.la; \
	    else \
	        echo "${DEINSTALL_LIB} ${LIBDIR}/lib${LIB}.a"; \
	        ${SUDO} ${DEINSTALL_LIB} ${DESTDIR}${LIBDIR}/lib${LIB}.a; \
	    fi; \
	fi
	@if [ "${SHARE}" != "none" ]; then \
	    for F in ${SHARE}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${SHAREDIR}/$$F; \
	    done; \
	fi
	@if [ "${SHARESRC}" != "none" ]; then \
	    for F in ${SHARESRC}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${SHAREDIR}/$$F; \
	    done; \
	fi
	@if [ "${CONF}" != "none" ]; then \
	    echo "+----------------"; \
	    echo "| To completely deinstall lib${LIB} you need to perform."; \
	    echo "| the following steps as root:"; \
	    echo "|"; \
	    for F in ${CONF}; do \
	        if [ -e "${DESTDIR}${SYSCONFDIR}/$$F" ]; then \
	            echo "| rm -f $$F"; \
	        fi; \
	    done; \
	    echo "|"; \
	    echo "| Do not do this if you plan on re-installing lib${LIB}"; \
	    echo "| at some future time."; \
	    echo "+----------------"; \
	fi

includes:
	(cd ${TOP} && ${MAKE} install-includes)

${LIBTOOL_COOKIE}: ${LTCONFIG} ${LTMAIN_SH} ${LTCONFIG_GUESS} ${LTCONFIG_SUB}
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "Yes" ]; then \
	    echo "${SH} ${LTCONFIG} ${LTMAIN_SH} ${HOST}"; \
	    env CC="${CC}" CXX="${CXX}" \
	        CFLAGS="${CFLAGS}" CXXFLAGS="${CXXFLAGS}" \
		${SH} ${LTCONFIG} ${LTMAIN_SH} ${HOST}; \
	    if [ $? != 0 ]; then \
	    	echo "${LTCONFIG} failed"; \
	    	exit 1; \
	    fi; \
	    if [ ! -f "${LIBTOOL}" ]; then \
		echo "mv libtool ${LIBTOOL}"; \
		mv libtool ${LIBTOOL}; \
	    fi; \
	    echo "${LIBTOOL}" > ${LIBTOOL_COOKIE}; \
	fi

none:

lib-tags:
	-@if [ "${CTAGS}" != "" ]; then \
	    if [ "${SRC}" != "" ]; then \
	        (cd ${SRC}; \
		 echo "${CTAGS} ${CTAGSFLAGS} -R"; \
	         ${CTAGS} ${CTAGSFLAGS} -R); \
	    else \
	        echo "${CTAGS} ${CTAGSFLAGS} -R"; \
	        ${CTAGS} ${CTAGSFLAGS} -R; \
	    fi; \
	fi

${LTCONFIG} ${LTCONFIG_GUESS} ${LTCONFIG_SUB} ${LTMAIN_SH}:

.PHONY: install deinstall includes clean cleandir regress depend
.PHONY: install-lib deinstall-lib clean-lib cleandir-lib
.PHONY: _lib_objs _lib_shobjs lib-tags none

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.dep.mk
include ${TOP}/mk/build.proj.mk
include ${TOP}/mk/build.subdir.mk
include .depend
