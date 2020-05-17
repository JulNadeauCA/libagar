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
# Build static and shared libraries from source.
#

LIB?=
LIB_INSTALL?=	No
LIB_SHARED?=	No
LIB_PROFILE?=	No
LIB_MODULE?=	No
LIB_CURRENT?=	1
LIB_REVISION?=	0
LIB_AGE?=	0
LIB_GUID?=
LIB_BUNDLE?=

ADA?=		ada
ADABIND?=	gnatbind
ADAPREP?=	gnatprep
ADAPREPFLAGS?=
ADAPREPFILE?=
AR?=		ar
ASM?=		nasm
CC?=		cc
CC_COMPILE?=	-c
CXX?=		c++
LEX?=		lex
LN?=		ln
OBJC?=		cc
RANLIB?=	ranlib
SH?=		sh
WINDRES?=
YACC?=		yacc

ADAFLAGS?=
ADABFLAGS?=	-x
ASMFLAGS?=	-g -w-orphan-labels
CFLAGS?=
CPPFLAGS?=
CXXFLAGS?=
LFLAGS?=
LIBL?=		-ll
MKDEP=		sh ${TOP}/mk/mkdep
MKDEP_ADA?=	gnatmake
MKDEP_ADAFLAGS?=-M -u -v
OBJCFLAGS?=
PICFLAGS?=	-fPIC
YFLAGS?=	-d

USE_LIBTOOL?=	No
LTBASE?=	${TOP}/mk/libtool
LIBTOOL_COOKIE?=${TOP}/mk/libtool.ok
LIBTOOL?=	${LTBASE}/libtool
LTCONFIG?=	${LTBASE}/configure
LTCONFIG_DEPS?=	${LTBASE}/config.guess \
		${LTBASE}/config.sub \
		${LTBASE}/aclocal.m4 \
		${LTBASE}/ltmain.sh
LTCONFIG_LOG?=	${LTBASE}/config.log
LIBTOOLFLAGS?=
LIBTOOLOPTS?=	--quiet
LIBTOOLOPTS_SHARED?=
LIBTOOLOPTS_STATIC?=

CONF?=
CONF_OVERWRITE?=No
CONFIGSCRIPTS?=
CTAGS?=
CTAGSFLAGS?=
CLEANFILES?=
CLEANDIRFILES?=
DATAFILES?=
DATAFILES_SRC?=
INCL?=
INCLDIR?=
OBJS?=
PCMODULES?=
SRCS?=
SRCS_GENERATED?=
SHOBJS?=
WINRES?=

all: all-subdir lib${LIB}.a lib${LIB}.so lib${LIB}.la
install: install-lib install-subdir
deinstall: deinstall-lib deinstall-subdir
clean: clean-lib clean-subdir
cleandir: clean-lib clean-subdir cleandir-lib cleandir-subdir
regress: regress-subdir
configure: configure-lib

.SUFFIXES: .ads .adb .asm .c .cc .cpp .l .lo .m .o .y

# Compile Ada code into an object file
.adb.o:
	@_cflags="${CFLAGS}"; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="$$_cflags ${PICFLAGS}"; fi; \
	echo "${ADA} ${ADAFLAGS} $$_cflags -c $<"; \
	${ADA} ${ADAFLAGS} $$_cflags -c $<

.ads.o:
	@_cflags="${CFLAGS}"; FB=`echo "$<" |sed 's/.ads$$//'`; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="$$_cflags ${PICFLAGS}"; fi; \
	if [ -e "$$FB.adb" ]; then \
	    echo "${ADA} ${ADAFLAGS} $$_cflags -c $$FB.adb"; \
	    ${ADA} ${ADAFLAGS} $$_cflags -c $$FB.adb; \
	else \
	    echo "${ADA} ${ADAFLAGS} $$_cflags -c $<"; \
	    ${ADA} ${ADAFLAGS} $$_cflags -c $<; \
	fi

# Compile assembly code into an object file
.asm.o:
	${ASM} ${ASMFLAGS} ${CPPFLAGS} -o $@ $<

# Compile C code into an object file
.c.o:
	@_cflags=""; _out="$@"; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _cflags="$$_cflags -pg -DPROF"; fi; \
	if [ "${HAVE_CC65}" = "yes" ]; then _out=`echo "$@" | sed 's/.o$$/.s/'`; fi; \
	echo "${CC} ${CFLAGS} ${CPPFLAGS} $$_cflags -o $$_out ${CC_COMPILE} $<"; \
	${CC} ${CFLAGS} ${CPPFLAGS} $$_cflags -o $$_out ${CC_COMPILE} $<; \
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

