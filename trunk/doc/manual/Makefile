# $Csoft: Makefile,v 1.2 2004/01/25 05:11:03 vedge Exp $

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

update: ${DOC}
	scp ${DOC} vedge@resin:www/agar/docs

.PHONY: ${DOC}

