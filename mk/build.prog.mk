#
# Copyright (c) 2001-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
PROG_INSTALL?=	Yes
PROG_PROFILE?=	No
PROG_TYPE?=	"CLI"
PROG_GUID?=
PROG_GUI_FLAGS?=
PROG_CLI_FLAGS?=
PROG_PREFIX?=
PROG_SUFFIX?=
PROG_TRANSFORM?=s,x,x,

ADA?=		ada
ADABIND?=	gnatbind
ADALINK?=	gnatlink
ASM?=		nasm
CC?=		cc
CC_COMPILE?=	-c
CXX?=		c++
LEX?=		lex
LN?=		ln
OBJC?=		cc
SH?=		sh
WINDRES?=
YACC?=		yacc

ADAFLAGS?=
ADABFLAGS?=	-x
ASMFLAGS?=	-g -w-orphan-labels
CFLAGS?=	-O -g
CPPFLAGS?=
CXXFLAGS?=
LFLAGS?=
LIBL?=		-ll
MKDEP=		sh ${TOP}/mk/mkdep
MKDEP_ADA?=	gnatmake
MKDEP_ADAFLAGS?=-M -u -v
OBJCFLAGS?=
YFLAGS?=	-d

CONF?=
CONF_OVERWRITE?=No
CONFIGSCRIPTS?=
CLEANFILES?=
CLEANDIRFILES?=
CTAGS?=
CTAGSFLAGS?=
DATAFILES?=
DATAFILES_SRC?=
LINKER_TYPE?=
OBJS?=
PCMODULES?=
SHOBJS?=
SRCS?=
SRCS_GENERATED?=
WINRES?=

PROG_BUNDLE?=
PROG_SIGNATURE?=	# sign
PROG_DISPLAY_NAME?=	# "${PROG}"
PROG_IDENTIFIER?=	# com.MYNAME.${PROG}
PROG_VERSION?=		# 1.0
PROG_COPYRIGHT?=	# "Copyright (c) 2018 MYNAME"
PROG_CATEGORY?=		# public.app-category.utilities, or
# business developer-tools education entertainment finance games graphics-design
# healthcare-fitness lifestyle medical music news photography productivity
# reference social-networking sports travel utilities video weather
# action-games adventure-games arcade-games board-games card-games casino-games
# dice-games educational-games family-games kids-games music-games puzzle-games
# racing-games role-playing-games simulation-games sports-games strategy-games
# trivial-games word-games

PROG_PRINCIPAL_CLASS?=	AG_AppDelegate
PROG_OSX_VERSION?=	10.3.0
PROG_INFO_EXTRA?=
PROG_REQUIRED_CAPABILITIES?=
# accelerometer armv6 armv7 arm64 auto-focus-camera bluetooth-le
# camera-flash front-facing-camera gamekit gps gyroscope healthkit
# location-services magnetometer metal microphone opengles-1 opengles-2
# opengles-3 peer-peer sms still-camera telephony video-camera wifi

all: all-subdir ${PROG}
install: install-prog install-subdir
deinstall: deinstall-prog deinstall-subdir
clean: clean-prog clean-subdir
cleandir: clean-prog clean-subdir cleandir-prog cleandir-subdir
regress: regress-subdir
configure: configure-prog

.SUFFIXES: .adb .ads .asm .c .cc .cpp .l .m .o .y

# Compile Ada code into an object file
.adb.o .ads.o:
	${ADA} ${ADAFLAGS} ${CFLAGS} -c $<

# Compile assembly code into an object file
.asm.o:
	${ASM} ${ASMFLAGS} ${CPPFLAGS} -o $@ $<

