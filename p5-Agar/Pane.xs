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
	{ "div1Fill",		AG_PANE_DIV1FILL },
	{ "forceDiv1Fill",	AG_PANE_FORCE_DIV1FILL },
	{ "frame",		AG_PANE_FRAME },
	{ "div",		AG_PANE_DIV },
	{ "forceDiv",		AG_PANE_FORCE_DIV },
	{ "unmovable",		AG_PANE_UNMOVABLE },
	{ NULL,			0 }
};

MODULE = Agar::Pane	PACKAGE = Agar::Pane	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Pane
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Pane->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_PaneNewHoriz(parent, flags);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Pane
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Pane->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_PaneNewVert(parent, flags);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setDividerWidth(self, pixels)
	Agar::Pane self
	int pixels
CODE:
	AG_PaneSetDividerWidth(self, pixels);

void
setDivisionMin(self, pixels)
	Agar::Pane self
	int pixels
CODE:
	AG_PaneSetDivisionMin(self, 0, pixels, pixels);
	AG_PaneSetDivisionMin(self, 1, pixels, pixels);

Agar::Box
leftPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[0];
OUTPUT:
	RETVAL

Agar::Box
rightPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[1];
OUTPUT:
	RETVAL

Agar::Box
topPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[0];
OUTPUT:
	RETVAL

Agar::Box
bottomPane(self)
	Agar::Pane self
CODE:
	RETVAL = self->div[1];
OUTPUT:
	RETVAL

void
moveDivider(self, x)
	Agar::Pane self
	int x
CODE:
	AG_PaneMoveDivider(self, x);

void
moveDividerPct(self, pct)
	Agar::Pane self
	int pct
CODE:
	AG_PaneMoveDividerPct(self, pct);


void
setFlag(self, name)
	Agar::Pane self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Pane self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Pane self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

