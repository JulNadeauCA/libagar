/*
 * Copyright (c) 2008-2009 Julien Nadeau (vedge@hypertriton.com)
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
 *
 * TODO: newFromPixelsRGB, newFromPixelsRGBA, newFromFile, exportFile,
 * blendRGB8, blendRGB8_At, fillRect, get8, put8, mapPixel{32,64}_RGB{8,16},
 * getColor{32,64}, animStateInit, animStateDestroy, animSetLoop, animSetPingPong,
 * animPlay, animStop, addFrame, pixelFormatIsSupported, clipped, mapPixel{32,64},
 * map_pixel_{32,64}_{rgb16,rgba16}, getColor{32,64}_{rgb8,rgb16}, get64_at,
 * get64, put64_at, put64, 
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>
#include "perl_agar.h"

static const AP_FlagNames flagNames[] = {
	{ "srcColorKey", AG_SURFACE_COLORKEY },
	{ "srcAlpha",    AG_SURFACE_ALPHA },
	{ "glTexture",   AG_SURFACE_GL_TEXTURE },
	{ "mapped",      AG_SURFACE_MAPPED },
	{ "static",      AG_SURFACE_STATIC },
	{ "extPixels",   AG_SURFACE_EXT_PIXELS },
	{ "animated",    AG_SURFACE_ANIMATED },
	{ "trace",       AG_SURFACE_TRACE },
	{ NULL,          0 }
};

MODULE = Agar::Surface		PACKAGE = Agar::Surface		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Surface
new(package, w, h, pf, ...)
	const char *package
	int w
	int h
	Agar::PixelFormat pf
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 5 && SvTYPE(SvRV(ST(4))) != SVt_PVHV) || items > 5) {
		Perl_croak(aTHX_ "Usage: Agar::Surface->new(w,h,pxFormat,[{opts}])");
	}
	if (items == 5) {
		AP_MapHashToFlags(SvRV(ST(4)), flagNames, &flags);
	}
	RETVAL = AG_SurfaceNew(pf, w,h, flags);
OUTPUT:
	RETVAL

Agar::Surface
newIndexed(package, w, h, bitsPerPixel, ...)
	const char *package
	int w
	int h
	int bitsPerPixel
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 5 && SvTYPE(SvRV(ST(4))) != SVt_PVHV) || items > 5) {
		Perl_croak(aTHX_ "Usage: Agar::Surface->newIndexed(w,h,depth,"
		           "[{opts}])");
	}
	if (items == 5) {
		AP_MapHashToFlags(SvRV(ST(4)), flagNames, &flags);
	}
	RETVAL = AG_SurfaceIndexed(w,h, bitsPerPixel, flags);
OUTPUT:
	RETVAL

Agar::Surface
newGrayscale(package, w, h, bitsPerPixel, ...)
	const char *package
	int w
	int h
	int bitsPerPixel
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 5 && SvTYPE(SvRV(ST(4))) != SVt_PVHV) || items > 5) {
		Perl_croak(aTHX_ "Usage: Agar::Surface->newGrayscale(w,h,depth,"
		           "[{opts}])");
	}
	if (items == 5) {
		AP_MapHashToFlags(SvRV(ST(4)), flagNames, &flags);
	}
	RETVAL = AG_SurfaceGrayscale(w,h, bitsPerPixel, flags);
OUTPUT:
	RETVAL

Agar::Surface
newEmpty(package)
	const char *package
CODE:
	RETVAL = AG_SurfaceEmpty();
OUTPUT:
	RETVAL

Agar::Surface
newFromBMP(package, path)
	const char *package
	const char *path
CODE:
	if ((RETVAL = AG_SurfaceFromBMP(path)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Surface
newFromPNG(package, path)
	const char *package
	const char *path
CODE:
	if ((RETVAL = AG_SurfaceFromPNG(path)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Surface
newFromJPEG(package, path)
	const char *package
	const char *path
CODE:
	if ((RETVAL = AG_SurfaceFromJPEG(path)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

void
DESTROY(s)
	Agar::Surface s
CODE:
	AG_SurfaceFree(s);

