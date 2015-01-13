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

static const AP_FlagNames flagNames[] = {
	{ "sticky",		AG_BUTTON_STICKY },
	{ "repeat",		AG_BUTTON_REPEAT },
	{ "invertState",	AG_BUTTON_INVSTATE },
	{ "excl",		AG_BUTTON_EXCL },
	{ NULL,			0 }
};

MODULE = Agar::Button	PACKAGE = Agar::Button	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Button
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Button->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_ButtonNewS(parent, flags, label);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setPadding(self, left, right, top, bottom)
	Agar::Button self
	int left
	int right
	int top
	int bottom
CODE:
	AG_ButtonSetPadding(self, left, right, top, bottom);

void
justify(self, mode)
	Agar::Button self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'L':case 'l':AG_ButtonJustify(self,AG_TEXT_LEFT); break;
		case 'R':case 'r':AG_ButtonJustify(self,AG_TEXT_RIGHT); break;
		case 'C':case 'c':AG_ButtonJustify(self,AG_TEXT_CENTER); break;
		default: Perl_croak(aTHX_ "Invalid justify mode");
	}

void
vAlign(self, mode)
	Agar::Button self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'T':case 't':AG_ButtonValign(self,AG_TEXT_TOP); break;
		case 'M':case 'm':AG_ButtonValign(self,AG_TEXT_MIDDLE); break;
		case 'B':case 'b':AG_ButtonValign(self,AG_TEXT_BOTTOM); break;
		default: Perl_croak(aTHX_ "Invalid vAlign mode");
	}

void
setRepeatMode(self, on)
	Agar::Button self
	int on
CODE:
	AG_ButtonSetRepeatMode(self, on);

void
surface(self, surface)
	Agar::Button self
	Agar::Surface surface
CODE:
	AG_ButtonSurface(self, surface);

void
setFlag(self, name)
	Agar::Button self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Button self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Button self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

