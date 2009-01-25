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

int nameToColor(const char *name)
{
	int i;
	for (i = 0; i < LAST_COLOR; i++) {
		if (strEQ(name, agColorNames[i])) {
			return i;
		}
	}
	return -1;
}

MODULE = Agar::Colors	PACKAGE = Agar::Colors	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

void
Reset()
CODE:
	AG_ColorsInit();

int
Load(path)
	const char * path
CODE:
	RETVAL = AG_ColorsLoad(path);
OUTPUT:
	RETVAL

int
Save(path)
	const char * path
CODE:
	RETVAL = AG_ColorsSave(path);
OUTPUT:
	RETVAL

void
SaveDefault()
CODE:
	AG_ColorsSaveDefault();

int
SetRGB(name, r, g, b)
	const char * name
	Uint8 r
	Uint8 g
	Uint8 b
PREINIT:
	int color;
CODE:
	if ((color = nameToColor(name)) < 0) {
		RETVAL = -1;
	} else {
		RETVAL = AG_ColorsSetRGB(color, r, g, b);
	}
OUTPUT:
	RETVAL

int
SetRGBA(name, r, g, b, a)
	const char * name
	Uint8 r
	Uint8 g
	Uint8 b
	Uint8 a
PREINIT:
	int color;
CODE:
	if ((color = nameToColor(name)) < 0) {
		RETVAL = -1;
	} else {
		RETVAL = AG_ColorsSetRGBA(color, r, g, b, a);
	}
OUTPUT:
	RETVAL

