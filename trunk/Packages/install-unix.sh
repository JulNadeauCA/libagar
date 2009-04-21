#!/bin/sh
#
# Installation script for %PROJNAME% %VERSION% on %OS%/%ARCH%.
# Public domain
#

PROJNAME="%PROJNAME%"
VERSION="%VERSION%"
RELEASE="%RELEASE%"
AR_OS="%OS%"
AR_ARCH="%ARCH%"
AR_FLAVOR="%FLAVOR%"
SUBDIRS="bin:exec lib:exec include: share:"
INSTALL_MODE="bsd"
INSTSCRIPTNAME="install.sh"
SH="/bin/sh"
EXECSUFFIX=""
CONFIG_PROGS="agar-config agar-core-config agar-dev-config agar-math-config agar-rg-config agar-vg-config"

#
# Generic Installation
#

if [ "${INSTDIR}" = "" ]; then
	INSTDIR="."
fi
if [ -e "/bin/echo" ]; then
    /bin/echo -n ""
    if [ $? = 0 ]; then 
        ECHO_N="/bin/echo -n"
    else        
        ECHO_N="echo -n"
    fi          
else            
    ECHO_N="echo -n"
fi              

AR="${PROJNAME}-${VERSION}-${AR_OS}-${AR_ARCH}${AR_FLAVOR}.tar.gz"
ARPATH="`pwd`/${AR}"
PLATFORM=`sh ${INSTDIR}/config.guess`

if [ "${PREFIX}" = "" ]; then
	PREFIX="/usr/local"
fi

case ${PLATFORM} in
*-*-fabbsd*)
	HOST_OS="fabbsd"
	;;
*-*-freebsd*)
	HOST_OS="freebsd"
	;;
*-*-netbsd*)
	HOST_OS="netbsd"
	;;
*-*-openbsd*)
	HOST_OS="openbsd"
	;;
*-*-linux*)
	HOST_OS="linux"
	;;
*-*-irix*)
	HOST_OS="irix"
	;;
*-*-cygwin | *-*-mingw32)
	HOST_OS="mingw32"
	EXECSUFFIX=".exe"
	;;
*-*-darwin*)
	HOST_OS="macosx"
	;;
*)
	HOST_OS="unknown"
	;;
esac

case ${PLATFORM} in
x86_64-*-* | amd64-*-*)
	HOST_ARCH="amd64"
	;;
i*86-*-*)
	HOST_ARCH="i386"
	;;
mips-*-*)
	HOST_ARCH="mipsel"
	;;
macppc-*-* | powerpc-*-*)
	HOST_ARCH="powerpc"
	;;
*)
	HOST_ARCH="unknown"
	;;
esac

if [ "$1" = "--dir" ]; then
	if [ "$2" = "" ]; then
		echo "Usage: $0 --dir [dir]"
		exit 1
	fi
	DIR=$2
	if [ ! -e "$DIR" ]; then
		echo "$DIR: $!"
		exit 1
	fi
	cd "$DIR"
	for FILE in `ls -1`; do
		if [ -d "$FILE" ]; then
			if [ ! -e "${DEST}/${FILE}" ]; then
				echo "mkdir $DEST/$FILE"
				mkdir "${DEST}/${FILE}"
				if [ $? != 0 ]; then
					exit 1
				fi
			fi
			env PREFIX="${PREFIX}" MODE=${MODE} \
			    INSTDIR="${INSTDIR}" \
			    INSTSCRIPTPATH="${INSTSCRIPTPATH}" \
			    DEST="${DEST}/${FILE}" REL="${REL}/${FILE}" \
			    ${INSTSCRIPTPATH} --dir ${FILE}
			if [ $? != 0 ]; then
				exit 1
			fi
		else
			if [ "$MODE" = "exec" ]; then
				echo "install -c -m 755 $FILE ${PREFIX}/$REL"
				install -c -m 755 "$FILE" "${PREFIX}/$REL"
			else
				echo "install -c -m 644 $FILE ${PREFIX}/$REL"
				install -c -m 644 "$FILE" "${PREFIX}/$REL"
			fi
			if [ $? != 0 ]; then
				exit 1
			fi
		fi
	done
	exit 0
fi

if [ "$1" != "--force" ]; then
	if [ "${AR_OS}" != "${HOST_OS}" -o "${AR_ARCH}" != "${HOST_ARCH}" ];
	then
		echo "*"
		echo "* ERROR: Operating system mismatch."
		echo "* Your system is: ${HOST_OS}-${HOST_ARCH}."
		echo "* This package was compiled for ${AR_OS}-${AR_ARCH}."
		echo "*"
		echo "* If you want to proceed and install it anyway, use:"
		echo "* ./install.sh --force"
		echo "*"
		exit 1;
	fi
fi

echo "*"
echo "* This script will install ${PROJNAME} for ${AR_OS}-${AR_ARCH}."
echo "*"
$ECHO_N "Do you want to continue? [Y/n] "
read CONF
case x${CONF} in
xy* | xY* | x)
	;;
*)
	echo "*"
	echo "* Aborted installation."
	echo "*"
	exit 1
	;;
esac

TAR=""
for path in `echo $PATH | sed 's/:/ /g'`; do
	if [ -x "${path}/tar" ]; then
	if [ -f "${path}/tar" ]; then
		TAR="${path}/tar"
	fi
	fi
