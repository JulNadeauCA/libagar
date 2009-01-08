/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Perl interface to the Agar GUI toolkit.
 */

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>
#include "perl_agar.h"

extern XS(boot_Agar__PixelFormat);
extern XS(boot_Agar__Surface);
extern XS(boot_Agar__Window);

MODULE = Agar		PACKAGE = Agar		PREFIX = AG_
PROTOTYPES: DISABLE

BOOT:
boot_Agar__PixelFormat(aTHX_ cv);
boot_Agar__Surface(aTHX_ cv);
boot_Agar__Window(aTHX_ cv);

SV *
Version()
PREINIT:
	AG_AgarVersion v;
CODE:
	AG_GetVersion(&v);
	RETVAL = newSVpvf("%d.%d.%d", v.major, v.minor, v.patch);
OUTPUT:
	RETVAL

SV *
Release()
PREINIT:
	AG_AgarVersion v;
CODE:
	AG_GetVersion(&v);
	RETVAL = newSVpv(v.release,0);
OUTPUT:
	RETVAL

void
SetError(msg)
	const char *msg
CODE:
	AG_SetError("%s", msg);

SV *
GetError()
CODE:
	RETVAL = newSVpv(AG_GetError(),0);
	SvUTF8_on(RETVAL);
OUTPUT:
	RETVAL

int
InitCore(progName, ...)
	const char *progName
PREINIT:
	const AP_FlagNames flagNames[] = {
		{ "verbose", AG_VERBOSE },
		{ NULL,      0 }
	};
	Uint flags = 0;
CODE:
	if (items == 2) {
		if (SvTYPE(SvRV(ST(1))) != SVt_PVHV) {
			Perl_croak(aTHX_ "Usage: Agar::InitCore(progName,[{opts}])");
		}
		AP_MapHashToFlags(SvRV(ST(1)), flagNames, &flags);
	} else if (items != 1) {
		Perl_croak(aTHX_ "Usage: Agar::InitCore(progName,[{opts}])");
	}
	RETVAL = (AG_InitCore(progName, flags) == 0);
OUTPUT:
	RETVAL

int
InitVideo(w=640, h=480, bitsPerPixel=32, ...)
	int w
	int h
	int bitsPerPixel
PREINIT:
	const AP_FlagNames flagNames[] = {
		{ "hwSurface",   AG_VIDEO_HWSURFACE },
		{ "asyncBlit",   AG_VIDEO_ASYNCBLIT },
		{ "anyFormat",   AG_VIDEO_ANYFORMAT },
		{ "hwPalette",   AG_VIDEO_HWPALETTE },
		{ "doubleBuf",   AG_VIDEO_DOUBLEBUF },
		{ "fullScreen",  AG_VIDEO_FULLSCREEN },
		{ "resizable",   AG_VIDEO_RESIZABLE },
		{ "noFrame",     AG_VIDEO_NOFRAME },
		{ "bgPopupMenu", AG_VIDEO_BGPOPUPMENU },
		{ "openGL",      AG_VIDEO_OPENGL },
		{ "openGLOrSDL", AG_VIDEO_OPENGL_OR_SDL },
		{ "noBgClear",   AG_VIDEO_NOBGCLEAR },
		{ NULL,          0 }
	};
	Uint flags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::InitVideo(w,h,bitsPerPixel,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
	}
	RETVAL = (AG_InitVideo(w, h, bitsPerPixel, flags) == 0);
OUTPUT:
	RETVAL

int
InitVideoSDL(sdl_surface, ...)
	SDL::Surface sdl_surface
PREINIT:
	const AP_FlagNames flagNames[] = {
		{ "hwSurface",   AG_VIDEO_HWSURFACE },
		{ "asyncBlit",   AG_VIDEO_ASYNCBLIT },
		{ "anyFormat",   AG_VIDEO_ANYFORMAT },
		{ "hwPalette",   AG_VIDEO_HWPALETTE },
		{ "doubleBuf",   AG_VIDEO_DOUBLEBUF },
		{ "fullScreen",  AG_VIDEO_FULLSCREEN },
		{ "resizable",   AG_VIDEO_RESIZABLE },
		{ "noFrame",     AG_VIDEO_NOFRAME },
		{ "bgPopupMenu", AG_VIDEO_BGPOPUPMENU },
		{ "openGL",      AG_VIDEO_OPENGL },
		{ "openGLOrSDL", AG_VIDEO_OPENGL_OR_SDL },
		{ "noBgClear",   AG_VIDEO_NOBGCLEAR },
		{ NULL,          0 }
	};
	Uint flags = 0;
CODE:
	if ((items == 2 && SvTYPE(SvRV(ST(1))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::InitVideoSDL(sdl_surface,[{opts}])");
	}
	if (items == 2) {
		AP_MapHashToFlags(SvRV(ST(1)), flagNames, &flags);
	}
	RETVAL = (AG_InitVideoSDL(sdl_surface, flags) == 0);
OUTPUT:
	RETVAL

void
EventLoop()
CODE:
	AG_EventLoop();

void
DESTROY()
CODE:
	AG_Destroy();

