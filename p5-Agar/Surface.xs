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
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>
#include "perl_agar.h"

static const AP_FlagNames flagNames[] = {
	{ "srcColorKey", AG_SRCCOLORKEY },
	{ "srcAlpha",    AG_SRCALPHA },
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
	if (pf->palette == NULL) {
		RETVAL = AG_SurfaceNew(AG_SURFACE_PACKED, w, h, pf, flags);
	} else {
		RETVAL = AG_SurfaceNew(AG_SURFACE_INDEXED, w, h, pf, flags);
	}
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
	RETVAL = AG_SurfaceIndexed(w, h, bitsPerPixel, flags);
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

void
DESTROY(s)
	Agar::Surface s
CODE:
	AG_SurfaceFree(s);

