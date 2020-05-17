/*	Public domain	*/

#ifndef _AGAR_GUI_KEYBOARD_H_
#define _AGAR_GUI_KEYBOARD_H_
#include <agar/gui/input_device.h>
#include <agar/gui/begin.h>

typedef enum ag_keyboard_action {
	AG_KEY_PRESSED  = 1,
	AG_KEY_RELEASED = 0
} AG_KeyboardAction;

typedef enum ag_key_category {
	AG_KCAT_NONE,			/* No category */
	AG_KCAT_CONTROL,		/* Control character */
	AG_KCAT_SPACING,		/* Whitespace */
	AG_KCAT_RETURN,			/* Return / Line feed */
	AG_KCAT_PRINT,			/* Printable character (not alphanumerici) */
	AG_KCAT_ALPHA,			/* Alphabetic character */
	AG_KCAT_NUMBER,			/* Numerical character */
	AG_KCAT_DIR,			/* Directional keys */
	AG_KCAT_FUNCTION,		/* Function keys */
	AG_KCAT_LOCK,			/* Num/caps/scroll lock */
	AG_KCAT_MODIFIER,		/* Shift/ctrl/alt/meta/super/compose */
} AG_KeyCategory;

typedef enum ag_key_sym {
	AG_KEY_NONE		= 0x0000,	/* Start of ASCII range */
	AG_KEY_ASCII_START	= 0x0000,
/*	AG_KEY_ASCII_SOH	= 0x0001, */
/*	AG_KEY_ASCII_STX	= 0x0002, */
/*	AG_KEY_ASCII_ETX	= 0x0003, */
/*	AG_KEY_ASCII_EOT	= 0x0004, */
/*	AG_KEY_ASCII_ENQ	= 0x0005, */
/*	AG_KEY_ASCII_ACK	= 0x0006, */
/*	AG_KEY_ASCII_BEL	= 0x0007, */
	AG_KEY_BACKSPACE	= 0x0008,
	AG_KEY_TAB		= 0x0009,
/*	AG_KEY_ASCII_NEWLINE	= 0x000a, */
/*	AG_KEY_ASCII_VT		= 0x000b, */
	AG_KEY_CLEAR		= 0x000c,
	AG_KEY_RETURN		= 0x000d,
/*	AG_KEY_ASCII_SO		= 0x000e, */
/*	AG_KEY_ASCII_SI		= 0x000f, */
/*	AG_KEY_ASCII_DLE	= 0x0010, */
/*	AG_KEY_ASCII_DC1	= 0x0011, */
/*	AG_KEY_ASCII_DC2	= 0x0012, */
	AG_KEY_PAUSE		= 0x0013,
/*	AG_KEY_ASCII_DC4	= 0x0014, */
/*	AG_KEY_ASCII_NAK	= 0x0015, */
/*	AG_KEY_ASCII_SYN	= 0x0016, */
/*	AG_KEY_ASCII_ETB	= 0x0017, */
/*	AG_KEY_ASCII_CAN	= 0x0018, */
/*	AG_KEY_ASCII_EM		= 0x0019, */
/*	AG_KEY_ASCII_SUB	= 0x001a, */
	AG_KEY_ESCAPE		= 0x001b,
/*	AG_KEY_ASCII_FS		= 0x001c, */
/*	AG_KEY_ASCII_GS		= 0x001d, */
/*	AG_KEY_ASCII_RS		= 0x001e, */
/*	AG_KEY_ASCII_US		= 0x001f, */
	AG_KEY_SPACE		= 0x0020,
	AG_KEY_EXCLAIM		= 0x0021,	/* ! */
	AG_KEY_QUOTEDBL		= 0x0022,	/* " */
	AG_KEY_HASH		= 0x0023,	/* # */
	AG_KEY_DOLLAR		= 0x0024,	/* $ */
	AG_KEY_PERCENT		= 0x0025,	/* % */
	AG_KEY_AMPERSAND	= 0x0026,	/* & */
	AG_KEY_QUOTE		= 0x0027,	/* ' */
	AG_KEY_LEFTPAREN	= 0x0028,	/* ( */
	AG_KEY_RIGHTPAREN	= 0x0029,	/* ) */
	AG_KEY_ASTERISK		= 0x002a,	/* * */
	AG_KEY_PLUS		= 0x002b,	/* + */
	AG_KEY_COMMA		= 0x002c,	/* , */
	AG_KEY_MINUS		= 0x002d,	/* - */
	AG_KEY_PERIOD		= 0x002e,	/* . */
	AG_KEY_SLASH		= 0x002f,	/* / */
	AG_KEY_0		= 0x0030,	/* 0 */
	AG_KEY_1		= 0x0031,	/* 1 */
	AG_KEY_2		= 0x0032,	/* 2 */
	AG_KEY_3		= 0x0033,	/* 3 */
	AG_KEY_4		= 0x0034,	/* 4 */
	AG_KEY_5		= 0x0035,	/* 5 */
	AG_KEY_6		= 0x0036,	/* 6 */
	AG_KEY_7		= 0x0037,	/* 7 */
	AG_KEY_8		= 0x0038,	/* 8 */
	AG_KEY_9		= 0x0039,	/* 9 */
	AG_KEY_COLON		= 0x003a,	/* : */
	AG_KEY_SEMICOLON	= 0x003b,	/* ; */
	AG_KEY_LESS		= 0x003c,	/* < */
	AG_KEY_EQUALS		= 0x003d,	/* = */
	AG_KEY_GREATER		= 0x003e,	/* > */
	AG_KEY_QUESTION		= 0x003f,	/* ? */
	AG_KEY_AT		= 0x0040,	/* @ */
#if 0
	AG_KEY_UPPER_A		= 0x0041,	/* A */
	AG_KEY_UPPER_B		= 0x0042,	/* B */
	AG_KEY_UPPER_C		= 0x0043,	/* C */
	AG_KEY_UPPER_D		= 0x0044,	/* D */
	AG_KEY_UPPER_E		= 0x0045,	/* E */
	AG_KEY_UPPER_F		= 0x0046,	/* F */
	AG_KEY_UPPER_G		= 0x0047,	/* G */
	AG_KEY_UPPER_H		= 0x0048,	/* H */
	AG_KEY_UPPER_I		= 0x0049,	/* I */
	AG_KEY_UPPER_J		= 0x004a,	/* J */
	AG_KEY_UPPER_K		= 0x004b,	/* K */
	AG_KEY_UPPER_L		= 0x004c,	/* L */
	AG_KEY_UPPER_M		= 0x004d,	/* M */
	AG_KEY_UPPER_N		= 0x004e,	/* N */
	AG_KEY_UPPER_O		= 0x004f,	/* O */
	AG_KEY_UPPER_P		= 0x0050,	/* P */
	AG_KEY_UPPER_Q		= 0x0051,	/* Q */
	AG_KEY_UPPER_R		= 0x0052,	/* R */
	AG_KEY_UPPER_S		= 0x0053,	/* S */
	AG_KEY_UPPER_T		= 0x0054,	/* T */
	AG_KEY_UPPER_U		= 0x0055,	/* U */
	AG_KEY_UPPER_V		= 0x0056,	/* V */
	AG_KEY_UPPER_W		= 0x0057,	/* W */
	AG_KEY_UPPER_X		= 0x0058,	/* X */
	AG_KEY_UPPER_Y		= 0x0059,	/* Y */
	AG_KEY_UPPER_Z		= 0x005a,	/* Z */
#endif
	AG_KEY_LEFTBRACKET	= 0x005b,	/* [ */
	AG_KEY_BACKSLASH	= 0x005c,	/* \ */
	AG_KEY_RIGHTBRACKET	= 0x005d,	/* ] */
	AG_KEY_CARET		= 0x005e,	/* ^ */
	AG_KEY_UNDERSCORE	= 0x005f,	/* _ */
	AG_KEY_BACKQUOTE	= 0x0060,	/* ` */
	AG_KEY_A		= 0x0061,	/* a */
	AG_KEY_B		= 0x0062,	/* b */
	AG_KEY_C		= 0x0063,	/* c */
	AG_KEY_D		= 0x0064,	/* d */
	AG_KEY_E		= 0x0065,	/* e */
	AG_KEY_F		= 0x0066,	/* f */
	AG_KEY_G		= 0x0067,	/* g */
	AG_KEY_H		= 0x0068,	/* h */
	AG_KEY_I		= 0x0069,	/* i */
	AG_KEY_J		= 0x006a,	/* j */
	AG_KEY_K		= 0x006b,	/* k */
	AG_KEY_L		= 0x006c,	/* l */
	AG_KEY_M		= 0x006d,	/* m */
	AG_KEY_N		= 0x006e,	/* n */
	AG_KEY_O		= 0x006f,	/* o */
	AG_KEY_P		= 0x0070,	/* p */
	AG_KEY_Q		= 0x0071,	/* q */
	AG_KEY_R		= 0x0072,	/* r */
	AG_KEY_S		= 0x0073,	/* s */
	AG_KEY_T		= 0x0074,	/* t */
	AG_KEY_U		= 0x0075,	/* u */
	AG_KEY_V		= 0x0076,	/* v */
	AG_KEY_W		= 0x0077,	/* w */
	AG_KEY_X		= 0x0078,	/* x */
	AG_KEY_Y		= 0x0079,	/* y */
	AG_KEY_Z		= 0x007a,	/* z */
	AG_KEY_DELETE		= 0x007f,
	AG_KEY_ASCII_END	= 0x007f,	/* End of ASCII range */
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
	AG_KEY_GRAVE		= 0x0143,
	AG_KEY_KP_CLEAR		= 0x0144,
	AG_KEY_COMMAND		= 0x0145,
	AG_KEY_FUNCTION		= 0x0146,
	AG_KEY_VOLUME_UP	= 0x0147,
	AG_KEY_VOLUME_DOWN	= 0x0148,
	AG_KEY_VOLUME_MUTE	= 0x0149,
	AG_KEY_F16		= 0x014a,
	AG_KEY_F17		= 0x014b,
	AG_KEY_F18		= 0x014c,
	AG_KEY_F19		= 0x014d,
	AG_KEY_F20		= 0x014e,
	AG_KEY_F21		= 0x014f,
	AG_KEY_F22		= 0x0150,
	AG_KEY_F23		= 0x0151,
	AG_KEY_F24		= 0x0152,
	AG_KEY_F25		= 0x0153,
	AG_KEY_F26		= 0x0154,
	AG_KEY_F27		= 0x0155,
	AG_KEY_F28		= 0x0156,
	AG_KEY_F29		= 0x0157,
	AG_KEY_F30		= 0x0158,
	AG_KEY_F31		= 0x0159,
	AG_KEY_F32		= 0x015a,
	AG_KEY_F33		= 0x015b,
	AG_KEY_F34		= 0x015c,
	AG_KEY_F35		= 0x015d,
	AG_KEY_BEGIN		= 0x015e,
	AG_KEY_RESET		= 0x015f,
	AG_KEY_STOP		= 0x0160,
	AG_KEY_USER		= 0x0161,
	AG_KEY_SYSTEM		= 0x0162,
	AG_KEY_PRINT_SCREEN	= 0x0163,
	AG_KEY_CLEAR_LINE	= 0x0164,
	AG_KEY_CLEAR_DISPLAY	= 0x0165,
	AG_KEY_INSERT_LINE	= 0x0166,
	AG_KEY_DELETE_LINE	= 0x0167,
	AG_KEY_INSERT_CHAR	= 0x0168,
	AG_KEY_DELETE_CHAR	= 0x0169,
	AG_KEY_PREV		= 0x016a,
	AG_KEY_NEXT		= 0x016b,
	AG_KEY_SELECT		= 0x016c,
	AG_KEY_EXECUTE		= 0x016d,
	AG_KEY_REDO		= 0x016e,
	AG_KEY_FIND		= 0x016f,
	AG_KEY_MODE_SWITCH	= 0x0170,
	AG_KEY_LAST		= 0x0171,
	AG_KEY_ANY		= 0xffff	/* For matching */
} AG_KeySym;

