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

MODULE = Agar::Config	PACKAGE = Agar::Config	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

void
lock(self)
	Agar::Config self
CODE:
	AG_ObjectLock(self);

void
unlock(self)
	Agar::Config self
CODE:
	AG_ObjectUnlock(self);

int
load(self, path)
	Agar::Config self
	const char * path
PREINIT:
	AG_DataSource * ds;
CODE:
	ds = AG_OpenFile(path, "r");
	if (ds) {
		RETVAL = AG_ObjectLoadVariables(self, ds);
		AG_CloseFile(ds);
	} else {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

int
save(self, path)
	Agar::Config self
	const char * path
PREINIT:
	AG_DataSource * ds;
CODE:
	ds = AG_OpenFile(path, "w");
	if (ds) {
		RETVAL = 0;
		AG_PropSave(self, ds);
		AG_CloseFile(ds);
	} else {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

int
getBool(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetBool(self, name);
OUTPUT:
	RETVAL

int
getInt(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetInt(self, name);
OUTPUT:
	RETVAL

Uint
getUint(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetUint(self, name);
OUTPUT:
	RETVAL

Uint8
getUint8(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetUint8(self, name);
OUTPUT:
	RETVAL

Sint8
getSint8(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetSint8(self, name);
OUTPUT:
	RETVAL

Uint16
getUint16(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetUint16(self, name);
OUTPUT:
	RETVAL

Sint16
getSint16(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetSint16(self, name);
OUTPUT:
	RETVAL

Uint32
getUint32(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetUint32(self, name);
OUTPUT:
	RETVAL

Sint32
getSint32(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetSint32(self, name);
OUTPUT:
	RETVAL

float
getFloat(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetFloat(self, name);
OUTPUT:
	RETVAL

double
getDouble(self, name)
	Agar::Config self
	const char * name
CODE:
	RETVAL = AG_GetDouble(self, name);
OUTPUT:
	RETVAL

SV *
getString(self, name)
	Agar::Config self
	const char * name
PREINIT:
	char * s;
CODE:
	RETVAL = newSVpv(AG_GetStringDup(self, name), 0);
OUTPUT:
	RETVAL

void
setBool(self, name, val)
	Agar::Config self
	const char * name
	int val
CODE:
	AG_SetBool(self, name, val);

void
setInt(self, name, val)
	Agar::Config self
	const char * name
	int val
CODE:
	AG_SetInt(self, name, val);

void
setUint(self, name, val)
	Agar::Config self
	const char * name
	Uint val
CODE:
	AG_SetUint(self, name, val);

void
setUint8(self, name, val)
	Agar::Config self
	const char * name
	Uint8 val
CODE:
	AG_SetUint8(self, name, val);

void
setSint8(self, name, val)
	Agar::Config self
	const char * name
	Sint8 val
CODE:
	AG_SetSint8(self, name, val);

void
setUint16(self, name, val)
	Agar::Config self
	const char * name
	Uint16 val
CODE:
	AG_SetUint16(self, name, val);

void
setSint16(self, name, val)
	Agar::Config self
	const char * name
	Sint16 val
CODE:
	AG_SetSint16(self, name, val);

void
setUint32(self, name, val)
	Agar::Config self
	const char * name
	Uint32 val
CODE:
	AG_SetUint32(self, name, val);

void
setSint32(self, name, val)
	Agar::Config self
	const char * name
	Sint32 val
CODE:
	AG_SetSint32(self, name, val);

void
setFloat(self, name, val)
	Agar::Config self
	const char * name
	float val
CODE:
	AG_SetFloat(self, name, val);

void
setDouble(self, name, val)
	Agar::Config self
	const char * name
	double val
CODE:
	AG_SetDouble(self, name, val);

void
setString(self, name, val)
	Agar::Config self
	const char * name
	const char * val
CODE:
	AG_SetString(self, name, val);

