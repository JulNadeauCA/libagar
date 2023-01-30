/*	Public domain	*/
/*
 * Abstract Input Device Class.
 * Parent class to AG_Mouse and AG_Keyboard.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

#define DEBUG_GRAB

AG_Object agInputDevices;		/* Input devices VFS */

/* Grab an input device (enable reception of events without focus). */
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

/* Ungrab an input device (disables reception of events without focus). */
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
	{ 0,0 },
	Init,
	NULL,		/* reset */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
