/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Generic interface to keyboards. Most graphics drivers will register a
 * single keyboard object, but multiple instances are supported.
 */

#include <core/core.h>
#include <core/config.h>
#include "window.h"

AG_Keyboard *
AG_KeyboardNew(void *drv, const char *desc)
{
	AG_Keyboard *kbd;
	
	AG_ASSERT_CLASS(drv, "AG_Driver:*");

	if ((kbd = TryMalloc(sizeof(AG_Keyboard))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(kbd, &agKeyboardClass);
	AGINPUTDEV(kbd)->drv = drv;
	if ((AGINPUTDEV(kbd)->desc = TryStrdup(desc)) == NULL) {
		goto fail;
	}
	AGDRIVER(drv)->kbd = kbd;
	AG_ObjectAttach(&agInputDevices, kbd);
	return (kbd);
fail:
	AG_ObjectDestroy(kbd);
	return (NULL);
}

static void
Init(void *obj)
{
	AG_Keyboard *kbd = obj;

	OBJECT(kbd)->flags |= AG_OBJECT_NAME_ONATTACH;
	kbd->flags = 0;
	kbd->keyCount = AG_KEY_LAST;
	kbd->keyState = Malloc(kbd->keyCount);
	memset(kbd->keyState, 0, kbd->keyCount);
	kbd->modState = AG_KEYMOD_NONE;
}

static void
Destroy(void *obj)
{
	AG_Keyboard *kbd = obj;

	Free(kbd->keyState);
}

/* Update keyboard state following a key press/release event. */
int
AG_KeyboardUpdate(AG_Keyboard *kbd, AG_KeyboardAction action, AG_KeySym ks,
    Uint32 ucs)
{
	Uint ms = kbd->modState;

	/* Update the keyboard state. */
	switch (action) {
	case AG_KEY_PRESSED:
		switch (ks) {
		case AG_KEY_NONE:
			return (0);
		case AG_KEY_NUMLOCK:
			ms ^= AG_KEYMOD_NUMLOCK;
			if ((ms & AG_KEYMOD_NUMLOCK) == 0) {
				action = AG_KEY_RELEASED;
			}
			break;
		case AG_KEY_CAPSLOCK:
			ms ^= AG_KEYMOD_CAPSLOCK;
			if ((ms & AG_KEYMOD_CAPSLOCK) == 0) {
				action = AG_KEY_RELEASED;
			}
			break;
		case AG_KEY_LCTRL:	ms |= AG_KEYMOD_LCTRL;		break;
		case AG_KEY_RCTRL:	ms |= AG_KEYMOD_RCTRL;		break;
		case AG_KEY_LSHIFT:	ms |= AG_KEYMOD_LSHIFT;		break;
		case AG_KEY_RSHIFT:	ms |= AG_KEYMOD_RSHIFT;		break;
		case AG_KEY_LALT:	ms |= AG_KEYMOD_LALT;		break;
		case AG_KEY_RALT:	ms |= AG_KEYMOD_RALT;		break;
		case AG_KEY_LMETA:	ms |= AG_KEYMOD_LMETA;		break;
		case AG_KEY_RMETA:	ms |= AG_KEYMOD_RMETA;		break;
		case AG_KEY_MODE:	ms |= AG_KEYMOD_MODE;		break;
		default:
			break;
		}
		break;
	case AG_KEY_RELEASED:
		switch (ks) {
		case AG_KEY_NONE:
		case AG_KEY_NUMLOCK:
		case AG_KEY_CAPSLOCK:
			return (0);
		case AG_KEY_LCTRL:	ms &= ~AG_KEYMOD_LCTRL;		break;
		case AG_KEY_RCTRL:	ms &= ~AG_KEYMOD_RCTRL;		break;
		case AG_KEY_LSHIFT:	ms &= ~AG_KEYMOD_LSHIFT;	break;
		case AG_KEY_RSHIFT:	ms &= ~AG_KEYMOD_RSHIFT;	break;
		case AG_KEY_LALT:	ms &= ~AG_KEYMOD_LALT;		break;
		case AG_KEY_RALT:	ms &= ~AG_KEYMOD_RALT;		break;
		case AG_KEY_LMETA:	ms &= ~AG_KEYMOD_LMETA;		break;
		case AG_KEY_RMETA:	ms &= ~AG_KEYMOD_RMETA;		break;
		case AG_KEY_MODE:	ms &= ~AG_KEYMOD_MODE;		break;
		default:
			break;
		}
		break;
	default:
		return (0);
	}
	kbd->modState = ms;
	if (kbd->keyState[ks] == (Uint8)action) {
		return (0);
	}
	kbd->keyState[ks] = (Uint8)action;
	return (1);
}

/* Post a key-up event to widgets with the UNFOCUSED_KEYUP flag set. */
static void
PostUnfocusedKeyUp(AG_Widget *wid, AG_KeySym ks, Uint kmod, Uint32 unicode)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_UNFOCUSED_KEYUP) {
		AG_PostEvent(NULL, wid,  "key-up",
		    "%i(key),%i(mod),%lu(unicode)",
		    (int)ks, (int)kmod, (Ulong)unicode);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid,  "window-keyup",
		    "%i,%i,%i",
		    (int)ks, (int)kmod, (int)unicode);
#endif
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		PostUnfocusedKeyUp(cwid, ks, kmod, unicode);
	}
	AG_ObjectUnlock(wid);
}

