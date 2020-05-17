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

#ifdef PERL_CAPI
#define WIN32IO_IS_STDIO
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <agar/core.h>
#include <agar/gui.h>

#include "perl_agar.h"

/*
 * Translate a hashref of options to a set of bit flags.
 */
void
AP_MapHashToFlags(void *pHV, const AP_FlagNames *map, Uint *pFlags)
{
	SV **val;
	int i;

	for (i = 0; map[i].name != NULL; i++) {
		val = hv_fetch((HV *)pHV, map[i].name, strlen(map[i].name), 0);
		if (val)
			*pFlags |= map[i].bitmask;
	}
}
int
AP_SetNamedFlag(const char *str, const AP_FlagNames *map, Uint *flags)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flags |= map[i].bitmask;
			return 0;
		}
	}
	return -1;
}
int
AP_UnsetNamedFlag(const char *str, const AP_FlagNames *map, Uint *flags)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		if (strEQ(str, map[i].name)) {
			*flags &= ~(map[i].bitmask);
			return 0;
		}
	}
	return -1;
}
int
AP_GetNamedFlag(const char *str, const AP_FlagNames *map, Uint flags, Uint *flag)
{
	int i;
	for (i = 0; map[i].name != NULL; i++) {
		printf("compare: %s,%s\n", str, map[i].name);
		if (strEQ(str, map[i].name)) {
			*flag = flags & map[i].bitmask;
			return 0;
		}
	}
	return -1;
}

/* Safely retrieves a coderef stored in an event handler. */
SV *
AP_RetrieveEventPV(const AG_Event *event)
{
	if (event == NULL) {
		return NULL;
	}
	if (event->fn == AP_EventHandler ||
	    event->fn == AP_EventHandlerDecRef) {
		return AG_PTR(1);
	}
	return NULL;
}

/* Store a coderef with an event handler and increment its reference count. */
void
AP_StoreEventPV(AG_Event *event, SV *pv)
{
	if (event != NULL) {
		AG_EventPushPointer(event, "perl.pv", pv);
		SvREFCNT_inc(pv);
	}
}

/*
 * A variant of AP_EventHandler() that decrements the reference count of
 * the coderef.
 */
void
AP_EventHandlerDecRef(AG_Event *event)
{
	AP_EventHandler(event);
	AP_DecRefEventPV(event);
}

/* Safely decrements the reference count of an event handler coderef. */
void
AP_DecRefEventPV(AG_Event *event)
{
	SV *pv = AP_RetrieveEventPV(event);
	if (pv != NULL) {
		SvREFCNT_dec(pv);
	}
}

/*
 * An event handler routine that calls the coderef stored in its argv[1].
 */
void
AP_EventHandler(AG_Event *event)
{
	dSP;
	SV *pv;

	if ((pv = AP_RetrieveEventPV(event)) == NULL)
		return;
	ENTER;
	SAVETMPS;

	PUSHMARK(SP);
	XPUSHs(sv_setref_iv(sv_newmortal(), "Agar::Event", PTR2IV(event)));
	PUTBACK;

	call_sv(pv, G_VOID);

	FREETMPS;
	LEAVE;
}
