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

MODULE = Agar::Object	PACKAGE = Agar::Object	PREFIX = AG_
PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

Agar::Event
setEvent(self, event_name, coderef)
	Agar::Object self
	const char * event_name
	SV * coderef
CODE:
	if (SvTYPE(SvRV(coderef)) == SVt_PVCV) {
		AP_DecRefEventPV(AG_FindEventHandler(self, event_name));
		RETVAL = AG_SetEvent(self, event_name, AP_EventHandler, "");
		AP_StoreEventPV(RETVAL, coderef);
	} else {
		Perl_croak(aTHX_ "Usage: $object->setEvent(eventName, codeRef)");
	}
OUTPUT:
	RETVAL

void
unsetEvent(self, event_name)
	Agar::Object self
	const char * event_name
CODE:
	AP_DecRefEventPV(AG_FindEventHandler(self, event_name));
	AG_UnsetEvent(self, event_name);

SV *
findEventHandler(self, event_name)
	Agar::Object self
	const char * event_name
PREINIT:
	AG_Event *event;
CODE:
	event = AG_FindEventHandler(self, event_name);
	if (event == NULL || event->handler == NULL) {
		XSRETURN_UNDEF;
	} else if ((RETVAL = AP_RetrieveEventPV(event)) == NULL) {
		XSRETURN_NO;
	}
OUTPUT:
	RETVAL

void
lock(self)
	Agar::Object self
CODE:
	AG_ObjectLock(self);

void
unlock(self)
	Agar::Object self
CODE:
	AG_ObjectUnlock(self);

void
attachTo(self, newParent)
	Agar::Object self
	Agar::Object newParent
CODE:
	AG_ObjectAttach(newParent, self);

void
detach(self)
	Agar::Object self
CODE:
	AG_ObjectDetach(self);

Agar::Object
root(self)
	Agar::Object self
CODE:
	RETVAL = AG_ObjectRoot(self);
OUTPUT:
	RETVAL

Agar::Object
parent(self)
	Agar::Object self
CODE:
	RETVAL = AG_ObjectParent(self);
OUTPUT:
	RETVAL

Agar::Object
findChild(self, name)
	Agar::Object self
	const char * name
CODE:
	if ((RETVAL = AG_ObjectFindChild(self, name)) == NULL) {
		XSRETURN_UNDEF;
	}
OUTPUT:
	RETVAL

Agar::Config
getProps(self)
	Agar::Object self
CODE:
	RETVAL = self;
OUTPUT:
	RETVAL

void
setName(self, name)
	Agar::Object self
	const char * name
CODE:
	AG_ObjectSetName(self, "%s", name);

SV *
getClassName(self)
	Agar::Object self
CODE:
	RETVAL = newSVpv(AGOBJECT_CLASS(self)->name, 0);
OUTPUT:
	RETVAL