# Compile C code into an object file
.c.o:
	@_cflags=""; _out="$@"; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _cflags="-pg -DPROF"; fi; \
	if [ "${HAVE_CC65}" = "yes" ]; then _out=`echo "$@" | sed 's/.o$$/.s/'`; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $$_out ${CC_COMPILE} $<"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $$_out ${CC_COMPILE} $<; \
	if [ $$? != 0 ]; then \
		echo "*"; \
		echo "* $$_out compilation failed."; \
		echo "*"; \
		exit 1; \
	fi; \
	if [ "${HAVE_CC65}" = "yes" ]; then \
		echo "ca65 -o $@ $$_out"; \
		ca65 -o $@ $$_out; \
	fi

# Compile C++ code into an object file
.cc.o .cpp.o:
	@_cxxflags=""; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _cxxflags="-pg -DPROF"; fi; \
	echo "${CXX} ${CXXFLAGS} $$_cxxflags ${CPPFLAGS} -o $@ -c $<"; \
	${CXX} ${CXXFLAGS} $$_cxxflags ${CPPFLAGS} -o $@ -c $<

# Compile Objective-C code into an object file
.m.o:
	@_objcflags=""; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _objcflags="-pg -DPROF"; fi; \
	echo "${OBJC} ${CFLAGS} ${OBJCFLAGS} $$_objcflags ${CPPFLAGS} -o $@ -c $<"; \
	${OBJC} ${CFLAGS} ${OBJCFLAGS} $$_objcflags ${CPPFLAGS} -o $@ -c $<

# Compile a Lex lexer into an object file
.l:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	@_cflags=""; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _cflags="-pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ $@.yy.c ${LIBL} ${LIBS}"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ $@.yy.c ${LIBL} ${LIBS}
	@rm -f $@.yy.c
.l.o:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	@_cflags=""; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _cflags="-pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.yy.c"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.yy.c
	@mv -f $@.yy.o $@
	@rm -f $@.yy.c

# Compile a Yacc parser into an object file
.y:
	${YACC} ${YFLAGS} -b $@ $<
	@_cflags=""; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _cflags="-pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ $@.tab.c ${LIBS}"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ $@.tab.c ${LIBS}
	@rm -f $@.tab.c
.y.o:
	${YACC} ${YFLAGS} -b $@ $<
	@_cflags=""; \
	if [ "${PROG_PROFILE}" = "Yes" ]; then _cflags="-pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.tab.c"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.tab.c
	@mv -f $@.tab.o $@
	@rm -f $@.tab.c

# Generate or update make dependencies
depend:	prog-tags depend-subdir
	@echo > .depend
	@_srcs="${SRCS}"; \
	if [ "$$_srcs" != "" -a "$$_srcs" != "none" ]; then \
	    _srcs_ada=""; \
	    _srcs_c=""; \
            for F in $$_srcs; do \
	        if echo $$F | grep -q '.ad[bs]$$'; then \
		    FB=`echo "$$F" | sed 's/.ad[bs]$$//'`; \
		    if [ ! -e "$$FB.o" ]; then \
		        echo "${MAKE} $$FB.o"; \
		        ${MAKE} $$FB.o; \
			if [ $$? != 0 ]; then \
			    echo "${MAKE} $$FB.o failed"; \
			    exit 1; \
			fi; \
	            fi; \
		    _srcs_ada="$$_srcs_ada $$F"; \
	        else \
		    _srcs_c="$$_srcs_c $$F"; \
		fi; \
	    done; \
	    if [ "${BUILD}" != "" ]; then \
	        export _mkdep_cflags="${CFLAGS} -I${BUILD}"; \
	    else \
	        export _mkdep_cflags="${CFLAGS}"; \
	    fi; \
	    if [ "$$_srcs_c" != "" ]; then \
	        echo "${MKDEP} $$_mkdep_cflags $$_srcs_c"; \
	        env CC=${CC} ${MKDEP} $$_mkdep_cflags $$_srcs_c; \
	        if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	            echo "${MKDEP} -a -l $$_mkdep_cflags $$_srcs_c"; \
	            env CC=${CC} ${MKDEP} -a -l $$_mkdep_cflags $$_srcs_c; \
	        fi; \
	    fi; \
	    if [ "$$_srcs_ada" != "" ]; then \
	        echo "${MKDEP_ADA} ${MKDEP_ADAFLAGS} ${CFLAGS} $$_srcs_ada >>.depend"; \
	        env ADA=${ADA} ${MKDEP_ADA} ${MKDEP_ADAFLAGS} ${CFLAGS} $$_srcs_ada 1>.ada_depend 2>.ada_errors; \
		if [ $$? != 0 ]; then \
		    echo "${MKDEP_ADA} failed"; \
		    cat .ada_errors; rm -f .ada_errors; \
		    exit 1; \
		fi; \
		if grep -q "must be recompiled" .ada_errors; then \
		    echo "${MKDEP_ADA} failed:"; \
		    cat .ada_errors; rm -f .ada_errors; \
		    exit 1; \
		fi; \
		cat .ada_depend >> .depend; \
		rm -f .ada_depend .ada_errors; \
	    fi; \
	fi

