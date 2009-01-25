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

MODULE = Agar::Scrollbar	PACKAGE = Agar::Scrollbar	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Scrollbar
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Scrollbar->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_ScrollbarNew(parent, AG_SCROLLBAR_HORIZ, 0);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Scrollbar
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Scrollbar->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_ScrollbarNew(parent, AG_SCROLLBAR_VERT, 0);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setIncrementInt(self, step)
	Agar::Scrollbar self
	int step
CODE:
	AG_ScrollbarSetIntIncrement(self, step);

void
setIncrementFloat(self, step)
	Agar::Scrollbar self
	double step
CODE:
	AG_ScrollbarSetRealIncrement(self, step);

void
incAction(self, coderef)
	Agar::Scrollbar self
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AP_DecRefEventPV(self->buttonIncFn);
		AG_ScrollbarSetIncFn(self, AP_EventHandler, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $scrollbar->incAction(codeRef)");
	}

void
decAction(self, coderef)
	Agar::Scrollbar self
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		SvREFCNT_inc(coderef);
		AP_DecRefEventPV(self->buttonDecFn);
		AG_ScrollbarSetDecFn(self, AP_EventHandler, "%p", coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $scrollbar->decAction(codeRef)");
	}

void
setFlag(self, name)
	Agar::Scrollbar self
	const char * name
CODE:
	AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Scrollbar self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Scrollbar self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL

