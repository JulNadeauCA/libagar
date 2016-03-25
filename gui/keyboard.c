/*
 * Copyright (c) 2009-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Generic interface to keyboards.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

static const struct {
	AG_KeySym ks;
	const char *name;
} agKeyNameTbl[] = {
	{ AG_KEY_NONE,		"None"		},
	{ AG_KEY_BACKSPACE,	"BackSpace"	},
	{ AG_KEY_TAB,		"Tab"		},
	{ AG_KEY_CLEAR,		"Clear"		},
	{ AG_KEY_RETURN,	"Return"	},
	{ AG_KEY_PAUSE,		"Pause"		},
	{ AG_KEY_ESCAPE,	"Escape"	},
	{ AG_KEY_SPACE,		"Space"		},
	{ AG_KEY_EXCLAIM,	"Exclaim"	},
	{ AG_KEY_QUOTEDBL,	"QuoteDbl"	},
	{ AG_KEY_HASH,		"Hash"		},
	{ AG_KEY_DOLLAR,	"Dollar"	},
	{ AG_KEY_PERCENT,	"Percent"	},
	{ AG_KEY_AMPERSAND,	"Ampersand"	},
	{ AG_KEY_QUOTE,		"Quote"		},
	{ AG_KEY_LEFTPAREN,	"LeftParen"	},
	{ AG_KEY_RIGHTPAREN,	"RightParen"	},
	{ AG_KEY_ASTERISK,	"Asterisk"	},
	{ AG_KEY_PLUS,		"Plus"		},
	{ AG_KEY_COMMA,		"Comma"		},
	{ AG_KEY_MINUS,		"Minus"		},
	{ AG_KEY_PERIOD,	"Period"	},
	{ AG_KEY_SLASH,		"Slash"		},
	{ AG_KEY_0,		"0"		},
	{ AG_KEY_1,		"1"		},
	{ AG_KEY_2,		"2"		},
	{ AG_KEY_3,		"3"		},
	{ AG_KEY_4,		"4"		},
	{ AG_KEY_5,		"5"		},
	{ AG_KEY_6,		"6"		},
	{ AG_KEY_7,		"7"		},
	{ AG_KEY_8,		"8"		},
	{ AG_KEY_9,		"9"		},
	{ AG_KEY_COLON,		"Colon"		},
	{ AG_KEY_SEMICOLON,	"SemiColon"	},
	{ AG_KEY_LESS,		"Less"		},
	{ AG_KEY_EQUALS,	"Equals"	},
	{ AG_KEY_GREATER,	"Greater"	},
	{ AG_KEY_QUESTION,	"Question"	},
	{ AG_KEY_AT,		"At"		},
	{ AG_KEY_LEFTBRACKET,	"LeftBracket"	},
	{ AG_KEY_BACKSLASH,	"Backslash"	},
	{ AG_KEY_RIGHTBRACKET,	"RightBracket"	},
	{ AG_KEY_CARET,		"Caret"		},
	{ AG_KEY_UNDERSCORE,	"Underscore"	},
	{ AG_KEY_BACKQUOTE,	"Backquote"	},
	{ AG_KEY_A,		"A"		},
	{ AG_KEY_B,		"B"		},
	{ AG_KEY_C,		"C"		},
	{ AG_KEY_D,		"D"		},
	{ AG_KEY_E,		"E"		},
	{ AG_KEY_F,		"F"		},
	{ AG_KEY_G,		"G"		},
	{ AG_KEY_H,		"H"		},
	{ AG_KEY_I,		"I"		},
	{ AG_KEY_J,		"J"		},
	{ AG_KEY_K,		"K"		},
	{ AG_KEY_L,		"L"		},
	{ AG_KEY_M,		"M"		},
	{ AG_KEY_N,		"N"		},
	{ AG_KEY_O,		"O"		},
	{ AG_KEY_P,		"P"		},
	{ AG_KEY_Q,		"Q"		},
	{ AG_KEY_R,		"R"		},
	{ AG_KEY_S,		"S"		},
	{ AG_KEY_T,		"T"		},
	{ AG_KEY_U,		"U"		},
	{ AG_KEY_V,		"V"		},
	{ AG_KEY_W,		"W"		},
	{ AG_KEY_X,		"X"		},
	{ AG_KEY_Y,		"Y"		},
	{ AG_KEY_Z,		"Z"		},
	{ AG_KEY_DELETE,	"Del"		},
	{ AG_KEY_KP0,		"Keypad0"	},
	{ AG_KEY_KP1,		"Keypad1"	},
	{ AG_KEY_KP2,		"Keypad2"	},
	{ AG_KEY_KP3,		"Keypad3"	},
	{ AG_KEY_KP4,		"Keypad4"	},
	{ AG_KEY_KP5,		"Keypad5"	},
	{ AG_KEY_KP6,		"Keypad6"	},
	{ AG_KEY_KP7,		"Keypad7"	},
	{ AG_KEY_KP8,		"Keypad8"	},
	{ AG_KEY_KP9,		"Keypad9"	},
	{ AG_KEY_KP_PERIOD,	"KeypadPeriod"	},
	{ AG_KEY_KP_DIVIDE,	"KeypadDiv"	},
	{ AG_KEY_KP_MULTIPLY,	"KeypadMult"	},
	{ AG_KEY_KP_MINUS,	"KeypadMinus"	},
	{ AG_KEY_KP_PLUS,	"KeypadPlus"	},
	{ AG_KEY_KP_ENTER,	"KeypadEnter"	},
	{ AG_KEY_KP_EQUALS,	"KeypadEquals"	},
	{ AG_KEY_UP,		"Up"		},
	{ AG_KEY_DOWN,		"Down"		},
	{ AG_KEY_RIGHT,		"Right"		},
	{ AG_KEY_LEFT,		"Left"		},
	{ AG_KEY_INSERT,	"Insert"	},
	{ AG_KEY_HOME,		"Home"		},
	{ AG_KEY_END,		"End"		},
	{ AG_KEY_PAGEUP,	"PageUp"	},
	{ AG_KEY_PAGEDOWN,	"PageDown"	},
	{ AG_KEY_F1,		"F1"		},
	{ AG_KEY_F2,		"F2"		},
	{ AG_KEY_F3,		"F3"		},
	{ AG_KEY_F4,		"F4"		},
	{ AG_KEY_F5,		"F5"		},
	{ AG_KEY_F6,		"F6"		},
	{ AG_KEY_F7,		"F7"		},
	{ AG_KEY_F8,		"F8"		},
	{ AG_KEY_F9,		"F9"		},
	{ AG_KEY_F10,		"F10"		},
	{ AG_KEY_F11,		"F11"		},
	{ AG_KEY_F12,		"F12"		},
	{ AG_KEY_F13,		"F13"		},
	{ AG_KEY_F14,		"F14"		},
	{ AG_KEY_F15,		"F15"		},
	{ AG_KEY_F16,		"F16"		},
	{ AG_KEY_F17,		"F17"		},
	{ AG_KEY_F18,		"F18"		},
	{ AG_KEY_F19,		"F19"		},
	{ AG_KEY_F20,		"F20"		},
	{ AG_KEY_F21,		"F21"		},
	{ AG_KEY_F22,		"F22"		},
	{ AG_KEY_F23,		"F23"		},
	{ AG_KEY_F24,		"F24"		},
	{ AG_KEY_F25,		"F25"		},
	{ AG_KEY_F26,		"F26"		},
	{ AG_KEY_F27,		"F27"		},
	{ AG_KEY_F28,		"F28"		},
	{ AG_KEY_F29,		"F29"		},
	{ AG_KEY_F30,		"F30"		},
	{ AG_KEY_F31,		"F31"		},
	{ AG_KEY_F32,		"F32"		},
	{ AG_KEY_F33,		"F33"		},
	{ AG_KEY_F34,		"F34"		},
	{ AG_KEY_F35,		"F35"		},
	{ AG_KEY_NUMLOCK,	"NumLock"	},
	{ AG_KEY_CAPSLOCK,	"CapsLock"	},
	{ AG_KEY_SCROLLOCK,	"ScrollLock"	},
	{ AG_KEY_RSHIFT,	"R-Shift"	},
	{ AG_KEY_LSHIFT,	"L-Shift"	},
	{ AG_KEY_RCTRL,		"R-Ctrl"	},
	{ AG_KEY_LCTRL,		"L-Ctrl"	},
	{ AG_KEY_RALT,		"R-Alt"		},
	{ AG_KEY_LALT,		"L-Alt"		},
	{ AG_KEY_RMETA,		"R-Meta"	},
	{ AG_KEY_LMETA,		"L-Meta"	},
	{ AG_KEY_LSUPER,	"L-Super"	},
	{ AG_KEY_RSUPER,	"R-Super"	},
	{ AG_KEY_MODE,		"Mode"		},
	{ AG_KEY_COMPOSE,	"Compose"	},
	{ AG_KEY_HELP,		"Help"		},
	{ AG_KEY_PRINT,		"Print"		},
	{ AG_KEY_SYSREQ,	"SysReq"	},
	{ AG_KEY_BREAK,		"Break"		},
	{ AG_KEY_MENU,		"Menu"		},
	{ AG_KEY_POWER,		"Power"		},
	{ AG_KEY_EURO,		"Euro"		},
	{ AG_KEY_UNDO,		"Undo"		},
	{ AG_KEY_GRAVE,		"Grave"		},
	{ AG_KEY_KP_CLEAR,	"KeypadClear"	},
	{ AG_KEY_COMMAND,	"Command"	},
	{ AG_KEY_FUNCTION,	"Function"	},
	{ AG_KEY_VOLUME_UP,	"VolumeUp"	},
	{ AG_KEY_VOLUME_DOWN,	"VolumeDown"	},
	{ AG_KEY_VOLUME_MUTE,	"VolumeMute"	},
	{ AG_KEY_BEGIN,		"Begin"		},
	{ AG_KEY_RESET,		"Reset"		},
	{ AG_KEY_STOP,		"Stop"		},
	{ AG_KEY_USER,		"User"		},
	{ AG_KEY_SYSTEM,	"System"	},
	{ AG_KEY_PRINT_SCREEN,	"PrintScreen"	},
	{ AG_KEY_CLEAR_LINE,	"ClearLine"	},
	{ AG_KEY_CLEAR_DISPLAY,	"ClearDisplay"	},
	{ AG_KEY_INSERT_LINE,	"InsertLine"	},
	{ AG_KEY_DELETE_LINE,	"DeleteLine"	},
	{ AG_KEY_INSERT_CHAR,	"InsertChar"	},
	{ AG_KEY_DELETE_CHAR,	"DeleteChar"	},
	{ AG_KEY_PREV,		"Prev"		},
	{ AG_KEY_NEXT,		"Next"		},
	{ AG_KEY_SELECT,	"Select"	},
	{ AG_KEY_EXECUTE,	"Execute"	},
	{ AG_KEY_REDO,		"Redo"		},
	{ AG_KEY_FIND,		"Find"		},
	{ AG_KEY_MODE_SWITCH,	"ModeSwitch"	}
};
static const Uint agKeyNameTblSize = sizeof(agKeyNameTbl) /
                                     sizeof(agKeyNameTbl[0]);

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
	kbd->keyState = Malloc(kbd->keyCount*sizeof(int));
	memset(kbd->keyState, 0, kbd->keyCount*sizeof(int));
	kbd->modState = AG_KEYMOD_NONE;
}

static void
Destroy(void *obj)
{
	AG_Keyboard *kbd = obj;

	Free(kbd->keyState);
}

/*
 * Update Agar's internal keyboard state following a key press/release
 * event. Drivers are only required to update this state for keys in the
 * AG_KEY_ASCII_START to AG_KEY_ASCII_END range, as well as modifier keys.
 */
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
		default:						break;
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
		default:						break;
		}
		break;
	default:
		return (0);
	}
	kbd->modState = ms;
	if (kbd->keyState[ks] == (int)action) {
		return (0);
	}
	kbd->keyState[ks] = (int)action;
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
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		PostUnfocusedKeyDown(cwid, ks, kmod, unicode);
	}
	AG_ObjectUnlock(wid);
}