# Build the program's object files
_prog_objs:
	@if [ "${PROG}" != "" -a "${OBJS}" = "" -a "${SRCS}" != "" ]; then \
	    FLIST=""; \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	        F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	        F=`echo $$F | sed 's/.cpp$$/.o/'`; \
		FLIST="$$FLIST $$F"; \
	    done; \
	    ${MAKE} $$FLIST; \
	    _make_result="$$?"; \
	    if [ $$_make_result != "0" ]; then \
	        echo "*"; \
	        echo "* Failed to make ${PROG} (${MAKE} returned $$_make_result)"; \
	        echo "*"; \
		exit 1; \
	    fi; \
	fi
	@if [ "${WINRES}" != "" -a "${WINDRES}" != "" ]; then \
	    echo "${WINDRES} -o ${WINRES}.o ${WINRES}"; \
	    ${WINDRES} -o ${WINRES}.o ${WINRES}; \
	fi

# Compile and link the program
${PROG}: ${SRCS_GENERATED} _prog_objs ${OBJS}
	@if [ "${PROG}" != "" -a "${SRCS}" != "" -a "${.TARGETS}" != "install" ]; then \
	    if [ "${PROG_TYPE}" = "GUI" ]; then \
	    	_prog_ldflags="${PROG_GUI_FLAGS}"; \
	    else \
	    	_prog_ldflags="${PROG_CLI_FLAGS}"; \
	    fi; \
	    _linker_type="${LINKER_TYPE}"; \
	    if [ "$$_linker_type" = "" ]; then \
	    	if [ "${HAVE_CC65}" = "yes" ]; then \
		    _linker_type="CL65"; \
		else \
                    for F in ${SRCS}; do \
	                if echo "$$F" | grep -q '.ad[bs]$$'; then \
		            _linker_type="ADA"; \
	                    break; \
			fi; \
	            done; \
	        fi; \
	    fi; \
	    _objs="${OBJS}"; \
	    if [ "${OBJS}" = "" ]; then \
                for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	            F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	            if [ "$$F" = "${PROG}.o" ]; then \
		    	if [ "$$_linker_type" != "ADA" ]; then \
	    	            _objs="$$_objs $$F"; \
			fi; \
		    else \
	    	        _objs="$$_objs $$F"; \
		    fi; \
                done; \
	    fi; \
	    if [ "${WINRES}" != "" ]; then \
	        _objs="$$_objs ${WINRES}.o"; \
	    fi; \
	    \
	    case "$$_linker_type" in \
	    ADA) \
	        _ada_cflags=""; \
	        for F in ${CFLAGS}; do \
	            case "$$F" in \
	            -I*) \
	                _ada_cflags="$$_ada_cflags $$F"; \
	                ;; \
	            esac; \
	        done; \
	        echo "${ADABIND} ${ADABFLAGS} $$_ada_cflags ${PROG}"; \
	        ${ADABIND} ${ADABFLAGS} $$_ada_cflags ${PROG}; \
	        echo "${ADALINK} ${LDFLAGS} ${ADALFLAGS} $$_ada_cflags $$_prog_ldflags ${PROG} ${LIBS}"; \
	        ${ADALINK} ${LDFLAGS} ${ADALFLAGS} $$_ada_cflags $$_prog_ldflags ${PROG} ${LIBS}; \
		;; \
	    CL65) \
	        echo "cl65 ${LDFLAGS} $$_prog_ldflags -Ln ${PROG}.lbl -m ${PROG}.map -o ${PROG} $$_objs ${LIBS}"; \
	        cl65 ${LDFLAGS} $$_prog_ldflags -Ln ${PROG}.lbl -m ${PROG}.map -o ${PROG} $$_objs ${LIBS}; \
	        ;; \
	    *) \
	        echo "${CC} ${CFLAGS} ${LDFLAGS} $$_prog_ldflags -o ${PROG} $$_objs ${LIBS}"; \
	        ${CC} ${CFLAGS} ${LDFLAGS} $$_prog_ldflags -o ${PROG} $$_objs ${LIBS}; \
		;; \
	    esac; \
	    if [ "${PROG_BUNDLE}" != "" ]; then \
		echo "perl ${TOP}/mk/gen-bundle.pl prog ${PROG_BUNDLE}"; \
		env PROG=${PROG} INSTALL_PROG="${INSTALL_PROG}" \
		PROG_SIGNATURE=${PROG_SIGNATURE} \
		PROG_DISPLAY_NAME=${PROG_DISPLAY_NAME} \
		PROG_IDENTIFIER=${PROG_IDENTIFIER} \
		PROG_OSX_VERSION=${PROG_OSX_VERSION} \
		PROG_PRINCIPAL_CLASS=${PROG_PRINCIPAL_CLASS} \
		PROG_COPYRIGHT=${PROG_COPYRIGHT} \
		PROG_VERSION=${PROG_VERSION} \
		PROG_CATEGORY=${PROG_CATEGORY} \
		PROG_REQUIRED_CAPABILITIES=${PROG_REQUIRED_CAPABILITIES} \
		PROG_INFO_EXTRA=${PROG_INFO_EXTRA} \
		perl ${TOP}/mk/gen-bundle.pl prog ${PROG_BUNDLE}; \
	    fi; \
	fi