.c.lo:
	${LIBTOOL} ${LIBTOOLOPTS} --mode=compile \
	    ${CC} ${LIBTOOLFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ ${CC_COMPILE} $<

# Compile Objective-C code into an object file
.m.o:
	@_objcflags=""; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _objcflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _objcflags="$$_objcflags -pg -DPROF"; fi; \
	echo "${OBJC} ${CFLAGS} ${OBJCFLAGS} $$_objcflags ${CPPFLAGS} -o $@ -c $<"; \
	${OBJC} ${CFLAGS} ${OBJCFLAGS} $$_objcflags ${CPPFLAGS} -o $@ -c $<
.m.lo:
	${LIBTOOL} ${LIBTOOLOPTS} --mode=compile \
	    ${OBJC} ${LIBTOOLFLAGS} ${CFLAGS} ${OBJCFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile C++ code into an object file
.cc.o .cpp.o:
	@_cxxflags=""; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cxxflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _cxxflags="$$_cxxflags -pg -DPROF"; fi; \
	echo "${CXX} ${CXXFLAGS} $$_cxxflags ${CPPFLAGS} -o $@ -c $<"; \
	${CXX} ${CXXFLAGS} $$_cxxflags ${CPPFLAGS} -o $@ -c $<
.cc.lo .cpp.lo:
	${LIBTOOL} ${LIBTOOLOPTS} --mode=compile \
	    ${CXX} ${LIBTOOLFLAGS} ${CXXFLAGS} ${CPPFLAGS} -o $@ -c $<

# Compile a Lex lexer into an object file
.l:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	@_cflags=""; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _cflags="$$_cflags -pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} ${LDFLAGS} -o $@ $@.yy.c ${LIBL} ${LIBS}"; 
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} ${LDFLAGS} -o $@ $@.yy.c ${LIBL} ${LIBS}
	@rm -f $@.yy.c
.l.o:
	${LEX} ${LFLAGS} -o$@.yy.c $<
	@_cflags=""; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _cflags="$$_cflags -pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.yy.c"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.yy.c
	@mv -f $@.yy.o $@
	@rm -f $@.yy.c

# Compile a Yacc parser into an object file
.y:
	${YACC} ${YFLAGS} -b $@ $<
	@_cflags=""; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _cflags="$$_cflags -pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} ${LDFLAGS} -o $@ $@.tab.c ${LIBS}"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} ${LDFLAGS} -o $@ $@.tab.c ${LIBS}
	@rm -f $@.tab.c
.y.o:
	${YACC} ${YFLAGS} -b $@ $<
	@_cflags=""; \
	if [ "${LIB_SHARED}" = "Yes" ]; then _cflags="${PICFLAGS}"; fi; \
	if [ "${LIB_PROFILE}" = "Yes" ]; then _cflags="$$_cflags -pg -DPROF"; fi; \
	echo "${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.tab.c"; \
	${CC} ${CFLAGS} $$_cflags ${CPPFLAGS} -o $@ ${CC_COMPILE} $@.tab.c
	@mv -f $@.tab.o $@
	@rm -f $@.tab.c

# Generate or update make dependencies
depend:	${SRCS_GENERATED} check-libtool lib-tags depend-subdir
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

# Build the library's object files (non-concurrent build only)
_lib_objs:
	@if [ "${LIB}" != "" -a "${OBJS}" = "" -a "${SRCS}" != "" \
	      -a "${USE_LIBTOOL}" = "No" ]; then \
	    _objs=""; \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	        F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	        F=`echo $$F | sed 's/.cpp$$/.o/'`; \
		_objs="$$_objs $$F"; \
            done; \
	    echo "${MAKE} $$_objs"; \
	    ${MAKE} $$_objs; \
	    if [ $$? != 0 ]; then \
	        echo "${MAKE}: failure"; \
	        exit 1; \
	    fi; \
	fi
	@if [ "${WINRES}" != "" -a "${WINDRES}" != "" ]; then \
	    echo "${WINDRES} -o ${WINRES}.o ${WINRES}"; \
	    ${WINDRES} -o ${WINRES}.o ${WINRES}; \
	fi

