/*	Public domain	*/

#ifndef _AGAR_GUI_KEYBOARD_H_
#define _AGAR_GUI_KEYBOARD_H_
#include <agar/gui/input_device.h>
#include <agar/gui/begin.h>

typedef enum ag_keyboard_action {
	AG_KEY_PRESSED,
	AG_KEY_RELEASED
} AG_KeyboardAction;

typedef enum ag_key_sym {
	AG_KEY_NONE		= 0x0000,
	AG_KEY_BACKSPACE	= 0x0008,
	AG_KEY_TAB		= 0x0009,
	AG_KEY_CLEAR		= 0x000c,
	AG_KEY_RETURN		= 0x000d,
	AG_KEY_PAUSE		= 0x0013,
	AG_KEY_ESCAPE		= 0x001b,
	AG_KEY_SPACE		= 0x0020,
	AG_KEY_EXCLAIM		= 0x0021,
	AG_KEY_QUOTEDBL		= 0x0022,
	AG_KEY_HASH		= 0x0023,
	AG_KEY_DOLLAR		= 0x0024,
	AG_KEY_AMPERSAND	= 0x0026,
	AG_KEY_QUOTE		= 0x0027,
	AG_KEY_LEFTPAREN	= 0x0028,
	AG_KEY_RIGHTPAREN	= 0x0029,
	AG_KEY_ASTERISK		= 0x002a,
	AG_KEY_PLUS		= 0x002b,
	AG_KEY_COMMA		= 0x002c,
	AG_KEY_MINUS		= 0x002d,
	AG_KEY_PERIOD		= 0x002e,
	AG_KEY_SLASH		= 0x002f,
	AG_KEY_0		= 0x0030,
	AG_KEY_1		= 0x0031,
	AG_KEY_2		= 0x0032,
	AG_KEY_3		= 0x0033,
	AG_KEY_4		= 0x0034,
	AG_KEY_5		= 0x0035,
	AG_KEY_6		= 0x0036,
	AG_KEY_7		= 0x0037,
	AG_KEY_8		= 0x0038,
	AG_KEY_9		= 0x0039,
	AG_KEY_COLON		= 0x003a,
	AG_KEY_SEMICOLON	= 0x003b,
	AG_KEY_LESS		= 0x003c,
	AG_KEY_EQUALS		= 0x003d,
	AG_KEY_GREATER		= 0x003e,
	AG_KEY_QUESTION		= 0x003f,
	AG_KEY_AT		= 0x0040,
	AG_KEY_LEFTBRACKET	= 0x005b,
	AG_KEY_BACKSLASH	= 0x005c,
	AG_KEY_RIGHTBRACKET	= 0x005d,
	AG_KEY_CARET		= 0x005e,
	AG_KEY_UNDERSCORE	= 0x005f,
	AG_KEY_BACKQUOTE	= 0x0060,
	AG_KEY_A		= 0x0061,
	AG_KEY_B		= 0x0062,
	AG_KEY_C		= 0x0063,
	AG_KEY_D		= 0x0064,
	AG_KEY_E		= 0x0065,
	AG_KEY_F		= 0x0066,
	AG_KEY_G		= 0x0067,
	AG_KEY_H		= 0x0068,
	AG_KEY_I		= 0x0069,
	AG_KEY_J		= 0x006a,
	AG_KEY_K		= 0x006b,
	AG_KEY_L		= 0x006c,
	AG_KEY_M		= 0x006d,
	AG_KEY_N		= 0x006e,
	AG_KEY_O		= 0x006f,
	AG_KEY_P		= 0x0070,
	AG_KEY_Q		= 0x0071,
	AG_KEY_R		= 0x0072,
	AG_KEY_S		= 0x0073,
	AG_KEY_T		= 0x0074,
	AG_KEY_U		= 0x0075,
	AG_KEY_V		= 0x0076,
	AG_KEY_W		= 0x0077,
	AG_KEY_X		= 0x0078,
	AG_KEY_Y		= 0x0079,
	AG_KEY_Z		= 0x007a,
	AG_KEY_DELETE		= 0x007f,
	AG_KEY_KP0		= 0x0100,
	AG_KEY_KP1		= 0x0101,
	AG_KEY_KP2		= 0x0102,
	AG_KEY_KP3		= 0x0103,
	AG_KEY_KP4		= 0x0104,
	AG_KEY_KP5		= 0x0105,
	AG_KEY_KP6		= 0x0106,
	AG_KEY_KP7		= 0x0107,
	AG_KEY_KP8		= 0x0108,
	AG_KEY_KP9		= 0x0109,
	AG_KEY_KP_PERIOD	= 0x010a,
	AG_KEY_KP_DIVIDE	= 0x010b,
	AG_KEY_KP_MULTIPLY	= 0x010c,
	AG_KEY_KP_MINUS		= 0x010d,
	AG_KEY_KP_PLUS		= 0x010e,
	AG_KEY_KP_ENTER		= 0x010f,
	AG_KEY_KP_EQUALS	= 0x0110,
	AG_KEY_UP		= 0x0111,
	AG_KEY_DOWN		= 0x0112,
	AG_KEY_RIGHT		= 0x0113,
	AG_KEY_LEFT		= 0x0114,
	AG_KEY_INSERT		= 0x0115,
	AG_KEY_HOME		= 0x0116,
	AG_KEY_END		= 0x0117,
	AG_KEY_PAGEUP		= 0x0118,
	AG_KEY_PAGEDOWN		= 0x0119,
	AG_KEY_F1		= 0x011a,
	AG_KEY_F2		= 0x011b,
	AG_KEY_F3		= 0x011c,
	AG_KEY_F4		= 0x011d,
	AG_KEY_F5		= 0x011e,
	AG_KEY_F6		= 0x011f,
	AG_KEY_F7		= 0x0120,
	AG_KEY_F8		= 0x0121,
	AG_KEY_F9		= 0x0122,
	AG_KEY_F10		= 0x0123,
	AG_KEY_F11		= 0x0124,
	AG_KEY_F12		= 0x0125,
	AG_KEY_F13		= 0x0126,
	AG_KEY_F14		= 0x0127,
	AG_KEY_F15		= 0x0128,
	AG_KEY_NUMLOCK		= 0x012c,
	AG_KEY_CAPSLOCK		= 0x012d,
	AG_KEY_SCROLLOCK	= 0x012e,
	AG_KEY_RSHIFT		= 0x012f,
	AG_KEY_LSHIFT		= 0x0130,
	AG_KEY_RCTRL		= 0x0131,
	AG_KEY_LCTRL		= 0x0132,
	AG_KEY_RALT		= 0x0133,
	AG_KEY_LALT		= 0x0134,
	AG_KEY_RMETA		= 0x0135,
	AG_KEY_LMETA		= 0x0136,
	AG_KEY_LSUPER		= 0x0137,
	AG_KEY_RSUPER		= 0x0138,
	AG_KEY_MODE		= 0x0139,
	AG_KEY_COMPOSE		= 0x013a,
	AG_KEY_HELP		= 0x013b,
	AG_KEY_PRINT		= 0x013c,
	AG_KEY_SYSREQ		= 0x013d,
	AG_KEY_BREAK		= 0x013e,
	AG_KEY_MENU		= 0x013f,
	AG_KEY_POWER		= 0x0140,
	AG_KEY_EURO		= 0x0141,
	AG_KEY_UNDO		= 0x0142,
	AG_KEY_LAST		= 0x0143,
	AG_KEY_ANY		= 0xffff	/* As argument for matching */
} AG_KeySym;