clean-prog:
	@if [ "${PROG}" != "" -a "${SRCS}" != "" ]; then \
	    _objs="${OBJS}"; \
	    if [ "${OBJS}" = "" ]; then \
                for F in ${SRCS}; do \
		    if echo $$F | grep -q '.ad[bs]$$'; then \
		        FB=`echo "$$F" | sed 's/.ad[bs]$$//'`; \
	                _objs="$$_objs $$FB.ali"; \
	            fi; \
	    	    F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
		    _objs="$$_objs $$F"; \
                done; \
	    fi; \
	    if [ "$$_objs" != "" ]; then \
	        echo "rm -f $$_objs"; \
	        rm -f $$_objs; \
	    fi; \
	    if [ "${WINRES}" != "" ]; then \
	        echo "rm -f ${WINRES}.o"; \
	        rm -f ${WINRES}.o; \
	    fi; \
	    echo "rm -f ${PROG}${EXECSUFFIX}"; \
	    rm -f ${PROG}${EXECSUFFIX}; \
	fi
	@if [ "${CLEANFILES}" != "" ]; then \
	    _cleanfiles=""; \
	    for F in ${CLEANFILES}; do \
	        if [ -e $$F ]; then _cleanfiles="$$_cleanfiles $$F"; fi; \
	    done; \
	    if [ "$$_cleanfiles" != "" ]; then \
	        echo "rm -f ${CLEANFILES}"; \
	        rm -f ${CLEANFILES}; \
	    fi; \
	fi
	@if [ "${SRCS_GENERATED}" != "" ]; then \
	    echo "rm -f ${SRCS_GENERATED}"; \
	    rm -f ${SRCS_GENERATED}; \
	fi

cleandir-prog:
	rm -f *.core config.log config.status configure.lua tags
	if [ -e "./config/prefix.h" ]; then rm -fr ./config; fi
	if [ -e "Makefile.config" ]; then echo >Makefile.config; fi
	@if [ "${CONFIGSCRIPTS}" != "" ]; then \
	    echo "rm -f ${CONFIGSCRIPTS}"; \
	    rm -f ${CONFIGSCRIPTS}; \
	fi
	@if [ "${PCMODULES}" != "" ]; then \
	    echo "rm -f ${PCMODULES}"; \
	    rm -f ${PCMODULES}; \
	fi
	@if [ "${CLEANDIRFILES}" != "" ]; then \
	    echo "rm -f ${CLEANDIRFILES}"; \
	    rm -f ${CLEANDIRFILES}; \
	fi
	@if [ -e ".depend" ]; then \
		echo "echo >.depend"; \
		echo >.depend; \
	fi

install-prog:
	@if [ "${DESTDIR}" != "" ]; then \
	    echo "# Installing under DESTDIR=${DESTDIR}:"; \
	    if [ ! -e "${DESTDIR}" ]; then \
	        echo "${INSTALL_DESTDIR} ${DESTDIR}"; \
	        ${SUDO} ${INSTALL_DESTDIR} ${DESTDIR}; \
	    fi; \
	fi; \
	if [ ! -e "${DESTDIR}${BINDIR}" ]; then \
	    echo "${INSTALL_PROG_DIR} ${BINDIR}"; \
	    ${SUDO} ${INSTALL_PROG_DIR} ${DESTDIR}${BINDIR}; \
	fi
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    if [ "${PROG_TRANSFORM}" != "s,x,x," ]; then \
	    	_prog=`echo "${PROG}" |sed -e '${PROG_TRANSFORM}'`; \
	    else \
	        _prog="${PROG}"; \
	    fi; \
	    _prog=${PROG_PREFIX}$$_prog${PROG_SUFFIX}; \
	    echo "${INSTALL_PROG} ${PROG} ${BINDIR}/$$_prog${EXECSUFFIX}"; \
	    ${SUDO} ${INSTALL_PROG} ${PROG} ${DESTDIR}${BINDIR}/$$_prog${EXECSUFFIX}; \
	fi
	@if [ "${DATAFILES}" != "" ]; then \
            if [ ! -d "${DESTDIR}${DATADIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${DATADIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${DATADIR}; \
            fi; \
            for F in ${DATAFILES}; do \
                echo "${INSTALL_DATA} $$F ${DATADIR}"; \
                ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${DATADIR}; \
            done; \
	fi
	@if [ "${DATAFILES_SRC}" != "" ]; then \
            if [ ! -d "${DESTDIR}${DATADIR}" ]; then \
                echo "${INSTALL_DATA_DIR} ${DATADIR}"; \
                ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${DATADIR}; \
            fi; \
	    if [ "${SRC}" != "" ]; then \
                for F in ${DATAFILES_SRC}; do \
                    echo "${INSTALL_DATA} $$F ${DATADIR}"; \
                    ${SUDO} ${INSTALL_DATA} ${SRC}/${BUILDREL}/$$F \
		    ${DESTDIR}${DATADIR}; \
                done; \
	    else \
                for F in ${DATAFILES_SRC}; do \
                    echo "${INSTALL_DATA} $$F ${DATADIR}"; \
                    ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${DATADIR}; \
                done; \
	    fi; \
	fi
	@if [ "${CONF}" != "" ]; then \
            if [ ! -d "${DESTDIR}${SYSCONFDIR}" ]; then \
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
	@if [ "${CONFIGSCRIPTS}" != "" ]; then \
	    if [ ! -e "${DESTDIR}${BINDIR}" ]; then \
	        echo "${INSTALL_PROG_DIR} ${BINDIR}"; \
	        ${SUDO} ${INSTALL_PROG_DIR} ${DESTDIR}${BINDIR}; \
	    fi; \
            for F in ${CONFIGSCRIPTS}; do \
                echo "${INSTALL_PROG} $$F ${BINDIR}"; \
                ${SUDO} ${INSTALL_PROG} $$F ${DESTDIR}${BINDIR}; \
            done; \
	fi
	@if [ "${PKGCONFIG}" != "" -a "${PCMODULES}" != "" ]; then \
	    if [ ! -e "${DESTDIR}${PKGCONFIG_LIBDIR}" ]; then \
	        echo "${INSTALL_DATA_DIR} ${PKGCONFIG_LIBDIR}"; \
	        ${SUDO} ${INSTALL_DATA_DIR} ${DESTDIR}${PKGCONFIG_LIBDIR}; \
	    fi; \
	    for F in ${PCMODULES}; do \
	        echo "${INSTALL_DATA} $$F ${PKGCONFIG_LIBDIR}"; \
	        ${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${PKGCONFIG_LIBDIR}; \
	    done; \
	fi

deinstall-prog:
	@if [ "${PROG}" != "" -a "${PROG_INSTALL}" != "No" ]; then \
	    if [ "${PROG_TRANSFORM}" != "s,x,x," ]; then \
	    	_prog=`echo "${PROG}" |sed -e '${PROG_TRANSFORM}'`; \
	    else \
	        _prog="${PROG}"; \
	    fi; \
	    _prog=${PROG_PREFIX}$$_prog${PROG_SUFFIX}; \
	    echo "${DEINSTALL_PROG} ${BINDIR}/$$_prog${EXECSUFFIX}"; \
	    ${SUDO} ${DEINSTALL_PROG} ${DESTDIR}${BINDIR}/$$_prog${EXECSUFFIX}; \
	fi
	@if [ "${DATAFILES}" != "" ]; then \
	    for F in ${DATAFILES}; do \
	        echo "${DEINSTALL_DATA} ${DATADIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${DATADIR}/$$F; \
	    done; \
	fi
	@if [ "${DATAFILES_SRC}" != "" ]; then \
	    for F in ${DATAFILES_SRC}; do \
	        echo "${DEINSTALL_DATA} ${DATADIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${DATADIR}/$$F; \
	    done; \
	fi
	@if [ "${CONF}" != "" ]; then \
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
	@if [ "${CONFIGSCRIPTS}" != "" ]; then \
            for F in ${CONFIGSCRIPTS}; do \
                echo "${DEINSTALL_PROG} ${BINDIR}/$$F"; \
                ${SUDO} ${DEINSTALL_PROG} ${DESTDIR}${BINDIR}/$$F; \
            done; \
	fi
	@if [ "${PKGCONFIG}" != "" -a "${PCMODULES}" != "" ]; then \
	    for F in ${PCMODULES}; do \
	        echo "${DEINSTALL_DATA} ${PKGCONFIG_LIBDIR}/$$F"; \
	        ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${PKGCONFIG_LIBDIR}/$$F; \
	    done; \
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

check-prog:
	@echo check-prog

configure-prog:
	@if [ "${PROG}" != "" ]; then \
		if [ -e "configure.in" ]; then \
			echo "cat configure.in | mkconfigure > configure"; \
			cat configure.in | mkconfigure > configure; \
			if [ ! -e configure ]; then \
				echo "mkconfigure (BSDBuild) failed."; \
				exit 1; \
			fi; \
			if [ ! -x configure ]; then \
				echo "chmod 755 configure"; \
				chmod 755 configure; \
			fi; \
		fi; \
	fi

.PHONY: install deinstall clean cleandir regress depend configure
.PHONY: install-prog deinstall-prog clean-prog cleandir-prog check-prog
.PHONY: configure-prog _prog_objs prog-tags none

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.proj.mk
include ${TOP}/mk/build.subdir.mk
include .depend
