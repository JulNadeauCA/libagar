# $Csoft: csoft.dep.mk,v 1.13 2003/03/05 16:13:09 vedge Exp $

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

MKDEP=	sh ${TOP}/mk/mkdep
CC?=	cc

depend:	${DPADD} depend-subdir
	@rm -f .depend
	@files="${SRCS}"; \
	 if [ "$$files" != "" ]; then \
	  if [ "${BUILD}" != "" ]; then \
	   env CC=${CC} ${MKDEP} -a ${MKDEP} ${CFLAGS} -I`pwd` -I${BUILD} \
	       $$files; \
	  else \
	   env CC=${CC} ${MKDEP} -a ${MKDEP} ${CFLAGS} -I`pwd` $$files; \
	  fi; \
	 fi

clean-depend:
	rm -f .depend

