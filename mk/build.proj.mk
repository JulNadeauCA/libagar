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
MKPROJFILES?=	mkprojfiles
PREMAKEOUT?=	premake.lua
PREMAKEFLAGS?=

PROJECT?=
PROJDIR?=	${TOP}/ProjectFiles
# PROJTARGETS=	monodev vs2002 vs2003 sharpdev
PROJTARGETS=	cb-gcc \
		vs6 \
		vs2002 \
		vs2003 \
		vs2005

proj: proj-subdir
	@if [ "${PROJECT}" = "" ]; then \
	    cat Makefile | ${MKPROJFILES} > ${PREMAKEOUT};\
	else \
	    if [ ! -d "${PROJDIR}" ]; then \
	    	echo "mkdir -p ${PROJDIR}"; \
	    	mkdir -p ${PROJDIR}; \
	    fi; \
	    for TGT in ${PROJTARGETS}; do \
	        perl ${TOP}/mk/cmpfiles.pl; \
	        cat Makefile | ${MKPROJFILES} > ${PREMAKEOUT};\
	        echo "${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} --target $$TGT"; \
	        ${PREMAKE} ${PREMAKEFLAGS} --file ${PREMAKEOUT} --target $$TGT;\
	        perl ${TOP}/mk/cmpfiles.pl added > .projfiles.out; \
	        rm .cmpfiles.out; \
		cat .projfiles.out | zip ${PROJDIR}/$$TGT.zip -@; \
		rm `cat .projfiles.out`; \
	    done; \
	fi

.PHONY: proj
