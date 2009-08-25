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

int
getBool(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetBool(self, name);
OUTPUT:
	RETVAL

int
getInt(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetInt(self, name);
OUTPUT:
	RETVAL

Uint
getUint(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint(self, name);
OUTPUT:
	RETVAL

Uint8
getUint8(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint8(self, name);
OUTPUT:
	RETVAL

Sint8
getSint8(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetSint8(self, name);
OUTPUT:
	RETVAL

Uint16
getUint16(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint16(self, name);
OUTPUT:
	RETVAL

Sint16
getSint16(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetSint16(self, name);
OUTPUT:
	RETVAL

Uint32
getUint32(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetUint32(self, name);
OUTPUT:
	RETVAL

Sint32
getSint32(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetSint32(self, name);
OUTPUT:
	RETVAL

float
getFloat(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetFloat(self, name);
OUTPUT:
	RETVAL

double
getDouble(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = AG_GetDouble(self, name);
OUTPUT:
	RETVAL

SV *
getString(self, name)
	Agar::Widget self
	const char * name
CODE:
	RETVAL = newSVpv(AG_GetStringDup(self, name), 0);
OUTPUT:
	RETVAL

void
setBool(self, name, val)
	Agar::Widget self
	const char * name
	int val
CODE:
	AG_SetBool(self, name, val);

void
setInt(self, name, val)
	Agar::Widget self
	const char * name
	int val
CODE:
	AG_SetInt(self, name, val);

void
setUint(self, name, val)
	Agar::Widget self
	const char * name
	Uint val
CODE:
	AG_SetUint(self, name, val);

void
setUint8(self, name, val)
	Agar::Widget self
	const char * name
	Uint8 val
CODE:
	AG_SetUint8(self, name, val);

void
setSint8(self, name, val)
	Agar::Widget self
	const char * name
	Sint8 val
CODE:
	AG_SetSint8(self, name, val);

void
setUint16(self, name, val)
	Agar::Widget self
	const char * name
	Uint16 val
CODE:
	AG_SetUint16(self, name, val);

void
setSint16(self, name, val)
	Agar::Widget self
	const char * name
	Sint16 val
CODE:
	AG_SetSint16(self, name, val);

void
setUint32(self, name, val)
	Agar::Widget self
	const char * name
	Uint32 val
CODE:
	AG_SetUint32(self, name, val);

void
setSint32(self, name, val)
	Agar::Widget self
	const char * name
	Sint32 val
CODE:
	AG_SetSint32(self, name, val);

void
setFloat(self, name, val)
	Agar::Widget self
	const char * name
	float val
CODE:
	AG_SetFloat(self, name, val);

void
setDouble(self, name, val)
	Agar::Widget self
	const char * name
	double val
CODE:
	AG_SetDouble(self, name, val);

void
setString(self, name, val)
	Agar::Widget self
	const char * name
	const char * val
CODE:
	AG_SetString(self, name, val);

