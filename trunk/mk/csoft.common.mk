# $Csoft: csoft.common.mk,v 1.18 2002/12/24 08:59:50 vedge Exp $

# Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistribution of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Neither the name of CubeSoft Communications, nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
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

PREFIX?=		/usr/local
LOCALSTATEDIR?=		${PREFIX}/share
SYSCONFDIR?=		${PREFIX}/etc

# Installation commands
INSTALL_PROG=		install -c -m 755
INSTALL_LIB=		install -c -m 444
INSTALL_DATA=		install -c -m 644
INSTALL_PROG_DIR=	mkdir -p
INSTALL_LIB_DIR=	mkdir -p
INSTALL_DATA_DIR=	mkdir -p
INSTALL_MAN_DIR=	mkdir -p

# Deinstallation commands
DEINSTALL_PROG=		rm -f
DEINSTALL_LIB=		rm -f
DEINSTALL_DATA=		rm -f
DEINSTALL_PROG_DIR=	rmdir -p
DEINSTALL_LIB_DIR=	rmdir -p
DEINSTALL_DATA_DIR=	rmdir -p

# Installation directories
SHAREDIR?=	${PREFIX}/share
INST_BINDIR?=	${PREFIX}/bin
INST_LIBDIR?=	${PREFIX}/lib
INST_MANDIR?=	${PREFIX}/man

