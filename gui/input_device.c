/*
 * Copyright (c) 2023 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Abstract class for input devices
 * (mice, keyboards, game controllers, sensors, etc.)
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

#define DEBUG_GRAB

AG_Object agInputDevices;		/* Input devices VFS */

/*
 * Grab an input device. Enable reception of events without focus.
 */
void
AG_GrabInputDevice(void *pWidget, void *pInputDevice)
{
	AG_Widget *wid = pWidget;
	AG_InputDevice *id = pInputDevice;
	int i;

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_OBJECT_ISA(id, "AG_InputDevice:*");

	AG_LockVFS(&agInputDevices);
	for (i = 0; i < id->nWidGrab; i++) {
		if (id->widGrab[i] == wid)
			break;
	}
	if (i == id->nWidGrab) {
		AG_Widget **widGrabNew;

		widGrabNew = TryRealloc(id->widGrab, (id->nWidGrab+1)*sizeof(AG_Widget *));
		if (widGrabNew == NULL) {
			goto out;
		}
		id->widGrab = widGrabNew;
		id->widGrab[id->nWidGrab++] = wid;
#ifdef DEBUG_GRAB
		Debug(id, "Grabbed by %s\n", OBJECT(wid)->name);
	} else {
		Debug(id, "Existing grab by %s\n", OBJECT(wid)->name);
#endif
	}
out:
	AG_UnlockVFS(&agInputDevices);
}

/*
 * Ungrab an input device. Disables reception of events without focus.
 */
void
AG_UngrabInputDevice(void *pWidget, void *pInputDevice)
{
	AG_Widget *wid = pWidget;
	AG_InputDevice *id = pInputDevice;
	int i;

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_OBJECT_ISA(id, "AG_InputDevice:*");

	AG_LockVFS(&agInputDevices);
	for (i = 0; i < id->nWidGrab; i++) {
		if (id->widGrab[i] == wid)
			break;
	}
	if (i < id->nWidGrab) {
		if (i < id->nWidGrab-1) {
			memmove(&id->widGrab[i], &id->widGrab[i+1],
			    (id->nWidGrab-i-1)*sizeof(AG_Widget *));
		}
		id->nWidGrab--;
#ifdef DEBUG_GRAB
		Debug(id, "Ungrabbed by %s\n", OBJECT(wid)->name);
	} else {
		Debug(id, "No grab for %s\n", OBJECT(wid)->name);
#endif
	}
	AG_UnlockVFS(&agInputDevices);
}

static void
Init(void *obj)
{
	AG_InputDevice *id = obj;

	id->drv = NULL;
	id->desc = NULL;
	id->flags = 0;
	id->widGrab = NULL;
	id->nWidGrab = 0;
}

static void
Destroy(void *obj)
{
	AG_InputDevice *id = obj;

	Free(id->desc);
}

AG_ObjectClass agInputDeviceClass = {
	"AG_InputDevice",
	sizeof(AG_InputDevice),
	{ 1,0, AGC_INPUT_DEVICE, 0x1F579 },
	Init,
	NULL,		/* reset */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
