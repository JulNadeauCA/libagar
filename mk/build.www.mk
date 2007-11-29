#
# Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistribution of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistribution in binary form must reproduce the above copyright
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
# Compile a set of HTML files (language and character set variants) from
# source files processed by m4 and xsltproc.
#

M4?=		m4
M4FLAGS?=
XSLTPROC?=	xsltproc
PERL?=		perl
ICONV?=		iconv
BASEDIR?=	m4
XSLDIR?=	xsl
TEMPLATE?=	csoft
LANGUAGES?=	en fr
DEF_LANGUAGE?=	en
XSL?=		${XSLDIR}/ml.xsl
MKDEPS=		build.www.mk build.subdir.mk build.common.mk hstrip.pl
HTMLDIR?=	none
CLEANFILES?=

all: ${HTML} all-subdir
clean: clean-www clean-subdir
cleandir: clean-www clean-subdir cleandir-subdir
install: install-www install-subdir
deinstall: deinstall-subdir
regress: regress-subdir
depend: depend-subdir

.SUFFIXES: .html .htm .jpg .jpeg .png .gif .m4

.htm.html:
	@cp -f $< ${BASEDIR}/base.htm
	@echo -n "$@:"
	@echo > $@.var
	@for LANG in ${LANGUAGES}; do \
	    echo -n " $$LANG"; \
	    ${M4} ${M4FLAGS} -D__BASE_DIR=${BASEDIR} -D__FILE=$@ \
	        -D__LANG=$$LANG \
	        ${BASEDIR}/${TEMPLATE}.m4 \
		| ${PERL} ${TOP}/mk/hstrip.pl > $@.$$LANG.prep; \
            ${XSLTPROC} --html --nonet --stringparam lang $$LANG ${XSL} \
	        $@.$$LANG.prep > $@.$$LANG.utf-8 2>/dev/null; \
	    rm -f $@.$$LANG.prep; \
	    case "$$LANG" in \
	    en) \
	        echo "URI: $@.$$LANG.utf-8" >> $@.var; \
	        echo "Content-language: $$LANG" >> $@.var; \
	        echo "Content-type: text/html;encoding=UTF-8" >> $@.var; \
	        echo "" >> $@.var; \
	        echo "URI: $@.$$LANG" >> $@.var; \
	        echo "Content-language: $$LANG" >> $@.var; \
	        echo "Content-type: text/html" >> $@.var; \
	        echo "" >> $@.var; \
	        cat $@.$$LANG.utf-8 | \
		    sed s/charset=UTF-8/charset=ISO-8859-1/ | \
		    ${ICONV} -f UTF-8 -t ISO-8859-1 > \
		    $@.$$LANG; \
	        ;; \
	    ab|af|eu|ca|da|nl|fo|fr|fi|de|is|ga|it|no|nb|nn|pt|rm|gd|es|sv|sw) \
	        echo "URI: $@.$$LANG.utf-8" >> $@.var; \
	        echo "Content-language: $$LANG" >> $@.var; \
	        echo "Content-type: text/html;encoding=UTF-8" >> $@.var; \
	        echo "" >> $@.var; \
	        echo "URI: $@.$$LANG.iso-8859-1" >> $@.var; \
	        echo "Content-language: $$LANG" >> $@.var; \
	        echo "Content-type: text/html;charset=ISO-8859-1" >> $@.var; \
	        echo "" >> $@.var; \
	        cat $@.$$LANG.utf-8 | \
		    sed s/charset=UTF-8/charset=ISO-8859-1/ | \
		    ${ICONV} -f UTF-8 -t ISO-8859-1 > \
		    $@.$$LANG.iso-8859-1; \
		cp -f $@.$$LANG.iso-8859-1 $@.$$LANG \
	        ;; \
	    *) \
	        ;; \
	    esac; \
	    echo >> $@.var; \
	done; \
	rm -f ${BASEDIR}/base.htm; \
	echo "."

clean-www:
	@for F in ${HTML}; do \
		echo "rm -f $$F $$F.var"; \
		rm -f $$F $$F.var; \
		for LANG in ${LANGUAGES}; do \
			echo "rm -f $$F.$$LANG.* $$F.$$LANG"; \
			rm -f $$F.$$LANG.* $$F.$$LANG; \
		done; \
	done
	@if [ "${CLEANFILES}" != "" ]; then \
	    echo "rm -f ${CLEANFILES}"; \
	    rm -f ${CLEANFILES}; \
	fi

