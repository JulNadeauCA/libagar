# $Csoft: csoft.common.mk,v 1.23 2004/03/17 03:41:58 vedge Exp $

# Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

PREFIX?=	/usr/local
LOCALSTATEDIR?=	${PREFIX}/share
SYSCONFDIR?=	${PREFIX}/etc
SHAREDIR?=	${PREFIX}/share
BINDIR?=	${PREFIX}/bin
LIBDIR?=	${PREFIX}/lib
INCLDIR?=	${PREFIX}/include
MANDIR?=	${PREFIX}/man
PSDIR?=		${PREFIX}/man

SUDO?=

INSTALL_PROG=		install -c -m 555
INSTALL_LIB=		install -c -m 444
INSTALL_DATA=		install -c -m 444
INSTALL_INCL=		install -c -m 444

INSTALL_PROG_DIR=	mkdir -p
INSTALL_LIB_DIR=	mkdir -p
INSTALL_DATA_DIR=	mkdir -p
INSTALL_INCL_DIR=	mkdir -p
INSTALL_MAN_DIR=	mkdir -p
INSTALL_PS_DIR=		mkdir -p

DEINSTALL_PROG=		rm -f
DEINSTALL_LIB=		rm -f
DEINSTALL_DATA=		rm -f
DEINSTALL_INCL=		rm -f

DEINSTALL_PROG_DIR=	rmdir -p
DEINSTALL_LIB_DIR=	rmdir -p
DEINSTALL_DATA_DIR=	rmdir -p
DEINSTALL_INCL_DIR=	rmdir -p

