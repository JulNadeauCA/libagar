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
	{ "multi",		AG_TLIST_MULTI },
	{ "multiToggle",	AG_TLIST_MULTITOGGLE },
	{ "tree",		AG_TLIST_TREE },
	{ NULL,			0 }
};

MODULE = Agar::Tlist	PACKAGE = Agar::Tlist	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Tlist
new(package, parent, ...)
	const char * package
	Agar::Widget parent
PREINIT:
	Uint flags = 0, wflags = 0;
CODE:
	if ((items == 3 && SvTYPE(SvRV(ST(2))) != SVt_PVHV) || items > 3) {
		Perl_croak(aTHX_ "Usage: Agar::Tlist->new(parent,[{opts}])");
	}
	if (items == 3) {
		AP_MapHashToFlags(SvRV(ST(2)), flagNames, &flags);
		AP_MapHashToFlags(SvRV(ST(2)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_TlistNew(parent, flags);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setItemHeight(self, pixels)
	Agar::Tlist self
	int pixels
CODE:
	AG_TlistSetItemHeight(self, pixels);

void
setIcon(self, item, surface)
	Agar::Tlist self
	Agar::TlistItem item
	Agar::Surface surface
CODE:
	AG_TlistSetIcon(self, item, AG_SurfaceDup(surface));

void
sizeHint(self, text, numItems)
	Agar::Tlist self
	const char * text
	int numItems
CODE:
	AG_TlistSizeHint(self, text, numItems);

void
sizeHintPixels(self, w, h)
	Agar::Tlist self
	int w
	int h
CODE:
	AG_TlistSizeHintPixels(self, w, h);

void
sizeHintLargest(self, numItems)
	Agar::Tlist self
	int numItems
CODE:
	AG_TlistSizeHintLargest(self, numItems);

Agar::TlistItem
addItem(self, text)
	Agar::Tlist self
	const char * text
CODE:
	RETVAL = AG_TlistAddS(self, NULL, text);
OUTPUT:
	RETVAL

void
delItem(self, item)
	Agar::Tlist self
	Agar::TlistItem item
CODE:
	AG_TlistDel(self, item);

void
beginRebuild(self)
	Agar::Tlist self
CODE:
	AG_TlistBegin(self);

void
endRebuild(self)
	Agar::Tlist self
CODE:
	AG_TlistEnd(self);

void
select(self, item)
	Agar::Tlist self
	Agar::TlistItem item
CODE:
	AG_TlistSelect(self, item);

void
selectAll(self)
	Agar::Tlist self
CODE:
	AG_TlistSelectAll(self);

void
deselect(self, item)
	Agar::Tlist self
	Agar::TlistItem item
CODE:
	AG_TlistDeselect(self, item);

void
deselectAll(self)
	Agar::Tlist self
CODE:
	AG_TlistDeselectAll(self);

Agar::TlistItem
findByIndex(self, index)
	Agar::Tlist self
	int index
CODE:
	RETVAL = AG_TlistFindByIndex(self, index);
OUTPUT:
	RETVAL

Agar::TlistItem
selectedItem(self)
	Agar::Tlist self
CODE:
	RETVAL = AG_TlistSelectedItem(self);
OUTPUT:
	RETVAL

Agar::TlistItem
findText(self, text)
	Agar::Tlist self
	const char * text
CODE:
	RETVAL = AG_TlistFindText(self, text);
OUTPUT:
	RETVAL

void
setFlag(self, name)
	Agar::Tlist self
	const char * name
CODE:
	if (AP_SetNamedFlag(name, flagNames, &(self->flags))) {
		AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

void
unsetFlag(self, name)
	Agar::Tlist self
	const char * name
CODE:
	if (AP_UnsetNamedFlag(name, flagNames, &(self->flags))) {
		AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));
	}

Uint
getFlag(self, name)
	Agar::Tlist self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, flagNames, self->flags, &RETVAL)) {
		if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags,
			&RETVAL)) { XSRETURN_UNDEF; }
	}
OUTPUT:
	RETVAL

MODULE = Agar::Tlist	PACKAGE = Agar::TlistItem	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

int
isSelected(item)
	Agar::TlistItem item
CODE:
	RETVAL = item->selected;
OUTPUT:
	RETVAL

SV *
getText(item)
	Agar::TlistItem item
CODE:
	RETVAL = newSVpv(item->text, 0);
OUTPUT:
	RETVAL

void
setText(item, text)
	Agar::TlistItem item
	const char * text
CODE:
	strncpy(item->text, text, AG_TLIST_LABEL_MAX);

int
getDepth(item)
	Agar::TlistItem item
CODE:
	RETVAL = item->depth;
OUTPUT:
	RETVAL

void
setDepth(item, depth)
	Agar::TlistItem item
	int depth
CODE:
	item->depth = depth;

int
isExpanded(item)
	Agar::TlistItem item
CODE:
	RETVAL = item->flags & AG_TLIST_EXPANDED;
OUTPUT:
	RETVAL

void
setExpanded(item, on)
	Agar::TlistItem item
	int on
CODE:
	if (on) {
		item->flags |= AG_TLIST_EXPANDED;
	} else {
		item->flags &= ~AG_TLIST_EXPANDED;
	}

int
hasChildren(item)
	Agar::TlistItem item
CODE:
	RETVAL = item->flags & AG_TLIST_HAS_CHILDREN;
OUTPUT:
	RETVAL

void
setHasChildren(item, on)
	Agar::TlistItem item
	int on
CODE:
	if (on) {
		item->flags |= AG_TLIST_HAS_CHILDREN;
	} else {
		item->flags &= ~AG_TLIST_HAS_CHILDREN;
	}

void
setNoSelect(item, on)
	Agar::TlistItem item
	int on
CODE:
	if (on) {
		item->flags |= AG_TLIST_NO_SELECT;
	} else {
		item->flags &= ~AG_TLIST_NO_SELECT;
	}

