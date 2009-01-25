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
	{ "multiLine",		AG_EDITABLE_MULTILINE },
	{ "static",		AG_EDITABLE_STATIC },
	{ "password",		AG_EDITABLE_PASSWORD },
	{ "abandonFocus",	AG_EDITABLE_ABANDON_FOCUS },
	{ "intOnly",		AG_EDITABLE_INT_ONLY },
	{ "floatOnly",		AG_EDITABLE_FLT_ONLY },
	{ "catchTab",		AG_EDITABLE_CATCH_TAB },
	{ "noScroll",		AG_EDITABLE_NOSCROLL },
	{ "noScrollOnce",	AG_EDITABLE_NOSCROLL_ONCE },
	{ "noEmacs",		AG_EDITABLE_NOEMACS },
	{ "noWordSeek",		AG_EDITABLE_NOWORDSEEK },
	{ "noLatin1",		AG_EDITABLE_NOLATIN1 },
	{ NULL,			0 }
};

MODULE = Agar::Editable		PACKAGE = Agar::Editable	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Editable
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Editable->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_EditableNew(parent, flags);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
sizeHint(self, text)
	Agar::Editable self
	const char * text
CODE:
	AG_EditableSizeHint(self, text);

void
sizeHintPixels(self, w, h)
	Agar::Editable self
	Uint w
	Uint h
CODE:
	AG_EditableSizeHintPixels(self, w, h);

void
sizeHintLines(self, numLines)
	Agar::Editable self
	Uint numLines
CODE:
	AG_EditableSizeHintLines(self, numLines);

int
getCursorPos(self)
	Agar::Editable self
CODE:
	RETVAL = AG_EditableGetCursorPos(self);
OUTPUT:
	RETVAL

int
setCursorPos(self, pos)
	Agar::Editable self
	int pos
CODE:
	RETVAL = AG_EditableSetCursorPos(self, pos);
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Editable self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Editable self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Editable self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

