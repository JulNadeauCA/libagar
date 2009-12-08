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

/* How many event args belong to perl? */
static const int apNumOurArgs = 1;

/* Store a coderef with an event handler and increment its reference count. */
void AP_StoreEventPV(AG_Event *event, SV *pv)
{
	if (event != NULL) {
		AG_EventPushPointer(event, "perl.pv", pv);
		SvREFCNT_inc(pv);
	}
}

/* An event handler routine that calls the coderef stored in its argv[1]. */
void AP_EventHandler(AG_Event *event)
{
	dSP;
	SV *pv = AP_RetrieveEventPV(event);
	if (pv == NULL) { return; }

	ENTER;
	SAVETMPS;

	PUSHMARK(SP);
	XPUSHs(sv_setref_iv(sv_newmortal(), "Agar::Event", PTR2IV(event)));
	PUTBACK;

	call_sv(pv, G_VOID);

	FREETMPS;
	LEAVE;
}

/* A variant of the above that decrements the reference count of the coderef. */
void AP_EventHandlerDecRef(AG_Event *event)
{
	AP_EventHandler(event);
	AP_DecRefEventPV(event);
}

/* Safely retrieves a coderef stored in an event handler. */
SV * AP_RetrieveEventPV(AG_Event *event)
{
	if (event == NULL) { return NULL; }
	if (event->handler == AP_EventHandler ||
	    event->handler == AP_EventHandlerDecRef) {
		return AG_PTR(1);
	}
	return NULL;
}

/* Safely decrements the reference count of an event handler coderef. */
void AP_DecRefEventPV(AG_Event *event)
{
	SV *pv = AP_RetrieveEventPV(event);
	if (pv != NULL) {
		SvREFCNT_dec(pv);
	}
}

MODULE = Agar::Event	PACKAGE = Agar::Event	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

const char *
name(event)
	Agar::Event event
CODE:
	RETVAL = event->name;
OUTPUT:
	RETVAL

Agar::Object
receiver(event)
	Agar::Event event
CODE:
	RETVAL = AG_SELF();
OUTPUT:
	RETVAL

Agar::Object
sender(event)
	Agar::Event event
CODE:
	if ((RETVAL = AG_SENDER()) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

IV
ptr(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = PTR2IV(AG_PTR(index));
OUTPUT:
	RETVAL

IV
ptrNamed(event, name)
	Agar::Event event
	const char *name
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	RETVAL = PTR2IV(AG_PTR_NAMED(name));
OUTPUT:
	RETVAL

Agar::Object
object(event, index, classSpec)
	Agar::Event event
	int index
	const char * classSpec
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_POINTER;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_OBJECT(index, classSpec);
OUTPUT:
	RETVAL

SV *
string(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_STRING;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = newSVpv(AG_STRING(index), 0);
OUTPUT:
	RETVAL

int
int(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_INT;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_INT(index);
OUTPUT:
	RETVAL

Uint
Uint(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_UINT;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_UINT(index);
OUTPUT:
	RETVAL

long
long(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_SINT32;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_LONG(index);
OUTPUT:
	RETVAL

Ulong
Ulong(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_UINT32;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_ULONG(index);
OUTPUT:
	RETVAL

float
float(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_FLOAT;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_FLOAT(index);
OUTPUT:
	RETVAL

double
double(event, index)
	Agar::Event event
	int index
PREINIT:
	static const enum ag_variable_type argtype = AG_VARIABLE_DOUBLE;
CODE:
	index += apNumOurArgs;
	if (index <= apNumOurArgs || index >= event->argc ||
	    event->argv[index].type != argtype) {
		XSRETURN_UNDEF;
	}
	RETVAL = AG_DOUBLE(index);
OUTPUT:
	RETVAL

