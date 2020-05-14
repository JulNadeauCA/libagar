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
# Install manual pages in mandoc format.
#

MANDOC?=
PAGER?=more
MAN1?=
MAN2?=
MAN3?=
MAN4?=
MAN5?=
MAN6?=
MAN7?=
MAN8?=
MAN9?=
MANS=${MAN1} ${MAN2} ${MAN3} ${MAN4} ${MAN5} ${MAN6} ${MAN7} ${MAN8} ${MAN9}
MANLINKS?=
NOMAN?=
NOMANLINKS?=
CLEANFILES?=

all: all-subdir
install: install-man install-subdir
deinstall: deinstall-man deinstall-subdir
clean: clean-man clean-subdir
cleandir: clean-man clean-subdir cleandir-subdir
regress: regress-subdir
depend: depend-subdir
lint: lint-man

clean-man:
	@if [ "${CLEANFILES}" != "" ]; then \
		echo "rm -f ${CLEANFILES}"; \
		rm -f ${CLEANFILES}; \
	fi

install-man:
	@if [ "${NOMAN}" != "yes" ]; then \
		if [ "${MANS}" != "       " ]; then \
			if [ "${DESTDIR}" != "" ]; then \
				echo "# Installing under DESTDIR=${DESTDIR}:"; \
				if [ ! -e "${DESTDIR}" ]; then \
					echo "${INSTALL_DESTDIR} ${DESTDIR}"; \
					${SUDO} ${INSTALL_DESTDIR} ${DESTDIR}; \
				fi; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man1" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man1"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man1; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man2" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man2"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man2; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man3" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man3"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man3; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man4" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man4"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man4; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man5" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man5"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man5; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man6" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man6"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man6; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man7" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man7"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man7; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man8" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man8"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man8; \
			fi; \
			if [ ! -d "${DESTDIR}${MANDIR}/man9" ]; then \
				echo "${INSTALL_MAN_DIR} ${MANDIR}/man9"; \
				${SUDO} ${INSTALL_MAN_DIR} ${DESTDIR}${MANDIR}/man9; \
			fi; \
		fi; \
		if [ "${MAN1}" != "" ]; then \
			for F in ${MAN1}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man1"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man1; \
			done; \
		fi; \
		if [ "${MAN2}" != "" ]; then \
			for F in ${MAN2}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man2"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man2; \
			done; \
		fi; \
		if [ "${MAN3}" != "" ]; then \
			for F in ${MAN3}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man3"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man3; \
			done; \
		fi; \
		if [ "${MAN4}" != "" ]; then \
			for F in ${MAN4}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man4"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man4; \
			done; \
		fi; \
		if [ "${MAN5}" != "" ]; then \
			for F in ${MAN5}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man5"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man5; \
			done; \
		fi; \
		if [ "${MAN6}" != "" ]; then \
			for F in ${MAN6}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man6"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man6; \
			done; \
		fi; \
		if [ "${MAN7}" != "" ]; then \
			for F in ${MAN7}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man7"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man7; \
			done; \
		fi; \
		if [ "${MAN8}" != "" ]; then \
			for F in ${MAN8}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man8"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man8; \
			done; \
		fi; \
		if [ "${MAN9}" != "" ]; then \
			for F in ${MAN9}; do \
				echo "${INSTALL_DATA} $$F ${MANDIR}/man9"; \
				${SUDO} ${INSTALL_DATA} $$F ${DESTDIR}${MANDIR}/man9; \
			done; \
		fi; \
		if [ "${NOMANLINKS}" != "yes" -a "${MANLINKS}" != "" ]; then \
			${ECHO_N} "# Installing manlinks ( "; \
			(cd ${DESTDIR}${MANDIR} && \
			 for L in ${MANLINKS}; do \
				MPG=`echo $$L | sed 's/:.*//'`; \
				MLNK=`echo $$L | sed 's/.*://'`; \
				MS=`echo $$L | sed 's/.*\.//'`; \
				${ECHO_N} "$$MLNK "; \
				${SUDO} ${LN} -fs $$MPG man$$MS/$$MLNK; \
			 done); \
			 echo " )."; \
		fi; \
	fi

