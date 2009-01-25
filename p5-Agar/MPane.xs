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
	{ "hFill",		AG_MPANE_HFILL },
	{ "vFill",		AG_MPANE_VFILL },
	{ "frames",		AG_MPANE_FRAMES },
	{ "forceDiv",		AG_MPANE_FORCE_DIV },
	{ NULL,			0 }
};

MODULE = Agar::MPane	PACKAGE = Agar::MPane	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::MPane
new(package, parent, layout, ...)
	const char * package
	Agar::Widget parent
	char * layout
PREINIT:
	Uint flags = 0, wflags = 0;
	enum ag_mpane_layout layout_enum = AG_MPANE4;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::MPane->new(parent,layout,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	if (strlen(layout) > 1) {
		layout[1] = toLOWER(layout[1]);
		if (strlen(layout) > 3) {
			layout[3] = toLOWER(layout[3]);
		}
	}
	if (strEQ(layout, "1"))		{ layout_enum = AG_MPANE1; }
	else if (strEQ(layout, "2v"))	{ layout_enum = AG_MPANE2V; }
	else if (strEQ(layout, "2h"))	{ layout_enum = AG_MPANE2H; }
	else if (strEQ(layout, "2l1r"))	{ layout_enum = AG_MPANE2L1R; }
	else if (strEQ(layout, "1l2r"))	{ layout_enum = AG_MPANE1L2R; }
	else if (strEQ(layout, "2t1b"))	{ layout_enum = AG_MPANE2T1B; }
	else if (strEQ(layout, "1t2b"))	{ layout_enum = AG_MPANE1T2B; }
	else if (strEQ(layout, "3l1r"))	{ layout_enum = AG_MPANE3L1R; }
	else if (strEQ(layout, "1l3r"))	{ layout_enum = AG_MPANE1L3R; }
	else if (strEQ(layout, "3t1b"))	{ layout_enum = AG_MPANE3T1B; }
	else if (strEQ(layout, "1t3b"))	{ layout_enum = AG_MPANE1T3B; }
	RETVAL = AG_MPaneNew(parent, layout_enum, flags);
	AGWIDGET(&(RETVAL->box))->flags |= wflags;
OUTPUT:
	RETVAL

Agar::Box
pane(self, index)
	Agar::MPane self
	Uint index
CODE:
	if (index >= self->npanes) {
		Perl_croak(aTHX_ "Pane index out of bounds");
	}
	RETVAL = self->panes[index];
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::MPane self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

void
unsetFlag(self, name)
	Agar::MPane self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(&self->box)->flags));
	}

Uint
getFlag(self, name)
	Agar::MPane self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(&self->box)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

