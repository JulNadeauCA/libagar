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
	{ "frame",		AG_LABEL_FRAME },
	{ "noMinSize",		AG_LABEL_NOMINSIZE },
	{ "partial",		AG_LABEL_PARTIAL },
	{ "regen",		AG_LABEL_REGEN },
	{ NULL,			0 }
};

MODULE = Agar::Label	PACKAGE = Agar::Label	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Label
new(package, parent, text, ...)
	const char * package
	Agar::Widget parent
	const char * text
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Label->new(parent,text,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_LabelNewS(parent, flags, text);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setPadding(self, left, right, top, bottom)
	Agar::Label self
	int left
	int right
	int top
	int bottom
CODE:
	AG_LabelSetPadding(self, left, right, top, bottom);

void
justify(self, mode)
	Agar::Label self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'L': case 'l': AG_LabelJustify(self,AG_TEXT_LEFT); break;
		case 'R': case 'r': AG_LabelJustify(self,AG_TEXT_RIGHT); break;
		case 'C': case 'c': AG_LabelJustify(self,AG_TEXT_CENTER); break;
		default: Perl_croak(aTHX_ "Invalid justify mode");
	}

void
vAlign(self, mode)
	Agar::Label self
	const char * mode
CODE:
	switch (mode[0]) {
		case 'T': case 't': AG_LabelValign(self,AG_TEXT_TOP); break;
		case 'M': case 'm': AG_LabelValign(self,AG_TEXT_MIDDLE); break;
		case 'B': case 'b': AG_LabelValign(self,AG_TEXT_BOTTOM); break;
		default: Perl_croak(aTHX_ "Invalid vAlign mode");
	}

void
sizeHint(self, numLines, text)
	Agar::Label self
	Uint numLines
	const char * text
CODE:
	AG_LabelSizeHint(self, numLines, text);

void
setText(self, text)
	Agar::Label self
	const char * text
CODE:
	AG_LabelTextS(self, text);

void
setFlag(self, name)
	Agar::Label self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Label self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Label self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