install-www:
	@if [ "${HTMLDIR}" = "none" ]; then \
		exit 0; \
	fi
	@for F in ${HTML}; do \
		rm -f $$F; \
        	if [ ! -d "${HTMLDIR}" ]; then \
			echo "${INSTALL_DATA_DIR} ${HTMLDIR}"; \
			${SUDO} ${INSTALL_DATA_DIR} ${HTMLDIR}; \
		fi; \
        	if [ ! -d "${HTMLDIR}/mk" ]; then \
			echo "${INSTALL_DATA_DIR} ${HTMLDIR}/mk"; \
			${SUDO} ${INSTALL_DATA_DIR} ${HTMLDIR}/mk; \
		fi; \
		for MK in ${MKDEPS}; do \
			echo "${INSTALL_DATA} ${TOP}/mk/$$MK ${HTMLDIR}/mk"; \
			${SUDO} ${INSTALL_DATA} ${TOP}/mk/$$MK ${HTMLDIR}/mk; \
		done; \
        	if [ ! -d "${HTMLDIR}/xsl" ]; then \
			echo "${INSTALL_DATA_DIR} ${HTMLDIR}/xsl"; \
			${SUDO} ${INSTALL_DATA_DIR} ${HTMLDIR}/xsl; \
		fi; \
		for XSL in ${XSL}; do \
			if [ -e "${HTMLDIR}/xsl/$$XSL" \
			     -a "${OVERWRITE}" = "" ]; then \
				echo "xsl/$$XSL: exists; preserving"; \
			else \
				echo "${INSTALL_DATA} $$XSL ${HTMLDIR}/xsl"; \
				${SUDO} ${INSTALL_DATA} $$XSL ${HTMLDIR}/xsl; \
			fi; \
		done; \
        	if [ ! -d "${HTMLDIR}/m4" ]; then \
			echo "${INSTALL_DATA_DIR} ${HTMLDIR}/m4"; \
			${SUDO} ${INSTALL_DATA_DIR} ${HTMLDIR}/m4; \
		fi; \
		(cd m4; for M4IN in `ls -1 *.m4`; do \
			if [ -e "${HTMLDIR}/m4/$$M4IN" \
			     -a "${OVERWRITE}" = "" ]; then \
				echo "m4/$$M4IN: exists; preserving"; \
			else \
				echo "${INSTALL_DATA} $$M4IN ${HTMLDIR}/m4"; \
				${SUDO} ${INSTALL_DATA} $$M4IN ${HTMLDIR}/m4; \
			fi; \
		done); \
		if [ ! -e "${HTMLDIR}/Makefile" ]; then \
			echo "TOP=." > Makefile.prep; \
			echo "HTML=${HTML}" >> Makefile.prep; \
			echo "HTMLDIR=none" >> Makefile.prep; \
			echo "M4=${M4}" >> Makefile.prep; \
			echo "XSLTPROC=${XSLTPROC}" >> Makefile.prep; \
			echo "PERL=${PERL}" >> Makefile.prep; \
			echo "BASEDIR=${BASEDIR}" >> Makefile.prep; \
			echo "TEMPLATE=${TEMPLATE}" >> Makefile.prep; \
			echo "LANGUAGES=${LANGUAGES}" >> Makefile.prep; \
			echo "XSL=${XSL}" >> Makefile.prep; \
			echo "include mk/build.www.mk" >> Makefile.prep; \
			echo "${INSTALL_DATA} Makefile.prep \
			    ${HTMLDIR}/Makefile"; \
			${SUDO} ${INSTALL_DATA} Makefile.prep \
			    ${HTMLDIR}/Makefile; \
			rm -f Makefile.prep; \
		fi; \
		export SF=`echo $$F |sed s,.html$$,.htm,`; \
		if [ -e "${HTMLDIR}/$$SF" \
		     -a "${OVERWRITE}" = "" ]; then \
			echo "$$SF exists; preserving"; \
		else \
			echo "${INSTALL_DATA} $$SF ${HTMLDIR}"; \
			${SUDO} ${INSTALL_DATA} $$SF ${HTMLDIR}; \
		fi; \
		if [ -e "${HTMLDIR}/$$F.var" \
		     -a "${OVERWRITE}" = "" ]; then \
			echo "$$F.var exists; preserving"; \
		else \
			echo "${INSTALL_DATA} $$F.var ${HTMLDIR}"; \
			${SUDO} ${INSTALL_DATA} $$F.var ${HTMLDIR}; \
		fi; \
		for LANG in ${LANGUAGES}; do \
			for ENC in `ls -1 $$F.$$LANG*`; do \
			    if [ -e "${HTMLDIR}/$$ENC" \
			         -a "${OVERWRITE}" = "" ]; then \
				    echo "$$ENC exists; preserving"; \
			    else \
				    echo "${INSTALL_DATA} $$ENC ${HTMLDIR}"; \
				    ${SUDO} ${INSTALL_DATA} $$ENC ${HTMLDIR}; \
			    fi; \
			done; \
		done; \
	done

.PHONY: install deinstall clean cleandir regress depend
.PHONY: install-www clean-www

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
