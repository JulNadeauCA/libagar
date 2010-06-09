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
	{ "modal",         AG_WINDOW_MODAL },
	{ "keepAbove",     AG_WINDOW_KEEPABOVE },
	{ "keepBelow",     AG_WINDOW_KEEPBELOW },
	{ "denyFocus",     AG_WINDOW_DENYFOCUS },
	{ "noTitle",       AG_WINDOW_NOTITLE },
	{ "noBorders",     AG_WINDOW_NOBORDERS },
	{ "noHResize",     AG_WINDOW_NOHRESIZE },
	{ "noVResize",     AG_WINDOW_NOVRESIZE },
	{ "noResize",      AG_WINDOW_NORESIZE },
	{ "noClose",       AG_WINDOW_NOCLOSE },
	{ "noMinimize",    AG_WINDOW_NOMINIMIZE },
	{ "noMaximize",    AG_WINDOW_NOMAXIMIZE },
	{ "noBackground",  AG_WINDOW_NOBACKGROUND },
	{ "noUpdateRect",  AG_WINDOW_NOUPDATERECT },
	{ "focusOnAttach", AG_WINDOW_FOCUSONATTACH },
	{ "hMaximize",     AG_WINDOW_HMAXIMIZE },
	{ "vMaximize",     AG_WINDOW_VMAXIMIZE },
	{ "plain",         AG_WINDOW_PLAIN },
	{ NULL,            0 }
};

MODULE = Agar::Window	PACKAGE = Agar::Window	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Window
new(package, ...)
	const char *package
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 2 && SvTYPE(SvRV(ST(1))) != SVt_PVHV) || items > 2) {
		Perl_croak(aTHX_ "Usage: Agar::Window->new([{opts}])");
	}
	if (items == 2) {
		AP_MapHashToFlags(SvRV(ST(1)), flagNames, &flags);
	}
	RETVAL = AG_WindowNew(flags);
OUTPUT:
	RETVAL

Agar::Window
newNamed(package, name, ...)
	const char *package
	const char *name
PREINIT:
	Uint flags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Window->newNamed(name,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), flagNames, &flags);
	}
	if ((RETVAL = AG_WindowNewNamedS(flags, name)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

SV *
caption(win, ...)
	Agar::Window win
CODE:
	if (items > 1) {
		AG_WindowSetCaptionS(win, (char *)SvPV_nolen(ST(1)));
	}
	RETVAL = newSVpv(win->caption,0);
OUTPUT:
	RETVAL

Agar::Surface
icon(win, ...)
	Agar::Window win
CODE:
	if (items > 1) {
		Agar__Surface s;
		if (sv_derived_from(ST(1), "Agar::Surface")) {
			IV tmp = SvIV((SV*)SvRV(ST(0)));
			s = INT2PTR(Agar__Surface,tmp);
		} else {
	    		Perl_croak(aTHX_ "icon is not of type Agar::Surface");
		}
		AG_WindowSetIcon(win, s);
	}
	RETVAL = AGWIDGET_SURFACE(win->icon,win->icon->surface);
OUTPUT:
	RETVAL

void
setGeometry(win, x, y, w, h)
	Agar::Window win
	int x
	int y
	int w
	int h
CODE:
	AG_WindowSetGeometry(win, x, y, w, h);

void
setMinSize(win, w, h)
	Agar::Window win
	int w
	int h
CODE:
	AG_WindowSetMinSize(win, w, h);

void
show(win)
	Agar::Window win
CODE:
	AG_WindowShow(win);

void
hide(win)
	Agar::Window win
CODE:
	AG_WindowHide(win);

void
draw(win)
	Agar::Window win
CODE:
	AG_WindowDraw(win);

void
attach(win, chld)
	Agar::Window win
	Agar::Window chld
CODE:
	AG_WindowAttach(win, chld);

void
detach(win, chld)
	Agar::Window win
	Agar::Window chld
CODE:
	AG_WindowDetach(win, chld);

Agar::Widget
findFocused(win)
	Agar::Window win
CODE:
	if ((RETVAL = AG_WidgetFindFocused(win)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

void
DESTROY(win)
	Agar::Window win
CODE:
	AG_ObjectDetach(win);

