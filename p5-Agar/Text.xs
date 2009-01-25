/*
 * Copyright (c) 2009 Mat Sutcliffe (oktal@gmx.co.uk)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>
#include "perl_agar.h"

MODULE = Agar::Text	PACKAGE = Agar::Text	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

void
PushState()
CODE:
	AG_PushTextState();

void
PopState()
CODE:
	AG_PopTextState();

void
Justify(mode)
	const char * mode
CODE:
	switch (mode[0]) {
		case 'l': case 'L': AG_TextJustify(AG_TEXT_LEFT); break;
		case 'r': case 'R': AG_TextJustify(AG_TEXT_RIGHT); break;
		case 'c': case 'C': AG_TextJustify(AG_TEXT_CENTER); break;
	}

void
Valign(mode)
	const char * mode
CODE:
	switch (mode[0]) {
		case 't': case 'T': AG_TextJustify(AG_TEXT_TOP); break;
		case 'm': case 'M': AG_TextJustify(AG_TEXT_MIDDLE); break;
		case 'b': case 'B': AG_TextJustify(AG_TEXT_BOTTOM); break;
	}

void
ColorRGB(r, g, b)
	Uint8 r
	Uint8 g
	Uint8 b
CODE:
	AG_TextColorRGB(r, g, b);

void
ColorRGBA(r, g, b, a)
	Uint8 r
	Uint8 g
	Uint8 b
	Uint8 a
CODE:
	AG_TextColorRGBA(r, g, b, a);

void
BGColorRGB(r, g, b)
	Uint8 r
	Uint8 g
	Uint8 b
CODE:
	AG_TextBGColorRGB(r, g, b);

void
BGColorRGBA(r, g, b, a)
	Uint8 r
	Uint8 g
	Uint8 b
	Uint8 a
CODE:
	AG_TextBGColorRGBA(r, g, b, a);

void
SetFont(font)
	Agar::Font font
CODE:
	AG_TextFont(font);

int
Width(text)
	const char * text
CODE:
	AG_TextSize(text, &RETVAL, NULL);
OUTPUT:
	RETVAL

int
Height(text)
	const char * text
CODE:
	AG_TextSize(text, NULL, &RETVAL);
OUTPUT:
	RETVAL

