#
# Copyright (c) 2001-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
# Compile executables from source.
#

PROG?=
GMONOUT?=	gmon.out
WINRES?=

CC?=		cc
ASM?=		nasm
LEX?=		lex
YACC?=		yacc
WINDRES?=

CFLAGS?=	-O2 -g
CPPFLAGS?=
CXXFLAGS?=
OBJCFLAGS?=	${CFLAGS}
ASMFLAGS?=	-g -w-orphan-labels
LFLAGS?=
LIBL?=		-ll
YFLAGS?=	-d

PROG_INSTALL?=	Yes
PROG_TYPE?=	"CLI"
PROG_GUID?=

SHARE?=none
SHARESRC?=none
SRCS?=none
OBJS?=none
POBJS?=none
SHOBJS?=none
CONF?=none
CONF_OVERWRITE?=No
CLEANFILES?=

CTAGS?=
CTAGSFLAGS?=
DPADD+=prog-tags

all: all-subdir ${PROG}
install: install-prog install-subdir
deinstall: deinstall-prog deinstall-subdir
clean: clean-prog clean-subdir
cleandir: clean-prog clean-subdir cleandir-prog cleandir-subdir
regress: regress-subdir
depend: depend-subdir

.SUFFIXES: .o .po .c .cc .cpp .asm .l .y .m

# Compile C code into an object file
.c.o:
	${CC} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile C++ code into an object file
.cc.o:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cc.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cpp.o:
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<
.cpp.po:
	${CXX} -pg -DPROF ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile C+Objective-C code into an object file
.m.o:
	${CC} ${OBJCFLAGS} ${CPPFLAGS} -o $@ -c $<
.m.po:
	${CC} -pg -DPROF ${OBJCFLAGS} ${CPPFLAGS} -o $@ -c $<

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

# Build the program's object files
_prog_objs:
	@if [ "${PROG}" != "" -a "${OBJS}" = "none" \
	      -a "${SRCS}" != "none" ]; then \
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

# Build profiled versions of the program's object files
_prog_pobjs:
	@if [ "${GMONOUT}" != "" -a "${POBJS}" = "none" \
	      -a "${SRCS}" != "none" ]; then \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.[clym]$$/.po/'`; \
	        F=`echo $$F | sed 's/.cc$$/.po/'`; \
	        F=`echo $$F | sed 's/.cpp$$/.po/'`; \
	        F=`echo $$F | sed 's/.asm$$/.po/'`; \
	        ${MAKE} $$F; \
		if [ $$? != 0 ]; then \
			echo "${MAKE}: failure"; \
			exit 1; \
		fi; \
	    done; \
	fi

# Compile and link the program
${PROG}: _prog_objs ${OBJS}
	@if [ "${PROG}" != "" -a "${SRCS}" != "none" ]; then \
	    if [ "${OBJS}" = "none" ]; then \
	        export _objs=""; \
                for F in ${SRCS}; do \
	            F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    _objs="$$_objs $$F"; \
                done; \
		if [ "${WINRES}" != "" ]; then \
	            echo "${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} $$_objs ${LIBS} \
		        ${WINRES}.o"; \
	            ${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} $$_objs ${LIBS} \
		        ${WINRES}.o; \
		else \
	            echo "${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} $$_objs ${LIBS}"; \
	            ${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} $$_objs ${LIBS}; \
		fi; \
	    else \
		if [ "${WINRES}" != "" ]; then \
	            echo "${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS} \
		        ${WINRES}.o"; \
	            ${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS} \
		        ${WINRES}.o; \
		else \
	            echo "${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}"; \
	            ${CC} ${CFLAGS} ${LDFLAGS} -o ${PROG} ${OBJS} ${LIBS}; \
		fi; \
	    fi; \
	fi

# Compile and link a profiled version of the program
${GMONOUT}: _prog_pobjs ${POBJS}
	@if [ "${GMONOUT}" != "" -a "${SRCS}" != "none" ]; then \
	    if [ "${POBJS}" = "none" ]; then \
	        export _pobjs=""; \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[clym]$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.po/'`; \
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

clean-prog:
	@if [ "${PROG}" != "" -a "${SRCS}" != "none" ]; then \
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
	    else \
		export _objs=""; \
                for F in ${SHOBJS}; do \
	    	    F=`echo $$F | sed 's/.lo$$/.o/'`; \
		    _objs="$$_objs $$F"; \
                done; \
	        echo "rm -f $$_objs ${SHOBJS}"; \
	        rm -f $$_objs ${SHOBJS}; \
	    fi; \
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
	    if [ "${POBJS}" = "none" ]; then \
                export _objs=""; \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[clym]$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.po/'`; \
		    _objs="$$_objs $$F"; \
                done; \
	    	echo "rm -f $$_objs"; \
	    	rm -f $$_objs; \
	    else \
	        echo "rm -f ${POBJS}"; \
	        rm -f ${OBJS}; \
	    fi; \
	    echo "rm -f ${PROG}${EXECSUFFIX} ${GMONOUT} ${WINRES}.o"; \
	    rm -f ${PROG}${EXECSUFFIX} ${GMONOUT} ${WINRES}.o; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi
	@if [ -e ".depend" ]; then \
		echo "echo >.depend"; \
		echo >.depend; \
	fi

cleandir-prog:
	rm -f *.core config.log configure.lua tags
	if [ -e "./config/prefix.h" ]; then rm -fr ./config; fi
	if [ -e "Makefile.config" ]; then echo >Makefile.config; fi
	if [ -e ".depend" ]; then echo >.depend; fi

install-prog:
	@if [ ! -e "${BINDIR}" ]; then \
	    echo "${INSTALL_PROG_DIR} ${BINDIR}"; \
	    ${SUDO} ${INSTALL_PROG_DIR} ${DESTDIR}${BINDIR}; \
	fi
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${INSTALL_PROG} ${PROG}${EXECSUFFIX} ${BINDIR}"; \
	    ${SUDO} ${INSTALL_PROG} ${PROG}${EXECSUFFIX} ${DESTDIR}${BINDIR}; \
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

deinstall-prog:
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${DEINSTALL_PROG} ${BINDIR}/${PROG}${EXECSUFFIX}"; \
	    ${SUDO} ${DEINSTALL_PROG} ${DESTDIR}${BINDIR}/${PROG}${EXECSUFFIX}; \
	fi
	@if [ "${SHARE}" != "none" ]; then \
	    for F in ${SHARE}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${SHAREDIR}/$$F; \
	    done; \
	fi
	@if [ "${CONF}" != "none" ]; then \
	    echo "+----------------"; \
	    echo "| To completely deinstall ${PROG} you need to perform."; \
	    echo "| the following steps as root:"; \
	    echo "|"; \
	    for F in ${CONF}; do \
	        if [ -e "${DESTDIR}${SYSCONFDIR}/$$F" ]; then \
	            echo "| rm -f $$F"; \
	        fi; \
	    done; \
	    echo "|"; \
	    echo "| Do not do this if you plan on re-installing ${PROG}"; \
	    echo "| at some future time."; \
	    echo "+----------------"; \
	fi

none:

prog-tags:
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

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-prog deinstall-prog clean-prog cleandir-prog
.PHONY: _prog_objs _prog_pobjs prog-tags none

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.dep.mk
include ${TOP}/mk/build.proj.mk
include ${TOP}/mk/build.subdir.mk
include .depend
