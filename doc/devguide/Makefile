# $Csoft: Makefile,v 1.2 2004/01/25 04:15:09 vedge Exp $

TOP=	../..

DOC=		agar-devguide.ps
OUTPUT=		ps
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
