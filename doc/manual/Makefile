# $Csoft: Makefile,v 1.3 2004/02/14 05:07:33 vedge Exp $

TOP=	../..

VER=		1.0
DOCPS=		agar-manual-${VER}.ps
DOCPDF=		agar-manual-${VER}.pdf
MACROS=		-mdoc
DOCSRC=		Cover.tr \
		Macros.tr \
		Manual.tr \
		Contents.tr

include ${TOP}/mk/csoft.doc.mk

update: ${DOCPS} ${DOCPDF}
	scp ${DOCPS} ${DOCPDF} vedge@resin:www/agar/docs

.PHONY: ${DOCPS} ${DOCPDF}

