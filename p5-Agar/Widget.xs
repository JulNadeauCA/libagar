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

MODULE = Agar::Widget	PACKAGE = Agar::Widget	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

void
draw(self)
	Agar::Widget self
CODE:
	AG_WidgetDraw(self);

void
enable(self)
	Agar::Widget self
CODE:
	AG_WidgetEnable(self);

void
disable(self)
	Agar::Widget self
CODE:
	AG_WidgetDisable(self);

int
isEnabled(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetEnabled(self);
OUTPUT:
	RETVAL

int
isDisabled(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetDisabled(self);
OUTPUT:
	RETVAL

void
setFocusable(self, isFocusable)
	Agar::Widget self
	int isFocusable
CODE:
	AG_WidgetSetFocusable(self, isFocusable);

int
isFocused(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetIsFocused(self);
OUTPUT:
	RETVAL

int
isFocusedInWindow(self)
	Agar::Widget self
CODE:
	RETVAL = AG_WidgetIsFocusedInWindow(self);
OUTPUT:
	RETVAL

void
focus(self)
	Agar::Widget self
CODE:
	AG_WidgetFocus(self);

void
unfocus(self)
	Agar::Widget self
CODE:
	AG_WidgetUnfocus(self);

Agar::Window
window(self)
	Agar::Widget self
CODE:
	RETVAL = AG_ParentWindow(self);
OUTPUT:
	RETVAL

void
requestSize(self, w, h)
	Agar::Widget self
	int w
	int h
PREINIT:
	static AG_SizeReq sizereq;
CODE:
	sizereq.w = w;
	sizereq.h = h;
	AG_WidgetSizeReq(self, &sizereq);

void
setSize(self, w, h)
	Agar::Widget self
	int w
	int h
CODE:
	AG_WidgetSetSize(self, w, h);

int
x(self)
	Agar::Widget self
CODE:
	RETVAL = self->x;
OUTPUT:
	RETVAL

int
y(self)
	Agar::Widget self
CODE:
	RETVAL = self->y;
OUTPUT:
	RETVAL

int
w(self)
	Agar::Widget self
CODE:
	RETVAL = self->w;
OUTPUT:
	RETVAL

int
h(self)
	Agar::Widget self
CODE:
	RETVAL = self->h;
OUTPUT:
	RETVAL

void
expandHoriz(self)
	Agar::Widget self
CODE:
	AG_ExpandHoriz(self);

void
expandVert(self)
	Agar::Widget self
CODE:
	AG_ExpandVert(self);

void
expand(self)
	Agar::Widget self
CODE:
	AG_Expand(self);

void
redraw(self)
	Agar::Widget self
CODE:
	AG_Redraw(self);

void
redrawOnChange(self, refresh_ms, name)
	Agar::Widget self
	int refresh_ms
	const char *name
CODE:
	AG_RedrawOnChange(self, refresh_ms, name);

void
redrawOnTick(self, refresh_ms)
	Agar::Widget self
	int refresh_ms
CODE:
	AG_RedrawOnTick(self, refresh_ms);

