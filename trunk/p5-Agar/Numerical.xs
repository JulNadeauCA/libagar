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

MODULE = Agar::Numerical	PACKAGE = Agar::Numerical	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Numerical
new(package, parent, label, ...)
	const char * package
	Agar::Widget parent
	const char * label
PREINIT:
	Uint wflags = 0;
CODE:
	if ((items == 4 && SvTYPE(SvRV(ST(3))) != SVt_PVHV) || items > 4) {
		Perl_croak(aTHX_ "Usage: Agar::Numerical->new(parent,label,[{opts}])");
	}
	if (items == 4) {
		AP_MapHashToFlags(SvRV(ST(3)), AP_WidgetFlagNames, &wflags);
	}
	RETVAL = AG_NumericalNewS(parent, 0, NULL, label);
	AGWIDGET(RETVAL)->flags |= wflags;
OUTPUT:
	RETVAL

void
setWriteable(self, on)
	Agar::Numerical self
	int on
CODE:
	AG_NumericalSetWriteable(self, on);

void
setRangeInt(self, min, max)
	Agar::Numerical self
	int min
	int max
CODE:
	AG_NumericalSetRangeInt(self, min, max);

void
setRangeFloat(self, min, max)
	Agar::Numerical self
	float min
	float max
CODE:
	AG_NumericalSetRangeFlt(self, min, max);

void
setRangeDouble(self, min, max)
	Agar::Numerical self
	double min
	double max
CODE:
	AG_NumericalSetRangeDbl(self, min, max);

void
setValue(self, value)
	Agar::Numerical self
	double value
CODE:
	AG_NumericalSetValue(self, value);

void
setIncrement(self, step)
	Agar::Numerical self
	double step
CODE:
	AG_NumericalSetIncrement(self, step);

void
setPrecision(self, format, prec)
	Agar::Numerical self
	const char * format
	int prec
CODE:
	AG_NumericalSetPrecision(self, format, prec);

void
setFlag(self, name)
	Agar::Numerical self
	const char * name
CODE:
	AP_SetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));

void
unsetFlag(self, name)
	Agar::Numerical self
	const char * name
CODE:
	AP_UnsetNamedFlag(name, AP_WidgetFlagNames, &(AGWIDGET(self)->flags));

Uint
getFlag(self, name)
	Agar::Numerical self
	const char * name
CODE:
	if (AP_GetNamedFlag(name, AP_WidgetFlagNames, AGWIDGET(self)->flags, &RETVAL))
		{ XSRETURN_UNDEF; }
OUTPUT:
	RETVAL

