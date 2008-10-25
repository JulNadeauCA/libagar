#
# Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
# For Makefiles using <build.prog.mk> and <build.lib.mk>, generate project
# files for various IDEs using Premake (http://premake.sourceforge.net/).
#

PREMAKE?=	premake
ZIP?=		zip
ZIPFLAGS?=	-r
MKPROJFILES?=	mkprojfiles
PREMAKEOUT?=	premake.lua
PREMAKEFLAGS?=

PROJECT?=
PROJDIR?=	ProjectFiles
PROJFILESEXTRA?=
PROJINCLUDES?=${TOP}/configure.lua
PROJFILELIST=	.projfiles2.out
PROJPREPKG?=
PROJPOSTPKG?=

PROJFILES?=	windows:i386:cb-gcc:: \
		windows:i386:vs6:: \
		windows:i386:vs2002:: \
		windows:i386:vs2003:: \
		windows:i386:vs2005::

CLEANFILES+=	premake.lua configure.lua

proj: proj-subdir
	@if [ "${PROJECT}" = "" ]; then \
	    echo "Checking ${PROJINCLUDES}"; \
	    for INCL in ${PROJINCLUDES}; do \
	        if [ ! -e "$$INCL" ]; then \
	            echo "Missing $$INCL; generating"; \
	            (cd ${TOP} && cat configure.in | mkconfigure \
	             --emul-env=vs2005 --emul-os=windows \
		     --emul-arch=i386 >/dev/null); \
	        fi; \
	    done; \
	    cat Makefile | ${MKPROJFILES} "" ${PROJINCLUDES} > ${PREMAKEOUT};\
	else \
	    if [ ! -d "${PROJDIR}" ]; then \
	    	echo "mkdir -p ${PROJDIR}"; \
	    	mkdir -p ${PROJDIR}; \
	    fi; \
	    for TGT in ${PROJFILES}; do \
	        _tgtos=`echo $$TGT |awk -F: '{print $$1}' `; \
	        _tgtarch=`echo $$TGT |awk -F: '{print $$2}' `; \
	        _tgtproj=`echo $$TGT |awk -F: '{print $$3}' `; \
	        _tgtflav=`echo $$TGT |awk -F: '{print $$4}' `; \
	        _tgtopts=`echo $$TGT |awk -F: '{print $$5}'|sed 's/,/ /g'`; \
		echo "Target: $$_tgtos ($$_tgtproj)"; \
		echo "Target flavor: $$_tgtflav"; \
		echo "Target options: $$_tgtopts"; \
		rm -fR config; \
		cat configure.in | \
		    mkconfigure --emul-env=$$_tgtproj --emul-os=$$_tgtos \
		    --emul-arch=$$_tgtarch > configure.tmp; \
		if [ $$? != 0 ]; then \
			echo "mkconfigure failed"; \
			rm -fR configure.tmp configure.lua; \
			exit 1; \
		fi; \
		/bin/sh ./configure.tmp $$_tgtopts --with-proj-generation; \
		if [ $$? != 0 ]; then \
			echo "configure failed"; \
			echo > Makefile.config; \
			exit 1; \
		fi; \
		rm -f configure.tmp config.log; \
		echo >Makefile.config; \
	        perl ${TOP}/mk/cmpfiles.pl; \
	        echo "cat Makefile | ${MKPROJFILES} "$$_tgtflav" \
		    ${PROJINCLUDES} > ${PREMAKEOUT}";\
	        cat Makefile | ${MKPROJFILES} "$$_tgtflav" \
		    ${PROJINCLUDES} > ${PREMAKEOUT};\
	        echo "${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} \
		    --os $$_tgtos --target $$_tgtproj"; \
	        ${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} \
		    --os $$_tgtos --target $$_tgtproj; \
		if [ $$? != 0 ]; then \
			echo "premake failed"; \
			exit 1; \
		fi; \
	        perl ${TOP}/mk/cmpfiles.pl added > .projfiles.out; \
		cp -f .projfiles.out ${PROJFILELIST}; \
	        rm .cmpfiles.out; \
		if [ "${PROJFILESEXTRA}" != "" ]; then \
	            for EXTRA in ${PROJFILESEXTRA}; do \
		        echo "$$EXTRA" >> ${PROJFILELIST}; \
		    done; \
		fi; \
	        echo "config" >> ${PROJFILELIST}; \
		rm -f ${PROJDIR}/$$_tgtproj-$$_tgtos.zip; \
		if [ "${PROJPREPKG}" != "" ]; then \
			echo "${MAKE} ${PROJPREPKG}"; \
			env PKG_OS=$$_tgtos PKG_ARCH=$$_tgtarch \
			    PKG_IDE=$$_tgtproj ${MAKE} ${PROJPREPKG}; \
		fi; \
		cat ${PROJFILELIST} | ${ZIP} ${ZIPFLAGS} \
		    ${PROJDIR}/$$_tgtproj-$$_tgtos-$$_tgtarch$$_tgtflav.zip -@;\
		if [ "${PROJPOSTPKG}" != "" ]; then \
			echo "${MAKE} ${PROJPOSTPKG}"; \
			env PKG_OS=$$_tgtos PKG_ARCH=$$_tgtarch \
			    PKG_IDE=$$_tgtproj ${MAKE} ${PROJPOSTPKG}; \
		fi; \
		rm -fR config .projfiles.out ${PROJFILELIST}; \
	    done; \
	fi

.PHONY: proj
