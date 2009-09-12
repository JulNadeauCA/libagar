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
	{ "poll",		AG_COMBO_POLL },
	{ "tree",		AG_COMBO_TREE },
	{ "anyText",		AG_COMBO_ANY_TEXT },
	{ "scrollToSel",	AG_COMBO_SCROLLTOSEL },
	{ NULL,			0 }
};

MODULE = Agar::Combo	PACKAGE = Agar::Combo	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Combo
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Combo->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_ComboNewS(parent, flags, label);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
sizeHint(self, text, numItems)
	Agar::Combo self
	const char * text
	int numItems
CODE:
	AG_ComboSizeHint(self, text, numItems);

void
sizeHintPixels(self, w, h)
	Agar::Combo self
	int w
	int h
CODE:
	AG_ComboSizeHintPixels(self, w, h);

void
select(self, item)
	Agar::Combo self
	Agar::TlistItem item
CODE:
	AG_ComboSelect(self, item);

void
selectText(self, text)
	Agar::Combo self
	const char * text
CODE:
	AG_ComboSelectText(self, text);

Agar::Tlist
list(self)
	Agar::Combo self
CODE:
	RETVAL = self->list;
OUTPUT:
	RETVAL

Agar::Textbox
tbox(self)
	Agar::Combo self
CODE:
	RETVAL = self->tbox;
OUTPUT:
	RETVAL

Agar::Button
button(self)
	Agar::Combo self
CODE:
	RETVAL = self->button;
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Combo self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Combo self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Combo self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

