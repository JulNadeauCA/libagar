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
	{ "homogenous",		AG_TOOLBAR_HOMOGENOUS },
	{ "sticky",		AG_TOOLBAR_STICKY },
	{ "multiSticky",	AG_TOOLBAR_MULTI_STICKY },
	{ NULL,			0 }
};

MODULE = Agar::Toolbar		PACKAGE = Agar::Toolbar		PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Toolbar
newHoriz(package, parent, numRows, ...)
	const char * package
	Agar::Widget parent
	int numRows
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Toolbar->newHoriz(parent,rows,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_ToolbarNew(parent, AG_TOOLBAR_HORIZ, numRows, flags);
	AGWIDGET(&(RETVAL->box))->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Toolbar
newVert(package, parent, numRows, ...)
	const char * package
	Agar::Widget parent
	int numRows
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Toolbar->newVert(parent,rows,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_ToolbarNew(parent, AG_TOOLBAR_VERT, numRows, flags);
	AGWIDGET(&(RETVAL->box))->flags |= wflags;
OUTPUT:
	RETVAL

void
setActiveRow(self, row)
	Agar::Toolbar self
	int row
CODE:
	AG_ToolbarRow(self, row);

Agar::Button
addTextButton(self, text)
	Agar::Toolbar self
	const char * text
CODE:
	RETVAL = AG_ToolbarButton(self, text, 0, NULL, "");
OUTPUT:
	RETVAL

Agar::Button
addIconButton(self, surface)
	Agar::Toolbar self
	Agar::Surface surface
CODE:
	RETVAL = AG_ToolbarButtonIcon(self, AG_SurfaceDup(surface), 0, NULL, "");

void
select(self, button)
	Agar::Toolbar self
	Agar::Button button
CODE:
	AG_ToolbarSelect(self, button);

void
deselect(self, button)
	Agar::Toolbar self
	Agar::Button button
CODE:
	AG_ToolbarDeselect(self, button);

void
selectOnly(self, button)
	Agar::Toolbar self
	Agar::Button button
CODE:
	AG_ToolbarSelectOnly(self, button);

void
selectAll(self)
	Agar::Toolbar self
CODE:
	AG_ToolbarSelectAll(self);

void
deselectAll(self)
	Agar::Toolbar self
CODE:
	AG_ToolbarDeselectAll(self);

Agar::Box
getRow(self, index)
	Agar::Toolbar self
	Uint index
CODE:
	if (index >= self->nRows) {
		Perl_croak(aTHX_ "Row index out of bounds");
	}
	RETVAL = self->rows[index];
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Toolbar self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

void
unsetFlag(self, name)
	Agar::Toolbar self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

Uint
getFlag(self, name)
	Agar::Toolbar self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(&self->box)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

