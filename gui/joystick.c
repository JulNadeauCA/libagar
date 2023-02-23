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
 * Interface to joystick / gamepads / accelerometers.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>
#include <agar/gui/cursors.h>
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#endif

/*
 * Create and attach a new joystick device.
 * The agInputDevices VFS must be locked.
 */
AG_Joystick *
AG_JoystickNew(void *obj, const char *desc)
{
	AG_Driver *drv = obj;
	AG_Joystick *joy, **joysNew;
	AG_InputDevice *idev;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	
	if ((joy = TryMalloc(sizeof(AG_Joystick))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(joy, &agJoystickClass);
	idev = AGINPUTDEV(joy);
	idev->drv = drv;
	if ((idev->desc = TryStrdup(desc)) == NULL) {
		goto fail;
	}
	joysNew = TryRealloc(drv->joys, (drv->nJoys + 1)*sizeof(AG_Joystick *));
	if (joysNew == NULL) {
		free(idev->desc);
		goto fail;
	}
	drv->joys = joysNew;
	drv->joys[drv->nJoys++] = joy;

	AG_ObjectAttach(&agInputDevices, joy);
	return (joy);
fail:
	AG_ObjectDestroy(joy);
	return (NULL);
}

void
AG_JoystickDestroy(void *obj, AG_Joystick *joy)
{
	AG_Driver *drv = obj;
	int i;

	AG_OBJECT_ISA(drv, "AG_Driver:*");

	for (i = 0; i < drv->nJoys; i++) {
		if (drv->joys[i] == joy)
			break;
	}
	if (i == drv->nJoys) {
		return;
	}
	if (i < drv->nJoys-1) {
		memmove(&drv->joys[i], &drv->joys[i+1],
		    (drv->nJoys-i-1)*sizeof(AG_Joystick *));
	}
	drv->nJoys--;

	AG_ObjectDestroy(joy);
}

void
AG_ProcessJoystick(AG_Window *win, const AG_DriverEvent *dev)
{
	AG_Widget *wFoc;
	AG_InputDevice *id;
	int instanceID;

	if ((instanceID = AG_GetJoystickInstanceID(dev)) == -1)
		return;

	AG_LockVFS(&agInputDevices);

	OBJECT_FOREACH_CHILD(id, &agInputDevices, ag_input_device) {
		int i;

		if (!AG_OfClass(id, "AG_InputDevice:AG_Joystick:*")) {
			continue;
		}
		if (instanceID != AGJOYSTICK(id)->instanceID) {
			continue;
		}
		for (i = 0; i < id->nWidGrab; i++) {
			AG_Widget *wGrab = id->widGrab[i];

			AG_OBJECT_ISA(wGrab, "AG_Widget:*");
			AG_ObjectLock(wGrab);
			if (WIDGET_OPS(wGrab)->joy != NULL) {
				WIDGET_OPS(wGrab)->joy(wGrab, id, dev);
			}
			AG_ObjectUnlock(wGrab);
		}
		break;
	}

	if (win != NULL) {
		if (!AG_WindowIsFocused(win) ||
		    (wFoc = AG_WidgetFindFocused(win)) == NULL) {
			goto out;
		}
		AG_ObjectLock(wFoc);
		if (WIDGET_OPS(wFoc)->joy != NULL) {
			WIDGET_OPS(wFoc)->joy(wFoc, id, dev);
		}
		AG_ObjectUnlock(wFoc);
	} else {
		AG_Driver *drv;

		AG_LockVFS(&agDrivers);

		OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_FOREACH_WINDOW(win, drv) {
				if (!AG_WindowIsFocused(win) ||
				    (wFoc = AG_WidgetFindFocused(win)) == NULL) {
					continue;
				}
				AG_ObjectLock(wFoc);
				if (WIDGET_OPS(wFoc)->joy != NULL) {
					WIDGET_OPS(wFoc)->joy(wFoc, id, dev);
				}
				AG_ObjectUnlock(wFoc);
			}
		}
		AG_UnlockVFS(&agDrivers);
	}
out:
	AG_UnlockVFS(&agInputDevices);
}

static void
Init(void *_Nonnull obj)
{
	AG_Joystick *joy = obj;

	OBJECT(joy)->flags |= AG_OBJECT_NAME_ONATTACH;
}

#if defined(AG_WIDGETS) && defined(AG_DEBUG)
static void *
Edit(void *obj)
{
	AG_Joystick *joy = obj;
	AG_Box *box;
	AG_Label *lbl;

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);

	lbl = AG_LabelNewS(box, 0, AGINPUTDEV(joy)->desc);
	AG_SetFontFamily(lbl, "league-spartan");
	AG_SetFontSize(lbl, "130%");

	AG_LabelNew(box, AG_LABEL_HFILL,
	    _("Name: " AGSI_BOLD "%s" AGSI_RST "\n"
	      "Vendor: " AGSI_CODE "0x%04x" AGSI_RST " | "
	      "Product: " AGSI_CODE "0x%04x" AGSI_RST "\n"
	      "GUID: " AGSI_CODE "%s" AGSI_RST "\n"
	      "Device Index: %d | Player Index: %d\n"
	      "Instance ID: %d\n\n"
	      "Buttons: " AGSI_BOLD "%u" AGSI_RST " | "
	      "Axes: " AGSI_BOLD "%u" AGSI_RST "\n"
	      "Balls: " AGSI_BOLD "%u" AGSI_RST " | "
	      "Hats: " AGSI_BOLD "%u" AGSI_RST "\n"),
	    joy->name,
	    joy->vendorID, joy->productID,
	    joy->guid,
	    joy->deviceIdx, joy->playerIdx,
	    joy->instanceID,
	    joy->nButtons,
	    joy->nAxes,
	    joy->nBalls,
	    joy->nHats);

	return (box);
}
#endif /* AG_WIDGETS and AG_DEBUG */

AG_ObjectClass agJoystickClass = {
	"Agar(InputDevice:Joystick)",
	sizeof(AG_Joystick),
	{ 0,0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
	Edit
#else
	NULL		/* edit */
#endif
};
