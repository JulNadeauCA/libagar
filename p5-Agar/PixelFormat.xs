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

MODULE = Agar::PixelFormat	PACKAGE = Agar::PixelFormat	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::PixelFormat
newRGB(package, depth, Rmask, Gmask, Bmask)
	const char *package
	int depth
	Uint32 Rmask
	Uint32 Gmask
	Uint32 Bmask
PREINIT:
	AG_PixelFormat *pf;
CODE:
	pf = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatRGB(pf, depth, Rmask, Gmask, Bmask);
	RETVAL = pf;
OUTPUT:
	RETVAL

Agar::PixelFormat
newRGBA(package, depth, Rmask, Gmask, Bmask, Amask)
	const char *package
	int depth
	Uint32 Rmask
	Uint32 Gmask
	Uint32 Bmask
	Uint32 Amask
PREINIT:
	AG_PixelFormat *pf;
CODE:
	pf = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatRGBA(pf, depth, Rmask, Gmask, Bmask, Amask);
	RETVAL = pf;
OUTPUT:
	RETVAL

Agar::PixelFormat
newIndexed(package, depth)
	const char *package
	int depth
PREINIT:
	AG_PixelFormat *pf;
CODE:
	pf = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatIndexed(pf, depth);
	RETVAL = pf;
OUTPUT:
	RETVAL

Agar::PixelFormat
newGrayscale(package, depth)
	const char *package
	int depth
PREINIT:
	AG_PixelFormat *pf;
CODE:
	pf = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatGrayscale(pf, depth);
	RETVAL = pf;
OUTPUT:
	RETVAL

Agar::PixelFormat
newStandard(package)
	const char *package
CODE:
	RETVAL = AG_PixelFormatDup(agSurfaceFmt);
OUTPUT:
	RETVAL

Uint32
mapRGB(pf, r, g, b)
	Agar::PixelFormat pf
	Uint8 r
	Uint8 g
	Uint8 b
CODE:
	RETVAL = AG_MapPixel32_RGB8(pf, r,g,b);
OUTPUT:
	RETVAL

Uint32
mapRGBA(pf, r, g, b, a)
	Agar::PixelFormat pf
	Uint8 r
	Uint8 g
	Uint8 b
	Uint8 a
CODE:
	RETVAL = AG_MapPixel32_RGBA8(pf, r,g,b,a);
OUTPUT:
	RETVAL

void
getRGB(pf, pixel)
	Agar::PixelFormat pf
	Uint32 pixel
PREINIT:
	Uint8 r, g, b;
PPCODE:
	AG_GetColor32_RGB8(pixel, pf, &r,&g,&b);
	if (GIMME_V == G_SCALAR) {
		XPUSHs(newSVpvf("#%02x%02x%02x", r,g,b));
	} else {
		EXTEND(SP, 3);
		PUSHs(newSViv(r));
		PUSHs(newSViv(g));
		PUSHs(newSViv(b));
	}

void
getRGBA(pf, pixel)
	Agar::PixelFormat pf
	Uint32 pixel
PREINIT:
	Uint8 r, g, b, a;
PPCODE:
	AG_GetColor32_RGBA8(pixel, pf, &r,&g,&b,&a);
	if (GIMME_V == G_SCALAR) {
		XPUSHs(newSVpvf("#%02x%02x%02x%02x", r,g,b,a));
	} else {
		EXTEND(SP, 4);
		PUSHs(newSViv(r));
		PUSHs(newSViv(g));
		PUSHs(newSViv(b));
		PUSHs(newSViv(a));
	}

void
DESTROY(pf)
	Agar::PixelFormat pf
CODE:
	AG_PixelFormatFree(pf);