deinstall-man:
	@if [ "${NOMAN}" != "yes" ]; then \
		if [ "${MAN1}" != "" ]; then \
			for F in ${MAN1}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man1/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man1/$$FL; \
			done; \
		fi; \
		if [ "${MAN2}" != "" ]; then \
			for F in ${MAN2}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man2/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man2/$$FL; \
			done; \
		fi; \
		if [ "${MAN3}" != "" ]; then \
			for F in ${MAN3}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man3/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man3/$$FL; \
			done; \
		fi; \
		if [ "${MAN4}" != "" ]; then \
			for F in ${MAN4}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man4/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man4/$$FL; \
			done; \
		fi; \
		if [ "${MAN5}" != "" ]; then \
			for F in ${MAN5}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man5/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man5/$$FL; \
			done; \
		fi; \
		if [ "${MAN6}" != "" ]; then \
			for F in ${MAN6}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man6/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man6/$$FL; \
			done; \
		fi; \
		if [ "${MAN7}" != "" ]; then \
			for F in ${MAN7}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man7/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man7/$$FL; \
			done; \
		fi; \
		if [ "${MAN8}" != "" ]; then \
			for F in ${MAN8}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man8/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man8/$$FL; \
			done; \
		fi; \
		if [ "${MAN9}" != "" ]; then \
			for F in ${MAN9}; do \
				FL=`echo $$F | sed 's:.*/::'`; \
				echo "${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man9/$$FL"; \
				${SUDO} ${DEINSTALL_DATA} ${DESTDIR}${MANDIR}/man9/$$FL; \
			done; \
		fi; \
		if [ "${NOMANLINKS}" != "yes" -a "${MANLINKS}" != "" ]; then \
			if [ "${MAN1}" != "" -a -e "${DESTDIR}${MANDIR}/man1" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man1 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN2}" != "" -a -e "${DESTDIR}${MANDIR}/man2" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man2 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN3}" != "" -a -e "${DESTDIR}${MANDIR}/man3" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man3 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN4}" != "" -a -e "${DESTDIR}${MANDIR}/man4" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man4 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN5}" != "" -a -e "${DESTDIR}${MANDIR}/man5" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man5 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN6}" != "" -a -e "${DESTDIR}${MANDIR}/man6" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man6 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN7}" != "" -a -e "${DESTDIR}${MANDIR}/man7" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man7 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN8}" != "" -a -e "${DESTDIR}${MANDIR}/man8" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man8 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
			if [ "${MAN9}" != "" -a -e "${DESTDIR}${MANDIR}/man9" ]; then \
				${ECHO_N} "# Deinstalling manlinks ( "; \
				(cd ${DESTDIR}${MANDIR}/man9 && \
				 for L in ${MANLINKS}; do \
					MLNK=`echo $$L | sed 's/.*://'`; \
					${ECHO_N} "$$MLNK "; \
					${SUDO} ${DEINSTALL_DATA} $$MLNK; \
				 done); \
				 echo " )."; \
			fi; \
		fi; \
	fi
	
man:
	@if [ "${MAN}" != "" ]; then \
		echo "${MANDOC} -Tascii ${MAN} | ${PAGER}"; \
		${MANDOC} -Tascii ${MAN} | ${PAGER}; \
	else \
		echo "Usage: ${MAKE} man MAN=(manpage)"; \
		exit 1; \
	fi

manlinks: Makefile
	echo > .manlinks.mk
	@if [ "${MANS}" != "       " ]; then \
		for F in ${MANS}; do \
			echo "cat $$F |manlinks.pl $$F >>.manlinks.mk"; \
			cat $$F |perl ${TOP}/mk/manlinks.pl $$F >>.manlinks.mk; \
		done; \
	fi

all-manlinks:
	@if [ "${SRC}" != "" ]; then \
		(cd ${SRC} && \
		 for DIR in `find . -name .manlinks.mk |sed 's/\/\.manlinks\.mk//'`; do \
			echo "(cd $$DIR && ${MAKE} manlinks)"; \
			(cd $$DIR && ${MAKE} manlinks); \
		done); \
	else \
		for DIR in `find . -name .manlinks.mk |sed 's/\/\.manlinks\.mk//'`; do \
			echo "(cd $$DIR && ${MAKE} manlinks)"; \
			(cd $$DIR && ${MAKE} manlinks); \
		done; \
	fi

lint-man:
	@if [ "${HAVE_MANDOC}" != "yes" ]; then \
		echo "Cannot find mandoc (re-run ./configure?)"; \
		exit 1; \
	fi
	@if [ "${MAN1}" != "" ]; then for F in ${MAN1}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN2}" != "" ]; then for F in ${MAN2}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN3}" != "" ]; then for F in ${MAN3}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN4}" != "" ]; then for F in ${MAN4}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN5}" != "" ]; then for F in ${MAN5}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN6}" != "" ]; then for F in ${MAN6}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN7}" != "" ]; then for F in ${MAN7}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN8}" != "" ]; then for F in ${MAN8}; do ${MANDOC} -Tlint $$F; done; fi
	@if [ "${MAN9}" != "" ]; then for F in ${MAN9}; do ${MANDOC} -Tlint $$F; done; fi

.PHONY: install deinstall clean cleandir regress depend lint
.PHONY: install-man deinstall-man clean-man 
.PHONY: man manlinks all-manlinks lint-man

include ${TOP}/mk/build.common.mk