/*
 * Process a key press or key release action, sending key-down/key-up
 * events to the appropriate Agar widget(s). This function is usually
 * invoked from the ProcessEvent() routine of a driver.
 *
 * Unicode characters without related keysym (or vice-versa) are allowed.
 * The ks argument can be AG_KEY_NONE, or the unicode argument can be 0.
 */
int
AG_ProcessKey(AG_Keyboard *kbd, AG_Window *win, AG_KeyboardAction action,
    AG_KeySym ks, Uint32 unicode)
{
	AG_Driver *drv = AGINPUTDEV(kbd)->drv;
	AG_Widget *wFoc;
	int tabCycle;
	int rv = 0;

#if 0
	const char *keyname;
	if ((keyname = AG_LookupKeyName(ks)) != NULL) {
		printf("Key%s: [%s:0x%x]\n",
		    (action == AG_KEY_PRESSED) ? "Down" : "Up",
		    keyname, (int)unicode);
	} else {
		printf("Key%s: [0x%x:0x%x]\n",
		    (action == AG_KEY_PRESSED) ? "Down" : "Up",
		    (int)ks, (int)unicode);
	}
#endif

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

/* Return a string describing a given Agar keysym. */
const char *
AG_LookupKeyName(AG_KeySym ks)
{
	int i;

	for (i = 0; i < agKeyNameTblSize; i++) {
		if (agKeyNameTbl[i].ks == ks) {
			return (agKeyNameTbl[i].name);
		}
	}
	return (NULL);
}

/* Lookup an Agar keysym by name. */
AG_KeySym
AG_LookupKeySym(const char *name)
{
	int i;

	for (i = 0; i < agKeyNameTblSize; i++) {
		if (strcmp(agKeyNameTbl[i].name,name) == 0) {
			return (agKeyNameTbl[i].ks);
		}
	}
	return (AG_KEY_NONE);
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
