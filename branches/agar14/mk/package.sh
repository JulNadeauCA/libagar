#!/bin/sh

INSTPREFIX=/usr/local
PKGPREFIX=usr/local

mkdir -p ${PKGPREFIX}/bin
mkdir -p ${PKGPREFIX}/lib
mkdir -p ${PKGPREFIX}/share
mkdir -p ${PKGPREFIX}/include

cp -fp ${INSTPREFIX}/bin/agar-config ${PKGPREFIX}/bin
cp -fp ${INSTPREFIX}/bin/dencomp ${PKGPREFIX}/bin
cp -fp ${INSTPREFIX}/bin/denex ${PKGPREFIX}/bin
cp -fRp ${INSTPREFIX}/lib/agar ${PKGPREFIX}/lib
cp -fRp ${INSTPREFIX}/share/agar ${PKGPREFIX}/share
cp -fRp ${INSTPREFIX}/include/agar ${PKGPREFIX}/include

