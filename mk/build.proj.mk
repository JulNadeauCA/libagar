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
PROJDIR?=	${TOP}/ProjectFiles
PROJFILESEXTRA?=
PROJINCLUDES?=	${TOP}/configure.lua

PROJTARGETS=	windows:cb-gcc \
		windows:vs6 \
		windows:vs2002 \
		windows:vs2003 \
		windows:vs2005

proj: proj-subdir
	@if [ "${PROJECT}" = "" ]; then \
	    cat Makefile | ${MKPROJFILES} ${PROJINCLUDES} > ${PREMAKEOUT};\
	else \
	    if [ ! -d "${PROJDIR}" ]; then \
	    	echo "mkdir -p ${PROJDIR}"; \
	    	mkdir -p ${PROJDIR}; \
	    fi; \
	    for TGT in ${PROJTARGETS}; do \
	        _tgtos=`echo $$TGT |awk -F: '{print $$1}' `; \
	        _tgtproj=`echo $$TGT |awk -F: '{print $$2}' `; \
		echo "Target: $$_tgtos ($$_tgtproj)"; \
	        perl ${TOP}/mk/cmpfiles.pl; \
	        cat Makefile | ${MKPROJFILES} ${PROJINCLUDES} > ${PREMAKEOUT};\
	        echo "${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} --os $$_tgtos --target $$_tgtproj"; \
	        ${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} --os $$_tgtos --target $$_tgtproj;\
		rm -f premake.lua; \
	        perl ${TOP}/mk/cmpfiles.pl added > .projfiles.out; \
		cp -f .projfiles.out .projfiles2.out; \
	        rm .cmpfiles.out; \
		if [ "${PROJFILESEXTRA}" != "" ]; then \
	            for EXTRA in ${PROJFILESEXTRA}; do \
		        echo "$$EXTRA" >> .projfiles2.out; \
		    done; \
		fi; \
		if [ -e "config.$$_tgtos" ]; then \
			if [ -e "config" ]; then \
				echo "mv -f config config.ORIG"; \
				mv -f config config.ORIG; \
			fi; \
			echo "mv -f config.$$_tgtos config"; \
			mv -f config.$$_tgtos config; \
			echo "mv -f config/.svn config.svn.ORIG"; \
			mv -f config/.svn config.svn.ORIG; \
		        echo "config" >> .projfiles2.out; \
			CONFIGFOUND=yes; \
		else \
		        CONFIGFOUND=no; \
		fi; \
		rm -f ${PROJDIR}/$$_tgtproj-$$_tgtos.zip; \
		cat .projfiles2.out | ${ZIP} ${ZIPFLAGS} \
		    ${PROJDIR}/$$_tgtproj-$$_tgtos.zip -@; \
		if [ "$$CONFIGFOUND" = "yes" ]; then \
		    echo "mv -f config.svn.ORIG config/.svn"; \
		    mv -f config.svn.ORIG config/.svn; \
		    echo "mv -f config config.$$_tgtos"; \
		    mv -f config config.$$_tgtos; \
		    if [ -e "config.ORIG" ]; then \
			    echo "mv -f config.ORIG config"; \
			    mv -f config.ORIG config; \
		    fi; \
		fi; \
		rm `cat .projfiles.out`; \
	    done; \
	fi

.PHONY: proj
