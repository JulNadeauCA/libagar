# $Csoft: Makefile,v 1.4 2004/02/14 05:06:57 vedge Exp $

TOP=	../..

VER=		1.0
DOCPS=		agar-devguide-${VER}.ps
DOCPDF=		agar-devguide-${VER}.pdf
MACROS=		-me

DOCSRC=		00.Macros.me \
		00.Intro.me \
		00.Chapters.me \
		01.Overview.me \
		02.Initialization.me \
		03.Object.me \
		04.Den.me

DOCSRC+=	A.Code-listings.me \
		B.List-of-figures.me \
		00.Contents.me

include ${TOP}/mk/csoft.doc.mk

update: ${DOC}
	scp ${DOC} vedge@resin:www/agar/docs

.PHONY: upload