typedef unsigned int AG_KeyMod;

#define AG_KEYMOD_NONE		0x0000
#define AG_KEYMOD_LSHIFT	0x0001
#define AG_KEYMOD_RSHIFT	0x0002
#define AG_KEYMOD_LCTRL		0x0040
#define AG_KEYMOD_RCTRL		0x0080
#define AG_KEYMOD_LALT		0x0100
#define AG_KEYMOD_RALT		0x0200
#define AG_KEYMOD_LMETA		0x0400
#define AG_KEYMOD_RMETA		0x0800
#define AG_KEYMOD_NUMLOCK	0x1000
#define AG_KEYMOD_CAPSLOCK	0x2000
#define AG_KEYMOD_MODE		0x4000
#define AG_KEYMOD_ANY		0xffff		/* As argument for matching */
#define AG_KEYMOD_CTRL		(AG_KEYMOD_LCTRL|AG_KEYMOD_RCTRL)
#define AG_KEYMOD_SHIFT		(AG_KEYMOD_LSHIFT|AG_KEYMOD_RSHIFT)
#define AG_KEYMOD_ALT		(AG_KEYMOD_LALT|AG_KEYMOD_RALT)
#define AG_KEYMOD_META		(AG_KEYMOD_LMETA|AG_KEYMOD_RMETA)

struct ag_window;

typedef struct ag_key {
	enum ag_key_sym sym;	/* Translated key */
	int mod;		/* Key modifier */
	Uint32 uch;		/* Corresponding Unicode character */
} AG_Key;

typedef struct ag_keyboard {
	struct ag_input_device _inherit;
	Uint flags;
	Uint8 *keyState;		/* Key state */
	Uint   keyCount;
	Uint modState;			/* Active modifiers */
} AG_Keyboard;

__BEGIN_DECLS
extern AG_ObjectClass agKeyboardClass;

AG_Keyboard *AG_KeyboardNew(void *, const char *);

int AG_KeyboardUpdate(AG_Keyboard *, AG_KeyboardAction, AG_KeySym, Uint32);
int AG_ProcessKey(AG_Keyboard *, struct ag_window *, AG_KeyboardAction,
                  AG_KeySym, Uint32);

void AG_InitGlobalKeys(void);
void AG_DestroyGlobalKeys(void);
void AG_BindGlobalKey(AG_KeySym, AG_KeyMod, void (*)(void));
void AG_BindGlobalKeyEv(AG_KeySym, AG_KeyMod, void (*)(AG_Event *));
int  AG_UnbindGlobalKey(AG_KeySym, AG_KeyMod);
void AG_ClearGlobalKeys(void);
int  AG_ExecGlobalKeys(AG_KeySym, AG_KeyMod);

/* Key/modifier state access routines. XXX thread unsafe */
static __inline__ Uint8 *
AG_GetKeyState(AG_Keyboard *kbd, int *count)
{
	if (count != NULL) { *count = kbd->keyCount; }
	return (kbd->keyState);
}
static __inline__ Uint
AG_GetModState(AG_Keyboard *kbd)
{
	return (kbd->modState);
}
static __inline__ void
AG_SetModState(AG_Keyboard *kbd, Uint ms)
{
	kbd->modState = ms;
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_KEYBOARD_H_ */
