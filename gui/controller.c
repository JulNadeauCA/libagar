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
 * Interface to managed controllers (subclass of Joystick).
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>
#include <agar/gui/cursors.h>
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#endif

/*
 * Create and attach a new controller device.
 * The agInputDevices VFS must be locked.
 */
AG_Controller *
AG_ControllerNew(void *obj, const char *desc)
{
	AG_Driver *drv = obj;
	AG_Controller *ctrl;
	AG_Joystick **joysNew;
	AG_InputDevice *idev;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	
	if ((ctrl = TryMalloc(sizeof(AG_Controller))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(ctrl, &agControllerClass);
	idev = AGINPUTDEV(ctrl);
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
	drv->joys[drv->nJoys++] = AGJOYSTICK(ctrl);

	AG_ObjectAttach(&agInputDevices, ctrl);
	return (ctrl);
fail:
	AG_ObjectDestroy(ctrl);
	return (NULL);
}

static __inline__ void
PostCtrl(AG_Widget *_Nonnull wid, AG_InputDevice *_Nonnull id,
    const AG_DriverEvent *dev)
{
	AG_ObjectLock(wid);

	/*
	 * TODO: Allow remapping focus-switching triggers to different
	 *       buttons or axes.
	 */
	if ((wid->flags & AG_WIDGET_CATCH_SHOULDER) == 0) {
		switch (dev->type) {
		case AG_DRIVER_CTRL_BUTTON_DOWN:
			switch (dev->ctrlButton.which) {
			case AG_CTRL_BUTTON_LEFT_SHOULDER:
				AG_WindowCycleFocus(AG_ParentWindow(wid), 1);
				goto out;
			case AG_CTRL_BUTTON_RIGHT_SHOULDER:
				AG_WindowCycleFocus(AG_ParentWindow(wid), 0);
				goto out;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
	if (WIDGET_OPS(wid)->ctrl != NULL)
		WIDGET_OPS(wid)->ctrl(wid, id, dev);
out:
	AG_ObjectUnlock(wid);
}

void
AG_ProcessController(AG_Window *win, const AG_DriverEvent *dev)
{
	AG_Widget *wFoc;
	AG_InputDevice *id;
	int instanceID;

	if ((instanceID = AG_GetJoystickInstanceID(dev)) == -1)
		return;

	AG_LockVFS(&agInputDevices);

	/* Post event to any widget with an active matching input device grab. */
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
			PostCtrl(wGrab, id, dev);
		}
		break;
	}

	/* Post the event to the focused widget in the focused window. */
	if (win != NULL) {
		if (!AG_WindowIsFocused(win) ||
		    (wFoc = AG_WidgetFindFocused(win)) == NULL) {
			goto out;
		}
		PostCtrl(wFoc, id, dev);
	} else {
		AG_Driver *drv;

		AG_LockVFS(&agDrivers);

		OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_FOREACH_WINDOW(win, drv) {
				if (!AG_WindowIsFocused(win) ||
				    (wFoc = AG_WidgetFindFocused(win)) == NULL) {
					continue;
				}
				PostCtrl(wFoc, id, dev);
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
	AG_Controller *ctrl = obj;

	OBJECT(ctrl)->flags |= AG_OBJECT_NAME_ONATTACH;
}

#if defined(AG_WIDGETS) && defined(AG_DEBUG)
static void *
Edit(void *obj)
{
	AG_Controller *ctrl = obj;
	AG_Box *box;
	AG_Label *lbl;

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);

	lbl = AG_LabelNewS(box, 0, AGINPUTDEV(ctrl)->desc);
	AG_SetFontFamily(lbl, "league-spartan");
	AG_SetFontSize(lbl, "130%");

	AG_LabelNew(box, AG_LABEL_HFILL,
	    _("Controller Type: " AGSI_BOLD "%d" AGSI_RST "\n"),
	    ctrl->type);

	return (box);
}
#endif /* AG_WIDGETS and AG_DEBUG */

AG_ObjectClass agControllerClass = {
	"Agar(InputDevice:Controller)",
	sizeof(AG_Controller),
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