# Build Libtool object files (if using Libtool, non-concurrent build only)
_lib_ltobjs:
	@if [ "${LIB}" != "" -a "${SHOBJS}" = "" -a "${SRCS}" != "" \
	      -a "${USE_LIBTOOL}" = "Yes" ]; then \
	    _ltobjs=""; \
	    for F in ${SRCS}; do \
	        F=`echo $$F | sed 's/.ad[bs]$$/.lo/'`; \
	        F=`echo $$F | sed 's/.asm$$/.lo/'`; \
	        F=`echo $$F | sed 's/.[clym]$$/.lo/'`; \
	        F=`echo $$F | sed 's/.cc$$/.lo/'`; \
	        F=`echo $$F | sed 's/.cpp$$/.lo/'`; \
		_ltobjs="$$_ltobjs $$F"; \
	    done; \
	    ${MAKE} $$_ltobjs; \
	    if [ $$? != 0 ]; then \
	        echo "${MAKE}: failure"; \
	        exit 1; \
	    fi; \
	fi

# Build a static library.
lib${LIB}.a: ${SRCS_GENERATED} _lib_objs ${OBJS}
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "No" -a \
	      "${SRCS}" != "" -a "${.TARGETS}" != "install" ]; then \
	    _objs="${OBJS}"; \
	    if [ "$$_objs" = "" ]; then \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	    _objs="$$_objs $$F"; \
                done; \
	    fi; \
	    if [ "${HAVE_CC65}" = "yes" ]; then \
	        echo "ar65 a ${LIB}.lib $$_objs"; \
	        ar65 a ${LIB}.lib $$_objs; \
		echo "cp ${LIB}.lib lib${LIB}.a"; \
		cp ${LIB}.lib lib${LIB}.a; \
	    else \
	        echo "${AR} -cru lib${LIB}.a $$_objs"; \
	        ${AR} -cru lib${LIB}.a $$_objs; \
	        echo "${RANLIB} lib${LIB}.a"; \
	        (${RANLIB} lib${LIB}.a || exit 0); \
	    fi; \
	fi