typedef unsigned int AG_KeyMod;

#define AG_KEYMOD_NONE		0x0000
#define AG_KEYMOD_LSHIFT	0x0001		/* Left Shift */
#define AG_KEYMOD_RSHIFT	0x0002		/* Right Shift */
#define AG_KEYMOD_CTRL_SHIFT	0x0004		/* Ctrl+Shift combined */
#define AG_KEYMOD_CTRL_ALT	0x0008		/* Ctrl+Alt combined */
			/*      0x0010 Reserved */
			/*      0x0020 Reserved */
#define AG_KEYMOD_LCTRL		0x0040		/* Left Ctrl */
#define AG_KEYMOD_RCTRL		0x0080		/* Right Ctrl */
#define AG_KEYMOD_LALT		0x0100		/* Left Alt */
#define AG_KEYMOD_RALT		0x0200		/* Right Alt */
#define AG_KEYMOD_LMETA		0x0400		/* Left Meta */
#define AG_KEYMOD_RMETA		0x0800		/* Right Meta */
#define AG_KEYMOD_NUMLOCK	0x1000		/* Num lock */
#define AG_KEYMOD_CAPSLOCK	0x2000		/* Caps lock */
#define AG_KEYMOD_MODE		0x4000		/* Mode key */
			/*      0x8000 Reserved */
