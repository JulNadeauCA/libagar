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

MODULE = Agar::Separator	PACKAGE = Agar::Separator	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Separator
newHoriz(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Separator->newHoriz(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_SeparatorNewHoriz(parent);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Separator
newVert(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Separator->newVert(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_SeparatorNewVert(parent);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Separator
newHorizSpacer(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Separator->newHorizSpacer(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_SpacerNewHoriz(parent);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Separator
newVertSpacer(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Separator->newVertSpacer(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_SpacerNewVert(parent);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setPadding(self, pixels)
	Agar::Separator self
	Uint pixels
CODE:
	AG_SeparatorSetPadding(self, pixels);

void
setFlag(self, name)
	Agar::Separator self
	const char * name
CODE:
	AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Separator self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Separator self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL

