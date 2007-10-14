#
# Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
CONFDIR?=

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
	if [ "${WINRES}" != "" -a "${WINDRES}" != "" ]; then \
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
	if [ "${GMONOUT}" != "" -a "${SRCS}" != "none" ]; then \
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
	    if [ "${OBJS}" = "none" ]; then \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    echo "rm -f $$F"; \
	    	    rm -f $$F; \
                done; \
	    else \
	        echo "rm -f ${OBJS}"; \
	        rm -f ${OBJS}; \
	    fi; \
	    if [ "${POBJS}" = "none" ]; then \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.[clym]$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.po/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.po/'`; \
	    	    echo "rm -f $$F"; \
	    	    rm -f $$F; \
                done; \
	    else \
	        echo "rm -f ${POBJS}"; \
	        rm -f ${POBJS}; \
	    fi; \
	    echo "rm -f ${PROG} ${GMONOUT} ${WINRES}.o"; \
	    rm -f ${PROG} ${GMONOUT} ${WINRES}.o; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi

cleandir-prog:
	rm -f core *.core config.log .depend
	if [ -e "./config/prefix.h" ]; then rm -fr ./config; fi
	if [ -e "Makefile.config" ]; then echo -n >Makefile.config; fi

install-prog:
	@if [ ! -e "${BINDIR}" ]; then \
	    echo "${INSTALL_PROG_DIR} ${BINDIR}"; \
	    ${SUDO} ${INSTALL_PROG_DIR} ${BINDIR}; \
	fi
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${INSTALL_PROG} ${PROG} ${BINDIR}"; \
	    ${SUDO} ${INSTALL_PROG} ${PROG} ${BINDIR}; \
	fi
	@if [ "${SHARE}" != "none" ]; then \
            if [ ! -d "${SHAREDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${SHAREDIR}; \
            fi; \
            for F in ${SHARE}; do \
                echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA} $$F ${SHAREDIR}; \
            done; \
	fi
	@if [ "${SHARESRC}" != "none" ]; then \
            if [ ! -d "${SHAREDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${SHAREDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${SHAREDIR}; \
            fi; \
	    if [ "${SRC}" != "" ]; then \
                for F in ${SHARESRC}; do \
                    echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                    ${SUDO} ${INSTALL_DATA} ${SRC}/${BUILDREL}/$$F \
		    ${SHAREDIR}; \
                done; \
	    else \
                for F in ${SHARESRC}; do \
                    echo "${INSTALL_DATA} $$F ${SHAREDIR}"; \
                    ${SUDO} ${INSTALL_DATA} $$F ${SHAREDIR}; \
                done; \
	    fi; \
	fi
	@if [ "${CONF}" != "none" ]; then \
            if [ ! -d "${CONFDIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${CONFDIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${CONFDIR}; \
            fi; \
	    echo "+----------------"; \
	    echo "| The following configuration files have been preserved."; \
	    echo "| You may want to compare them to the current sample files.";\
	    echo "|"; \
            for F in ${CONF}; do \
	        if [ -e "${CONFDIR}/$$F" ]; then \
          	      echo "| - $$F"; \
		else \
         	       ${SUDO} ${INSTALL_DATA} $$F ${CONFDIR}; \
		fi; \
            done; \
	    echo "+----------------"; \
	fi

deinstall-prog:
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    echo "${DEINSTALL_PROG} ${BINDIR}/${PROG}"; \
	    ${SUDO} ${DEINSTALL_PROG} ${BINDIR}/${PROG}; \
	fi
	@if [ "${SHARE}" != "none" ]; then \
	    for F in ${SHARE}; do \
	        echo "${DEINSTALL_DATA} ${SHAREDIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${SHAREDIR}/$$F; \
	    done; \
	fi
	@if [ "${CONF}" != "none" ]; then \
	    echo "+----------------"; \
	    echo "| To completely deinstall ${PROG} you need to perform."; \
	    echo "| this step as root:"; \
	    echo "|"; \
	    echo "|           rm -fR ${CONFDIR}"; \
	    echo "|"; \
	    echo "| Do not do this if you plan on re-installing ${PROG}"; \
	    echo "| at some future time."; \
	    echo "+----------------"; \
	fi

none:

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-prog deinstall-prog clean-prog cleandir-prog
.PHONY: _prog_objs _prog_pobjs none

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.dep.mk
include ${TOP}/mk/build.proj.mk
include ${TOP}/mk/build.subdir.mk