/* Post a key-down event to widgets with the UNFOCUSED_KEYDOWN flag set. */
static void
PostUnfocusedKeyDown(AG_Widget *wid, AG_KeySym ks, Uint kmod, Uint32 unicode)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_UNFOCUSED_KEYDOWN) {
		AG_PostEvent(NULL, wid,  "key-down",
		    "%i(key),%i(mod),%lu(unicode)",
		    (int)ks, (int)kmod, (Ulong)unicode);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-keydown",
		    "%i,%i,%i",
		    (int)ks, (int)kmod, (int)unicode);
#endif
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		PostUnfocusedKeyDown(cwid, ks, kmod, unicode);
	}
	AG_ObjectUnlock(wid);
}

int
AG_ProcessKey(AG_Keyboard *kbd, AG_Window *win, AG_KeyboardAction action,
    AG_KeySym ks, Uint32 unicode)
{
	AG_Driver *drv = AGINPUTDEV(kbd)->drv;
	AG_Widget *wFoc;
	int tabCycle;
	int rv = 0;

	switch (action) {
	case AG_KEY_RELEASED:
		PostUnfocusedKeyUp(WIDGET(win), ks, kbd->modState, unicode);
		break;
	case AG_KEY_PRESSED:
		if (AG_ExecGlobalKeys(ks, kbd->modState)) {
			return (1);
		}
		PostUnfocusedKeyDown(WIDGET(win), ks, kbd->modState, unicode);
		break;
	}

	/* Ignore modifier key events unless requested otherwise. */
	if (!(win->flags & AG_WINDOW_MODKEYEVENTS)) {
		switch (ks) {
		case AG_KEY_LSHIFT:
		case AG_KEY_RSHIFT:
		case AG_KEY_LALT:
		case AG_KEY_RALT:
		case AG_KEY_LCTRL:
		case AG_KEY_RCTRL:
			return (0);
		default:
			break;
		}
	}

	/* Deliver the event to any focused widget. */
	tabCycle = 1;
	if (AG_WindowIsFocused(win) &&
	   (wFoc = AG_WidgetFindFocused(win)) != NULL) {
		AG_ObjectLock(wFoc);
		if (ks != AG_KEY_TAB || wFoc->flags & AG_WIDGET_CATCH_TAB) {
			if (wFoc->flags & AG_WIDGET_CATCH_TAB) {
				tabCycle = 0;
			}
			AG_PostEvent(NULL, wFoc,
			    (action == AG_KEY_RELEASED) ?
			    "key-up" : "key-down",
			    "%i(key),%i(mod),%lu(unicode)",
			    (int)ks, (int)kbd->modState, (Ulong)unicode);
#ifdef AG_LEGACY
			AG_PostEvent(NULL, wFoc,
			    (action == AG_KEY_RELEASED) ?
			    "window-keyup" : "window-keydown",
			    "%i,%i,%i",
			    (int)ks, (int)kbd->modState, (int)unicode);
#endif
			if (AGDRIVER_SINGLE(drv)) {
				/*
				 * Ensure the keyup event is posted to
				 * this window when the key is released,
				 * in case a keydown event handler
				 * changes the window focus.
				 */
				AGDRIVER_SW(drv)->winLastKeydown = win;
			}
			rv = 1;
		}
		AG_ObjectUnlock(wFoc);
	}

	/* Cycle focus */
	if (tabCycle && ks == AG_KEY_TAB && action == AG_KEY_RELEASED) {
		AG_WindowCycleFocus(win, (kbd->modState & AG_KEYMOD_SHIFT));
		rv = 1;
	}
	return (rv);
}

AG_ObjectClass agKeyboardClass = {
	"Agar(InputDevice:Keyboard)",
	sizeof(AG_Keyboard),
	{ 0,0 },
	Init,
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
