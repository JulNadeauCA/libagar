# $Csoft: Makefile,v 1.1 2004/01/23 06:29:07 vedge Exp $

TOP=	../..

DOC=		agar-manual.ps
MACROS=		-mdoc
DOCSRC=		Cover.tr \
		Macros.tr \
		Manual.tr \
		Contents.tr

include ${TOP}/mk/csoft.doc.mk

update: ${DOC}
	scp ${DOC} vedge@resin:www/agar/docs

.PHONY: ${DOC}