# Build a shared library (without Libtool)
lib${LIB}.so: ${SRCS_GENERATED} _lib_objs ${OBJS}
	@if [ "${LIB}" != "" -a "${LIB_SHARED}" = "Yes" -a \
	      "${USE_LIBTOOL}" = "No" -a "${SRCS}" != "" -a \
	      "${.TARGETS}" != "install" ]; then \
	    \
	    case "${HOST}" in \
	    *-darwin*) \
	        _libout="lib${LIB}.${LIB_CURRENT}.dylib"; \
	        _libnames="lib${LIB}.dylib"; \
		;; \
	    *-mingw*) \
	        _libout="${LIB}.dll"; \
	        _libnames=""; \
		;; \
	    *) \
	        _libout="lib${LIB}.so.${LIB_CURRENT}.${LIB_REVISION}.${LIB_AGE}"; \
	        _libnames="lib${LIB}.so.${LIB_CURRENT} lib${LIB}.so"; \
	        ;; \
	    esac; \
	    \
	    _objs="${OBJS}"; \
	    if [ "$$_objs" = "" ]; then \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.o/'`; \
	    	    _objs="$$_objs $$F"; \
                done; \
	    fi; \
	    \
	    case "${HOST}" in \
	    *-darwin*) \
	        echo "${CC} -shared -o $$_libout -Wl,-rpath,${PREFIX}/lib ${LDFLAGS} -dynamiclib -install_name lib${LIB}.dylib $$_objs ${LIBS}"; \
	        ${CC} -shared -o $$_libout -Wl,-rpath ${PREFIX}/lib ${LDFLAGS} -dynamiclib -install_name lib${LIB}.dylib $$_objs ${LIBS}; \
	        ;; \
	    *-mingw*) \
	        echo "${CC} -shared -o $$_libout -Wl,--out-implib,lib${LIB}_dll.lib -Wl,-rpath ${PREFIX}/lib ${LDFLAGS} $$_objs ${LIBS}"; \
	        ${CC} -shared -o $$_libout -Wl,--out-implib,lib${LIB}_dll.lib -Wl,-rpath ${PREFIX}/lib ${LDFLAGS} $$_objs ${LIBS}; \
	        ;; \
	    *) \
	        echo "${CC} -shared -o $$_libout -Wl,-rpath,${PREFIX}/lib ${LDFLAGS} $$_objs"; \
	        ${CC} -shared -o $$_libout -Wl,-rpath ${PREFIX}/lib ${LDFLAGS} $$_objs; \
	        ;; \
	    esac; \
	    \
	    for LIBNAME in $$_libnames; do \
	        echo "${LN} -fs $$_libout $$LIBNAME"; \
	        ${LN} -fs $$_libout $$LIBNAME; \
	    done; \
	    \
	    echo "# lib${LIB}.la - a libtool library file" > lib${LIB}.la; \
	    echo "# Generated by build.lib.mk(5) from BSDBuild ${BSDBUILD_VERSION}" >> lib${LIB}.la; \
	    echo '# <https://bsdbuild.hypertriton.com/>' >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# The name that we can dlopen(3).' >> lib${LIB}.la; \
	    echo "dlname='$$_libout'" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Names of this library.' >> lib${LIB}.la; \
	    echo "library_names='$$_libout $$_libnames'" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# The name of the static archive.' >> lib${LIB}.la; \
	    echo "old_library='lib${LIB}.a'" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    _linker_flags=""; \
	    _dependency_libs=""; \
	    for F in ${LIBS}; do \
	    	if echo "$$F" | grep -q '^-[lL]'; then \
		    _dependency_libs="$$_dependency_libs $$F"; \
		else \
		    _linker_flags="$$_linker_flags $$F"; \
		fi; \
	    done; \
	    echo '# Linker flags that can not go in dependency_libs.' >> lib${LIB}.la; \
	    echo "inherited_linker_flags='$$_linker_flags'" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Libraries that this one depends upon.' >> lib${LIB}.la; \
	    echo "dependency_libs='$$_dependency_libs'" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Names of additional weak libraries provided by this library' >> lib${LIB}.la; \
	    echo "weak_library_names=''" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Version information for lib${LIB}.' >> lib${LIB}.la; \
	    echo "current=${LIB_CURRENT}" >> lib${LIB}.la; \
	    echo "age=${LIB_AGE}" >> lib${LIB}.la; \
	    echo "revision=${LIB_REVISION}" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Is this an already installed library?' >> lib${LIB}.la; \
	    echo 'installed=no' >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Should we warn about portability when linking against -modules?' >> lib${LIB}.la; \
	    echo 'shouldnotlink=no' >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Files to dlopen/dlpreopen' >> lib${LIB}.la; \
	    echo "dlopen=''" >> lib${LIB}.la; \
	    echo "dlpreopen=''" >> lib${LIB}.la; \
	    echo >> lib${LIB}.la; \
	    echo '# Directory that this library needs to be installed in:' >> lib${LIB}.la; \
	    echo "libdir='${PREFIX}/lib'" >> lib${LIB}.la; \
	    if [ "${LIB_BUNDLE}" != "" ]; then \
	        echo "perl ${TOP}/mk/gen-bundle.pl lib ${LIB_BUNDLE}"; \
	        perl ${TOP}/mk/gen-bundle.pl lib ${LIB_BUNDLE}; \
	    fi; \
	fi

