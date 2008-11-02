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
PROJFILELIST=	.projfiles2.out
PROJPREPKG?=
PROJPOSTPKG?=
PROJCONFIGDIR?=

PROJFILES?=	bsd:cb-gcc:: \
		linux:cb-gcc:: \
		macosx:cb-gcc:: \
		windows:cb-gcc:: \
		windows:cb-ow:: \
		windows:vs6:: \
		windows:vs2002:: \
		windows:vs2003:: \
		windows:vs2005:: \
		windows:vs2008::

CLEANFILES+=	${PREMAKEOUT} ${PROJINCLUDES}

proj-package:
	@if [ "${PROJECT}" = "" ]; then \
	    echo "cat Makefile | ${MKPROJFILES} > ${PREMAKEOUT}"; \
	    cat Makefile | \
	        env PROJTARGET="${PROJTARGET}" PROJOS="${PROJOS}" \
		PROJFLAVOR="" PROJINCLUDES="${PROJINCLUDES}" \
	        ${MKPROJFILES} > ${PREMAKEOUT}; \
	fi

proj:
	@if [ ! -d "${PROJDIR}" ]; then \
		echo "mkdir -p ${PROJDIR}"; \
		mkdir -p ${PROJDIR}; \
	fi
	@for TGT in ${PROJFILES}; do \
		_tgtos=`echo $$TGT |awk -F: '{print $$1}' `; \
		_tgtproj=`echo $$TGT |awk -F: '{print $$2}' `; \
		_tgtflav=`echo $$TGT |awk -F: '{print $$3}' `; \
		_tgtopts=`echo $$TGT |awk -F: '{print $$4}'|sed 's/,/ /g'`; \
		echo "*"; \
		echo "* Target: $$_tgtos ($$_tgtproj)"; \
		echo "* Target flavor: $$_tgtflav"; \
		echo "* Target options: $$_tgtopts"; \
		echo "*"; \
		\
		if [ -e "config" ]; then \
			echo "rm -fR config"; \
			rm -fR config; \
		fi; \
		if [ -e "include" ]; then \
			echo "rm -fR include"; \
			rm -fR include; \
		fi; \
		echo "mkconfigure --emul-env=$$_tgtproj --emul-os=$$_tgtos \
		    > configure.tmp"; \
		cat configure.in | \
		    mkconfigure --emul-env=$$_tgtproj --emul-os=$$_tgtos \
		    > configure.tmp; \
		if [ $$? != 0 ]; then \
			echo "mkconfigure failed"; \
			rm -fR configure.tmp ${PROJINCLUDES}; \
			exit 1; \
		fi; \
		echo "./configure.tmp $$_tgtopts --with-proj-generation"; \
		${SH} ./configure.tmp $$_tgtopts --with-proj-generation; \
		if [ $$? != 0 ]; then \
			echo "configure failed"; \
			echo > Makefile.config; \
			exit 1; \
		fi; \
		echo "${MAKE} proj-package-subdir"; \
		env PROJTARGET="$$_tgtproj" PROJOS="$$_tgtos" \
		    PROJINCLUDES="${PROJINCLUDES}" \
		    ${MAKE} proj-package-subdir; \
		\
		if [ "${PROJCONFIGDIR}" != "" ]; then \
			echo "rm -fR ${PROJCONFIGDIR}"; \
			rm -fR ${PROJCONFIGDIR}; \
			echo "cp -fR config ${PROJCONFIGDIR}"; \
			cp -fR config ${PROJCONFIGDIR}; \
		fi; \
		echo "rm -f configure.tmp config.log"; \
		rm -f configure.tmp config.log; \
		echo >Makefile.config; \
		\
	        perl ${TOP}/mk/cmpfiles.pl; \
	        echo "cat Makefile | ${MKPROJFILES} > ${PREMAKEOUT}"; \
	        cat Makefile | \
		    env PROJFLAVOR="$$_tgtflav" \
		    PROJOS="$$_tgtos" \
		    PROJINCLUDES="${PROJINCLUDES}" \
		    ${MKPROJFILES} > ${PREMAKEOUT}; \
	        echo "${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} \
		    --os $$_tgtos --target $$_tgtproj"; \
	        ${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} \
		    --os $$_tgtos --target $$_tgtproj; \
		if [ $$? != 0 ]; then \
			echo "premake failed"; \
			exit 1; \
		fi; \
	        perl ${TOP}/mk/cmpfiles.pl added > .projfiles.out; \
		echo "* Generated files: "; \
		cat .projfiles.out; \
		cp -f .projfiles.out ${PROJFILELIST}; \
	        rm .cmpfiles.out; \
		if [ "${PROJFILESEXTRA}" != "" ]; then \
	            for EXTRA in ${PROJFILESEXTRA}; do \
		        echo "+ $$EXTRA: "; \
		        echo "$$EXTRA" >> ${PROJFILELIST}; \
		    done; \
		fi; \
		echo "+ config"; \
	        echo "config" >> ${PROJFILELIST}; \
		echo "+ include"; \
	        echo "include" >> ${PROJFILELIST}; \
		echo "rm -f ${PROJDIR}/$$_tgtproj-$$_tgtos$$_tgtflav.zip"; \
		rm -f "${PROJDIR}/$$_tgtproj-$$_tgtos$$_tgtflav.zip"; \
		if [ "${PROJPREPKG}" != "" ]; then \
			echo "${MAKE} ${PROJPREPKG}"; \
			env PKG_OS=$$_tgtos PKG_IDE=$$_tgtproj \
			    PROJINCLUDES="${PROJINCLUDES}" \
			    ${MAKE} ${PROJPREPKG}; \
		fi; \
		echo "* Creating $$_tgtproj-$$_tgtos$$_tgtflav.zip";\
		cat ${PROJFILELIST} | ${ZIP} ${ZIPFLAGS} \
		    ${PROJDIR}/$$_tgtproj-$$_tgtos$$_tgtflav.zip -@;\
		if [ "${PROJPOSTPKG}" != "" ]; then \
			echo "${MAKE} ${PROJPOSTPKG}"; \
			env PKG_OS=$$_tgtos PKG_IDE=$$_tgtproj \
			    PROJINCLUDES="${PROJINCLUDES}" \
			    ${MAKE} ${PROJPOSTPKG}; \
		fi; \
		echo "* Cleaning up"; \
		cat .projfiles.out | perl ${TOP}/mk/cleanfiles.pl; \
		rm -fR include config ${PROJFILELIST}; \
		rm -f .projfiles.out ${PROJINCLUDES}; \
	done
	${MAKE} proj-clean
	@echo "* Done"

proj-clean: proj-clean-subdir
	@echo "rm -f ${PREMAKEOUT}"
	@rm -f ${PREMAKEOUT}

.PHONY: proj
