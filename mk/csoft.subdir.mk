# $Csoft: csoft.subdir.mk,v 1.20 2003/09/30 02:37:24 vedge Exp $

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

MAKE?=	    make

all-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

all-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)

clean-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/ clean); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

clean-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/ clean); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)

install-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/ install); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

install-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/ install); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)

deinstall-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/ deinstall); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

deinstall-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/ deinstall); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)

depend-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/ depend); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

depend-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/ depend); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)


cleandir-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/ cleandir); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

cleandir-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/ cleandir); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)

regress-subdir:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    echo "==> ${REL}$$F"; \
		    (cd $$F && ${MAKE} REL=${REL}$$F/ regress); \
		    if [ $$? != 0 ]; then \
		    	exit 1; \
		    fi; \
		done; \
	fi)

regress-subdir-ifexists:
	@(if [ "${SUBDIR}" = "" ]; then \
	    SUBDIR="NONE"; \
	else \
	    SUBDIR="${SUBDIR}"; \
	fi; \
	if [ "$$SUBDIR" != "" -a "$$SUBDIR" != "NONE" ]; then \
		for F in $$SUBDIR; do \
		    if [ -e "$$F" ]; then \
		        echo "==> ${REL}$$F"; \
		        (cd $$F && ${MAKE} REL=${REL}$$F/ regress); \
		        if [ $$? != 0 ]; then \
		    	    exit 1; \
		        fi; \
		    fi; \
		done; \
	fi)

.PHONY:	all-subdir clean-subdir cleandir-subdir
.PHONY: install-subdir deinstall-subdir depend-subdir regress-subdir
.PHONY:	all-subdir-ifexists clean-subdir-ifexists cleandir-subdir-ifexists
.PHONY: install-subdir-ifexists deinstall-subdir-ifexists
.PHONY: depend-subdir-ifexists regress-subdir-ifexists