# Build a shared library using libtool
lib${LIB}.la: check-libtool ${SRCS_GENERATED} _lib_ltobjs ${SHOBJS}
	@if [ "${LIB}" != "" -a "${USE_LIBTOOL}" = "Yes" -a \
	      "${SRCS}" != "" -a "${.TARGETS}" != "install" ]; then \
	    _ltobjs="${SHOBJS}"; \
	    _moduleopts=""; \
	    if [ "$$_ltobjs" = "" ]; then \
	        for F in ${SRCS}; do \
	    	    F=`echo $$F | sed 's/.ad[bs]$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.asm$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.[clym]$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.cc$$/.lo/'`; \
	    	    F=`echo $$F | sed 's/.cpp$$/.lo/'`; \
	    	    _ltobjs="$$_ltobjs $$F"; \
                done; \
	    fi; \
	    if [ "${LIB_MODULE}" = "Yes" ]; then \
	        _moduleopts="-module";  \
	    fi; \
	    if [ "${LIB_SHARED}" = "Yes" ]; then \
	        echo "${LIBTOOL} ${LIBTOOLOPTS} --mode=link ${CC} -o lib${LIB}.la ${LIBTOOLOPTS_SHARED} -rpath ${PREFIX}/lib $$_moduleopts -version-info ${LIB_CURRENT}:${LIB_REVISION}:${LIB_AGE} ${LDFLAGS} $$_ltobjs ${LIBS}"; \
	        ${LIBTOOL} ${LIBTOOLOPTS} --mode=link \
		    ${CC} -o lib${LIB}.la ${LIBTOOLOPTS_SHARED} \
		    -rpath ${PREFIX}/lib $$_moduleopts \
		    -version-info ${LIB_CURRENT}:${LIB_REVISION}:${LIB_AGE} \
		    ${LDFLAGS} $$_ltobjs ${LIBS}; \
	    else \
	        echo "${LIBTOOL} ${LIBTOOLOPTS} --mode=link ${CC} -o lib${LIB}.la -static ${LIBTOOLOPTS_STATIC} ${LDFLAGS} $$_ltobjs ${LIBS}"; \
	        ${LIBTOOL} ${LIBTOOLOPTS} --mode=link \
		    ${CC} -o lib${LIB}.la -static ${LIBTOOLOPTS_STATIC} \
		    ${LDFLAGS} $$_ltobjs ${LIBS}; \
	    fi; \
	    if [ "${LIB_BUNDLE}" != "" ]; then \
	        echo "perl ${TOP}/mk/gen-bundle.pl lib ${LIB_BUNDLE}"; \
	        perl ${TOP}/mk/gen-bundle.pl lib ${LIB_BUNDLE}; \
	    fi; \
	fi

clean-lib:
	@if [ "${LIB}" != "" -a "${SRCS}" != "" ]; then \
	    if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	        _objs="${SHOBJS}"; \
	        if [ "$$_objs" = "" ]; then \
                    for F in ${SRCS}; do \
	    	        F=`echo $$F | sed 's/.ad[bs]$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.asm$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.[clym]$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.cc$$/.lo/'`; \
	    	        F=`echo $$F | sed 's/.cpp$$/.lo/'`; \
			_objs="$$_objs $$F"; \
	    	        F=`echo $$F | sed 's/.ad[bs]$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.asm$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.[clym]$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.cc$$/.o/'`; \
	    	        F=`echo $$F | sed 's/.cpp$$/.o/'`; \
			_objs="$$_objs $$F"; \
                    done; \
		else \
		    if [ "${SHOBJS}" != "" ]; then \
		        for F in ${SHOBJS}; do \
	    	            F=`echo $$F | sed 's/.lo$$/.o/'`; \
			    _objs="$$_objs $$F"; \
                        done; \
		    fi; \
		fi; \
	    	echo "rm -f lib${LIB}.la $$_objs"; \
	    	rm -f lib${LIB}.la $$_objs; \
		if [ -e ".libs" ]; then \
		    echo "rm -fR .libs"; \
		    rm -fR .libs; \
		fi; \
	    else \
	        _objs="${OBJS}"; \
	        if [ "$$_objs" = "" ]; then \
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
		case "${HOST}" in \
		*-darwin*) \
	   	    echo "rm -f lib${LIB}.a lib${LIB}.${LIB_CURRENT}.dylib lib${LIB}.dylib lib${LIB}.la"; \
	   	    rm -f lib${LIB}.a lib${LIB}.${LIB_CURRENT}.dylib \
		          lib${LIB}.dylib lib${LIB}.la; \
		    ;; \
		*-mingw*) \
	   	    echo "rm -f lib${LIB}.a lib${LIB}_dll.lib ${LIB}.dll lib${LIB}.la"; \
	   	    rm -f lib${LIB}.a lib${LIB}_dll.lib ${LIB}.dll lib${LIB}.la; \
		    ;; \
		*) \
		    echo "rm -f lib${LIB}.a lib${LIB}.so lib${LIB}.so.${LIB_CURRENT} lib${LIB}.so.${LIB_CURRENT}.${LIB_REVISION}.${LIB_AGE} lib${LIB}.la"; \
		    rm -f lib${LIB}.a lib${LIB}.so lib${LIB}.so.${LIB_CURRENT} \
		          lib${LIB}.so.${LIB_CURRENT}.${LIB_REVISION}.${LIB_AGE} \
			  lib${LIB}.la; \
		    ;; \
		esac; \
	    fi; \
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
	@if [ "${HAVE_CC65}" = "yes" ]; then \
	    echo "rm -f ${LIB}.lib *.s"; \
	    rm -f ${LIB}.lib *.s; \
	fi

