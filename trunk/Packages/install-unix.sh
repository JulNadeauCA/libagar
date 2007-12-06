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

#
# Generic Installation
#

AR="${PROJNAME}-${VERSION}-${AR_OS}-${AR_ARCH}${AR_FLAVOR}.tar.gz"
ARPATH="`pwd`/${AR}"
HOST_OS=`uname -s`
HOST_ARCH=`uname -m`

if [ "${PREFIX}" = "" ]; then
	PREFIX="/usr/local"
fi

if [ "${HOST_OS}" = "FreeBSD" ]; then	HOST_OS="freebsd";	fi
if [ "${HOST_OS}" = "NetBSD" ]; then	HOST_OS="netbsd";	fi
if [ "${HOST_OS}" = "OpenBSD" ]; then	HOST_OS="openbsd";	fi
if [ "${HOST_OS}" = "Linux" ]; then	HOST_OS="linux";	fi
if [ "${HOST_OS}" = "IRIX" ]; then	HOST_OS="irix";		fi
if [ "${HOST_OS}" = "IRIX64" ]; then	HOST_OS="irix";		fi
if [ "${HOST_OS}" = "CYGWIN_NT-5.1" ]; then HOST_OS="cygwin";	fi
if [ "${HOST_OS}" = "Darwin" ]; then	HOST_OS="darwin";	fi

if [ "${HOST_ARCH}" = "x86_64" ]; then	HOST_ARCH="amd64";	fi
if [ "${HOST_ARCH}" = "i486" ]; then	HOST_ARCH="i386";	fi
if [ "${HOST_ARCH}" = "i586" ]; then	HOST_ARCH="i386";	fi
if [ "${HOST_ARCH}" = "i686" ]; then	HOST_ARCH="i386";	fi
if [ "${HOST_ARCH}" = "mips" ]; then	HOST_ARCH="mipsel";	fi
if [ "${HOST_ARCH}" = "Power Macintosh" ]; then	HOST_ARCH="powerpc"; fi

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
	cd $DIR
	for FILE in `ls -1`; do
		if [ -d $FILE ]; then
			if [ ! -e "$DEST/$FILE" ]; then
				echo "mkdir $DEST/$FILE"
				mkdir $DEST/$FILE
				if [ $? != 0 ]; then
					exit 1
				fi
			fi
			env PREFIX=${PREFIX} MODE=$MODE INST=$INST \
			    DEST=$DEST/$FILE REL=$REL/$FILE \
			    $INST --dir $FILE
			if [ $? != 0 ]; then
				exit 1
			fi
		else
			if [ "$MODE" = "exec" ]; then
				echo "install -c -m 755 $FILE ${PREFIX}/$REL"
				install -c -m 755 $FILE ${PREFIX}/$REL
			else
				echo "install -c -m 644 $FILE ${PREFIX}/$REL"
				install -c -m 644 $FILE ${PREFIX}/$REL
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
		echo "* ERROR: Operating system mismatch"
		echo -n "* Your system is ${HOST_OS}-${HOST_ARCH}. "
		echo "This package was compiled for ${AR_OS}-${AR_ARCH}."
		echo -n "* If you want to install it anyway, run this script "
		echo "with the --force option."
		echo "*"
		exit 1;
	fi
fi

echo "This script will install ${PROJNAME} for ${AR_OS}-${AR_ARCH}."
echo -n "Do you want to continue? [Y/n] "
read CONF

if [ "${CONF}" != "y" -a "${CONF}" != "Y" -a "${CONF}" != "" \
     -a "${CONF}" != " " ]; then
	echo "* Installation aborted."
	exit 1;
fi

TAR=""
for path in `echo $PATH | sed 's/:/ /g'`; do
	if [ -x "${path}/tar" ]; then TAR="${path}/tar"; fi
done
if [ "${TAR}" = "" ]; then
	echo "ERROR: This installation program requires the tar(1) utility."
	exit 1;
fi
GUNZIP=""
for path in `echo $PATH | sed 's/:/ /g'`; do
	if [ -x "${path}/gunzip" ]; then GUNZIP="${path}/gunzip"; fi
done
if [ "${GUNZIP}" = "" ]; then
	echo "ERROR: This installation program requires the gunzip(1) utility."
	exit 1
fi

echo -n "Installation prefix? [$PREFIX] "
read UPREFIX
if [ "${UPREFIX}" != "" -a "${UPREFIX}" != " " ]; then
	PREFIX=${UPREFIX}
	if [ ! -e "${PREFIX}" ]; then
		echo "The directory ${PREFIX} does not exist."
		echo -n "Do you want to create it? [Y/n] "
		read CONF
		if [ "${CONF}" = "y" -o "${CONF}" = "Y" -o "${CONF}" = "" \
		     -o "${CONF}" = " " ]; then
			mkdir ${PREFIX}
			if [ $? != 0 ]; then
				mkdir -p ${PREFIX}
				if [ $? != 0 ]; then
					echo "ERROR: Cannot create ${PREFIX}";
					exit 1
				fi
			fi
		fi
		echo "Created directory ${PREFIX}."
	fi
fi

if [ "${INSTALL_MODE}" = "bsd" ]; then
	for DIR in ${SUBDIRS}; do
		dir=`echo $DIR |awk -F: '{print $1}'`;
		mode=`echo $DIR |awk -F: '{print $2}'`;
		if [ ! -e "${PREFIX}/$dir" ]; then
			mkdir ${PREFIX}/$dir
			if [ $? != 0 ]; then
				echo "Failed to create ${PREFIX}/$dir"
				exit 1
			fi
			echo "Created directory ${PREFIX}/$dir."
		fi
		echo "> $dir"
		env PREFIX=${PREFIX} MODE=$mode INST=`pwd`/install \
		    DEST=${PREFIX}/$dir REL=$dir \
		    ./install --dir $dir
		if [ $? != 0 ]; then
			echo "File installation failed"
			exit 1
		fi
	done
elif [ "${INSTALL_MODE}" = "tarball" ]; then
	if [ ! -e "${AR}" ]; then
		echo "ERROR: Cannot find archive (${AR}) in current directory."
		exit 1
	fi
	echo "Unpacking archive into ${PREFIX}..."
	(cd ${PREFIX} && ${GUNZIP} < ${ARPATH} | tar -xf -)
else
	echo "Bad INSTALL_MODE: ${INSTALL_MODE}"
	exit 1
fi

#
# Agar-specific installation
#

rm -f ${PREFIX}/bin/agar-config
cat agar-config.sh | \
    sed "s,%INSTALLED_VERSION%,${VERSION}," | \
    sed "s,%INSTALLED_RELEASE%,${RELEASE}," | \
    sed "s,%PREFIX%,${PREFIX}," > ${PREFIX}/bin/agar-config
if [ $? != 0 ]; then
	echo "ERROR: Failed to process agar-config. Installation is incomplete."
	exit 1
fi

chmod 755 ${PREFIX}/bin/agar-config
if [ $? != 0 ]; then
	echo "ERROR: Failed to set permissions on agar-config"
	exit 1
fi

AGARCONFIG="no"
for path in `echo $PATH | sed 's/:/ /g'`; do
	if [ -x "${path}/agar-config" ]; then AGARCONFIG="yes"; fi
done
if [ "${AGARCONFIG}" = "no" ]; then
	echo "WARNING: The agar-config program cannot be found in your PATH."
	echo "If you want agar applications to compile, you will need to"
	echo "add the ${PREFIX}/bin directory to your PATH."
fi

echo "${PROJNAME} was installed successfully into ${PREFIX}."