done
if [ "${TAR}" = "" ]; then
	echo "*"
	echo "* ERROR: This script requires the tar(1) utility."
	echo "* Aborted installation."
	echo "*"
	exit 1;
fi
GUNZIP=""
for path in `echo $PATH | sed 's/:/ /g'`; do
	if [ -x "${path}/gunzip" ]; then
	if [ -f "${path}/gunzip" ]; then
		GUNZIP="${path}/gunzip"
	fi
	fi
done
if [ "${GUNZIP}" = "" ]; then
	echo "*"
	echo "* ERROR: This script requires the gunzip(1) utility."
	echo "* Aborted installation."
	echo "*"
	exit 1
fi

$ECHO_N "Installation prefix? [$PREFIX] "
read UPREFIX
if [ "${UPREFIX}" != "" ]; then
if [ "${UPREFIX}" != " " ]; then
	PREFIX=${UPREFIX}
	if [ ! -e "${PREFIX}" ]; then
		echo "The directory ${PREFIX} does not exist."
		$ECHO_N "Do you want to create it? [Y/n] "
		read CONF
		case x${CONF} in
		xy* | xY* | x)
			;;
		*)
			echo "*"
			echo "* Aborted installation"
			echo "*"
			exit 1
			;;
		esac
		echo "mkdir ${PREFIX}"
		mkdir "${PREFIX}"
		if [ $? != 0 ]; then
			echo "mkdir -p ${PREFIX}"
			mkdir -p "${PREFIX}"
			if [ $? != 0 ]; then
				echo "*"
				echo "* ERROR: Could not create ${PREFIX}."
				echo "* Installation aborted."
				echo "*"
				exit 1
			fi
		fi
		echo "Created directory ${PREFIX}."
	fi
fi
fi

if [ "${INSTALL_MODE}" = "bsd" ]; then
	for DIR in ${SUBDIRS}; do
		dir=`echo $DIR |awk -F: '{print $1}'`;
		mode=`echo $DIR |awk -F: '{print $2}'`;
		if [ ! -e "${PREFIX}/$dir" ]; then
			mkdir "${PREFIX}/$dir"
			if [ $? != 0 ]; then
				echo "*"
				echo "* Failed to create ${PREFIX}/$dir"
				echo "* Installation aborted."
				echo "*"
				exit 1
			fi
			echo "Created directory ${PREFIX}/$dir."
		fi
		echo "> $dir"
		env PREFIX="${PREFIX}" MODE=$mode \
		    INSTDIR="`pwd`" \
		    INSTSCRIPTPATH="`pwd`/${INSTSCRIPTNAME}" \
		    DEST="${PREFIX}/${dir}" REL="${dir}" \
		    ${SH} ${INSTSCRIPTNAME} --dir ${dir}
		if [ $? != 0 ]; then
			echo "* "
			echo "* File installation failed"
			echo "* Installation aborted"
			echo "*"
			exit 1
		fi
	done
elif [ "${INSTALL_MODE}" = "tarball" ]; then
	if [ ! -e "${AR}" ]; then
		echo "ERROR: Cannot find archive (${AR}) in current directory."
		exit 1
	fi
	echo "Unpacking archive into ${PREFIX}..."
	(cd "${PREFIX}" && ${GUNZIP} < "${ARPATH}" | tar -xf -)
else
	echo "Bad INSTALL_MODE: ${INSTALL_MODE}"
	exit 1
fi

#
# Agar-specific installation
#

for F in ${CONFIG_PROGS}; do
	if [ -e "${PREFIX}/bin/${F}${EXECSUFFIX}" ]; then
		rm -f "${PREFIX}/bin/${F}${EXECSUFFIX}"
	fi
	cat "${F}.sh" | \
	    sed "s,%INSTALLED_VERSION%,${VERSION}," | \
	    sed "s,%INSTALLED_RELEASE%,${RELEASE}," | \
	    sed "s,%PREFIX%,${PREFIX}," > "${PREFIX}/bin/${F}"
	if [ $? != 0 ]; then
		echo "*"
		echo "* ERROR: Failed to process ${F}."
		echo "* Is sed(1) working?"
		echo "* Installation is incomplete."
		echo "*"
		exit 1
	fi
	chmod 755 "${PREFIX}/bin/${F}"
	if [ $? != 0 ]; then
		echo "*"
		echo "* ERROR: Could not set permissions on ${F}!"
		echo "*"
		exit 1
	fi
done


AGARCONFIG="no"
for path in `echo $PATH | sed 's/:/ /g'`; do
	if [ -x "${path}/agar-config" ]; then
	if [ -f "${path}/agar-config" ]; then
		AGARCONFIG="yes"
	fi
	fi
done
if [ "${AGARCONFIG}" = "no" ]; then
	echo "*"
	echo "* WARNING: The agar-config program cannot be found in your PATH!"
	echo "*"
	echo "* If you want Agar applications to compile, you must add the"
	echo "* ${PREFIX}/bin directory to your PATH environment variable."
	echo "*"
fi

if [ "${AR_OS}" = "macosx" ]; then
	$ECHO_N "Running ranlib(1) on library files..."
	for FILE in `ls -1 lib/*.a`; do
		echo "ranlib ${PREFIX}/$FILE"
		ranlib "${PREFIX}/$FILE"
	done
	echo "Done."
fi

echo "*"
echo "* ${PROJNAME}-${VERSION} was successfully installed in ${PREFIX}."
echo "*"