cleandir-lib:
	rm -f ${LIBTOOL} ${LIBTOOL_COOKIE} ${LTCONFIG_LOG} config.log config.status tags
	@if [ -e "./config/prefix.h" ]; then \
	    echo "rm -fr ./config"; \
	    rm -fr ./config; \
	fi
	@if [ -e "Makefile.config" ]; then \
	    echo "echo >Makefile.config"; \
	    echo >Makefile.config; \
	fi
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

install-lib: check-libtool
	@if [ "${DESTDIR}" != "" ]; then \
	    echo "# Installing under DESTDIR=${DESTDIR}:"; \
	    if [ ! -e "${DESTDIR}" ]; then \
	        echo "${INSTALL_DESTDIR} ${DESTDIR}"; \
	        ${SUDO} ${INSTALL_DESTDIR} ${DESTDIR}; \
	    fi; \
	fi
	@if [ "${INCL}" != "" ]; then \
	    if [ ! -d "${DESTDIR}${INCLDIR}" ]; then \
                echo "${INSTALL_INCL_DIR} ${INCLDIR}"; \
                ${SUDO} ${INSTALL_INCL_DIR} ${DESTDIR}${INCLDIR}; \
	    fi; \
	    for F in ${INCL}; do \
	        echo "${INSTALL_INCL} $$F ${INCLDIR}"; \
	        ${SUDO} ${INSTALL_INCL} $$F ${DESTDIR}${INCLDIR}; \
	    done; \
	fi
	@if [ "${LIB}" != "" -a "${LIB_INSTALL}" = "Yes" ]; then \
	    if [ ! -d "${DESTDIR}${LIBDIR}" ]; then \
                echo "${INSTALL_LIB_DIR} ${LIBDIR}"; \
                ${SUDO} ${INSTALL_LIB_DIR} ${DESTDIR}${LIBDIR}; \
	    fi; \
	    if [ ! -d "${DESTDIR}${INCLDIR}" ]; then \
                echo "${INSTALL_INCL_DIR} ${INCLDIR}"; \
                ${SUDO} ${INSTALL_INCL_DIR} ${DESTDIR}${INCLDIR}; \
	    fi; \
	    if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	        echo "${LIBTOOL} ${LIBTOOLOPTS} --mode=install ${INSTALL_LIB} lib${LIB}.la ${LIBDIR}"; \
	        ${SUDO} ${LIBTOOL} ${LIBTOOLOPTS} --mode=install ${INSTALL_LIB} lib${LIB}.la ${DESTDIR}${LIBDIR}; \
	        echo "${LIBTOOL} ${LIBTOOLOPTS} --finish ${LIBDIR}"; \
	        ${SUDO} ${LIBTOOL} ${LIBTOOLOPTS} --finish ${DESTDIR}${LIBDIR}; \
	    else \
	    	if [ "${LIB_SHARED}" = "Yes" ]; then \
	            sed 's/installed=no/installed=yes/' lib${LIB}.la > lib${LIB}.la.$$$$; \
		    case "${HOST}" in \
		    *-darwin*) \
		        _libout="lib${LIB}.${LIB_CURRENT}.dylib"; \
	    	        echo "${INSTALL_LIB} $$_libout ${LIBDIR}"; \
	                ${SUDO} ${INSTALL_LIB} $$_libout ${DESTDIR}${LIBDIR}; \
			echo "(cd ${LIBDIR} && ${LN} -fs $$_libout lib${LIB}.dylib)"; \
			(cd ${DESTDIR}${LIBDIR} && ${SUDO} ${LN} -fs $$_libout lib${LIB}.dylib); \
		    	;; \
		    *-mingw*) \
		        _libout="${LIB}.dll"; \
	    	        echo "${INSTALL_PROG} $$_libout ${BINDIR}"; \
	                ${SUDO} ${INSTALL_PROG} $$_libout ${DESTDIR}${BINDIR}; \
	    	        echo "${INSTALL_LIB} lib${LIB}_dll.lib ${LIBDIR}"; \
	                ${SUDO} ${INSTALL_LIB} lib${LIB}_dll.lib ${DESTDIR}${LIBDIR}; \
			echo "(cd ${LIBDIR} && ${LN} -fs $$_libout lib${LIB}.so.${LIB_CURRENT})"; \
			(cd ${DESTDIR}${LIBDIR} && ${SUDO} ${LN} -fs $$_libout lib${LIB}.so.${LIB_CURRENT}); \
			echo "(cd ${LIBDIR} && ${LN} -fs $$_libout lib${LIB}.so)"; \
			(cd ${DESTDIR}${LIBDIR} && ${SUDO} ${LN} -fs $$_libout lib${LIB}.so); \
		    	;; \
		    *) \
	                _libout="lib${LIB}.so.${LIB_CURRENT}.${LIB_REVISION}.${LIB_AGE}"; \
	    	        echo "${INSTALL_LIB} $$_libout ${LIBDIR}"; \
	                ${SUDO} ${INSTALL_LIB} $$_libout ${DESTDIR}${LIBDIR}; \
			echo "(cd ${LIBDIR} && ${LN} -fs $$_libout lib${LIB}.so.${LIB_CURRENT})"; \
			(cd ${DESTDIR}${LIBDIR} && ${SUDO} ${LN} -fs $$_libout lib${LIB}.so.${LIB_CURRENT}); \
			echo "(cd ${LIBDIR} && ${LN} -fs $$_libout lib${LIB}.so)"; \
			(cd ${DESTDIR}${LIBDIR} && ${SUDO} ${LN} -fs $$_libout lib${LIB}.so); \
			;; \
		    esac; \
	    	    echo "${INSTALL_LIB} lib${LIB}.la ${LIBDIR}"; \
	            ${SUDO} ${INSTALL_LIB} lib${LIB}.la.$$$$ ${DESTDIR}${LIBDIR}/lib${LIB}.la; \
		    rm -f lib${LIB}.la.$$$$; \
		fi; \
	        if [ "${HAVE_CC65}" = "yes" ]; then \
	            echo "${INSTALL_LIB} ${LIB}.lib ${LIBDIR}"; \
	            ${SUDO} ${INSTALL_LIB} ${LIB}.lib ${DESTDIR}${LIBDIR}; \
	        else \
	            echo "${INSTALL_LIB} lib${LIB}.a ${LIBDIR}"; \
	            ${SUDO} ${INSTALL_LIB} lib${LIB}.a ${DESTDIR}${LIBDIR}; \
	        fi; \
	    fi; \
	    for F in ${SRCS}; do \
	        if echo $$F | grep -q '.ad[bs]$$'; then \
		    FB=`echo "$$F" | sed 's/.ad[bs]$$//'`; \
	            if [ -e "$$FB.ads" ]; then \
			if [ "${ADAPREPFILE}" != "" ]; then \
				echo "${ADAPREP} ${ADAPREPFLAGS} $$FB.ads ${INCLDIR}/$$FB.ads ${ADAPREPFILE}"; \
				${SUDO} ${ADAPREP} ${ADAPREPFLAGS} $$FB.ads ${INCLDIR}/$$FB.ads ${ADAPREPFILE}; \
			else \
	                	echo "${INSTALL_INCL} $$FB.ads ${INCLDIR}"; \
	                	${SUDO} ${INSTALL_INCL} $$FB.ads ${DESTDIR}${INCLDIR}; \
			fi; \
		    fi; \
	            echo "${INSTALL_DATA} $$FB.ali ${INCLDIR}"; \
	            ${SUDO} ${INSTALL_DATA} $$FB.ali ${DESTDIR}${INCLDIR}; \
                fi; \
	    done; \
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
	@case "${HOST}" in \
	    *-linux*) \
	        if [ "${EUID}" = "0" -o "${USER}" = "root" ]; then \
	            if [ -x "/sbin/ldconfig" ]; then \
	                echo "/sbin/ldconfig"; \
	                /sbin/ldconfig; \
		    fi; \
		else \
		    echo "*"; \
		    echo "* You may need to run /sbin/ldconfig."; \
		    echo "*"; \
		fi; \
		;; \
	esac