#define AG_KEYMOD_CTRL		(AG_KEYMOD_LCTRL  | AG_KEYMOD_RCTRL)
#define AG_KEYMOD_SHIFT		(AG_KEYMOD_LSHIFT | AG_KEYMOD_RSHIFT)
#define AG_KEYMOD_ALT		(AG_KEYMOD_LALT   | AG_KEYMOD_RALT)
#define AG_KEYMOD_META		(AG_KEYMOD_LMETA  | AG_KEYMOD_RMETA)
#define AG_KEYMOD_ANY		0xffff		/* Any modifier */

struct ag_window;

typedef struct ag_key {
	enum ag_key_sym sym;	/* Translated key */
	int mod;		/* Key modifier */
	AG_Char uch;		/* Corresponding Unicode character */
} AG_Key;

typedef struct ag_keyboard {
	struct ag_input_device _inherit;
	int *_Nonnull keyState;		/* Key state */
	Uint keyCount;
	Uint modState;			/* Modifiers state */
} AG_Keyboard;

__BEGIN_DECLS
extern AG_ObjectClass agKeyboardClass;

extern const char *agKeySyms[];    /* Map AG_KeySym to a string (or NULL) */
extern const int agKeySymCount;

AG_Keyboard *_Nullable AG_KeyboardNew(void *_Nonnull, const char *_Nonnull);

int AG_KeyboardUpdate(AG_Keyboard *_Nonnull, AG_KeyboardAction, AG_KeySym);
int AG_ProcessKey(AG_Keyboard *_Nonnull, struct ag_window *_Nonnull,
                  AG_KeyboardAction, AG_KeySym, AG_Char);

const char *_Nullable AG_LookupKeyName(AG_KeySym) _Pure_Attribute;
AG_KeySym             AG_LookupKeySym(const char *_Nonnull) _Pure_Attribute;

void AG_InitGlobalKeys(void);
void AG_DestroyGlobalKeys(void);
void AG_BindStdGlobalKeys(void);
void AG_BindGlobalKey(AG_KeySym, AG_KeyMod, void (*_Nonnull)(void));
void AG_BindGlobalKeyEv(AG_KeySym, AG_KeyMod, void (*_Nonnull)(AG_Event *_Nonnull));
int  AG_UnbindGlobalKey(AG_KeySym, AG_KeyMod);
void AG_ClearGlobalKeys(void);
int  AG_ExecGlobalKeys(AG_KeySym, AG_KeyMod);
int  AG_CompareKeyMods(Uint, const char *_Nonnull) _Pure_Attribute;
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_KEYBOARD_H_ */