deinstall-lib: check-libtool
	@if [ "${LIB}" != "" ]; then \
	    if [ "${USE_LIBTOOL}" = "Yes" ]; then \
	        echo "${LIBTOOL} ${LIBTOOLOPTS} --mode=uninstall rm -f ${LIBDIR}/lib${LIB}.la"; \
	        ${SUDO} ${LIBTOOL} ${LIBTOOLOPTS} --mode=uninstall rm -f ${DESTDIR}${LIBDIR}/lib${LIB}.la; \
	    else \
	    	if [ "${LIB_SHARED}" = "Yes" ]; then \
	            case "${HOST}" in \
	            *-darwin*) \
	                _libout="lib${LIB}.${LIB_CURRENT}.dylib"; \
	                _libnames="$$_libout lib${LIB}.dylib"; \
		        ;; \
	            *-mingw*) \
	                _libout="${LIB}.dll"; \
	                _libnames="$$_libout"; \
		        ;; \
	            *) \
	                _libout="lib${LIB}.so.${LIB_CURRENT}.${LIB_REVISION}.${LIB_AGE}"; \
	                _libnames="$$_libout lib${LIB}.so.${LIB_CURRENT} lib${LIB}.so"; \
	                ;; \
	            esac; \
		    for F in $$_libnames; do \
	                echo "${DEINSTALL_LIB} ${LIBDIR}/$$F"; \
	                ${SUDO} ${DEINSTALL_LIB} ${DESTDIR}${LIBDIR}/$$F; \
		    done; \
		fi; \
	        if [ "${HAVE_CC65}" = "yes" ]; then \
	            echo "${DEINSTALL_LIB} ${LIBDIR}/${LIB}.lib"; \
	            ${SUDO} ${DEINSTALL_LIB} ${DESTDIR}${LIBDIR}/${LIB}.lib; \
	        else \
	            echo "${DEINSTALL_LIB} ${LIBDIR}/lib${LIB}.a"; \
	            ${SUDO} ${DEINSTALL_LIB} ${DESTDIR}${LIBDIR}/lib${LIB}.a; \
	        fi; \
	        echo "${DEINSTALL_LIB} ${LIBDIR}/lib${LIB}.la"; \
	        ${SUDO} ${DEINSTALL_LIB} ${DESTDIR}${LIBDIR}/lib${LIB}.la; \
	    fi; \
	    for F in ${SRCS}; do \
	        if echo $$F | grep -q '.ad[bs]$$'; then \
		    FB=`echo "$$F" | sed 's/.ad[bs]$$//'`; \
	            if [ -e "$$FB.ads" ]; then \
	                echo "${DEINSTALL_INCL} ${INCLDIR}/$$FB.ads"; \
	                ${SUDO} ${DEINSTALL_INCL} ${DESTDIR}${INCLDIR}/$$FB.ads; \
		    fi; \
	            echo "${DEINSTALL_DATA} ${INCLDIR}/$$FB.ali"; \
	            ${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${INCLDIR}/$$FB.ali; \
                fi; \
	    done; \
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

includes:
	(cd ${TOP} && ${MAKE} install-includes)

check-libtool:
	@if [ "${USE_LIBTOOL}" = "Yes" -a "${LIBTOOL_BUNDLED}" = "yes" ]; then \
	    if [ ! -e "${LIBTOOL_COOKIE}" ]; then \
	        echo "(cd ${LTBASE} && \
	            ${SH} ./configure --build=${BUILD} --host=${HOST})"; \
	        (cd ${LTBASE} && env CC="${CC}" OBJC="${OBJC}" CXX="${CXX}" \
	            CFLAGS="${CFLAGS}" OBJCFLAGS="${OBJCFLAGS}" CXXFLAGS="${CXXFLAGS}" \
		    ${SH} ./configure --build=${BUILD} --host=${HOST}); \
	        if [ $$? != 0 ]; then \
	    	    echo "USE_LIBTOOL=Yes and ${LTCONFIG} failed"; \
	    	    exit 1; \
	        fi; \
	        if [ ! -f "${LIBTOOL}" ]; then \
		    echo "mv libtool ${LIBTOOL}"; \
		    mv libtool ${LIBTOOL}; \
	        fi; \
	        echo "echo "${LIBTOOL}" > ${LIBTOOL_COOKIE}"; \
	        echo "${LIBTOOL}" > ${LIBTOOL_COOKIE}; \
	    fi; \
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

${LTCONFIG} ${LTCONFIG_DEPS}:

configure-lib:
	@if [ "${LIB}" != "" ]; then \
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

.PHONY: install deinstall includes clean cleandir regress depend configure
.PHONY: install-lib deinstall-lib clean-lib cleandir-lib configure-lib
.PHONY: _lib_objs _lib_ltobjs lib-tags check-libtool none

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.proj.mk
include ${TOP}/mk/build.subdir.mk
include .depend
