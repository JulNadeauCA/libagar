/*	Public domain	*/
/*
 * Miscellaneous mappings for Win32 -> Agar keysym translation.
 */

static AG_KeySym agWindowsKeymap[256];
static const struct ag_windows_key_mapping agWindowsKeyMappings[] = {
	{ '0',			AG_KEY_0 },
	{ '1',			AG_KEY_1 },
	{ '2',			AG_KEY_2 },
	{ '3',			AG_KEY_3 },
	{ '4',			AG_KEY_4 },
	{ '5',			AG_KEY_5 },
	{ '6',			AG_KEY_6 },
	{ '7',			AG_KEY_7 },
	{ '8',			AG_KEY_8 },
	{ '9',			AG_KEY_9 },
	{ 'A',			AG_KEY_A },
	{ 'B',			AG_KEY_B },
	{ 'C',			AG_KEY_C },
	{ 'D',			AG_KEY_D },
	{ 'E',			AG_KEY_E },
	{ 'F',			AG_KEY_F },
	{ 'G',			AG_KEY_G },
	{ 'H',			AG_KEY_H },
	{ 'I',			AG_KEY_I },
	{ 'J',			AG_KEY_J },
	{ 'K',			AG_KEY_K },
	{ 'L',			AG_KEY_L },
	{ 'M',			AG_KEY_M },
	{ 'N',			AG_KEY_N },
	{ 'O',			AG_KEY_O },
	{ 'P',			AG_KEY_P },
	{ 'Q',			AG_KEY_Q },
	{ 'R',			AG_KEY_R },
	{ 'S',			AG_KEY_S },
	{ 'T',			AG_KEY_T },
	{ 'U',			AG_KEY_U },
	{ 'V',			AG_KEY_V },
	{ 'W',			AG_KEY_W },
	{ 'X',			AG_KEY_X },
	{ 'Y',			AG_KEY_Y },
	{ 'Z',			AG_KEY_Z },
	{ VK_F1,		AG_KEY_F1 },
	{ VK_F2,		AG_KEY_F2 },
	{ VK_F3,		AG_KEY_F3 },
	{ VK_F4,		AG_KEY_F4 },
	{ VK_F5,		AG_KEY_F5 },
	{ VK_F6,		AG_KEY_F6 },
	{ VK_F7,		AG_KEY_F7 },
	{ VK_F8,		AG_KEY_F8 },
	{ VK_F9,		AG_KEY_F9 },
	{ VK_F10,		AG_KEY_F10 },
	{ VK_F11,		AG_KEY_F11 },
	{ VK_F12,		AG_KEY_F12 },
	{ VK_F13,		AG_KEY_F13 },
	{ VK_F14,		AG_KEY_F14 },
	{ VK_F15,		AG_KEY_F15 },

	{ VK_BACK,		AG_KEY_BACKSPACE },
	{ VK_TAB,		AG_KEY_TAB },
	{ VK_CLEAR,		AG_KEY_CLEAR },
	{ VK_RETURN,		AG_KEY_RETURN },
	{ VK_PAUSE,		AG_KEY_PAUSE },
	{ VK_ESCAPE,		AG_KEY_ESCAPE },
	{ VK_SPACE,		AG_KEY_SPACE },
	{ VK_DELETE,		AG_KEY_DELETE },
	{ VK_UP,		AG_KEY_UP },
	{ VK_DOWN,		AG_KEY_DOWN },
	{ VK_RIGHT,		AG_KEY_RIGHT },
	{ VK_LEFT,		AG_KEY_LEFT },
	{ VK_INSERT,		AG_KEY_INSERT },
	{ VK_HOME,		AG_KEY_HOME },
	{ VK_END,		AG_KEY_END },
	{ VK_PRIOR,		AG_KEY_PAGEUP },
	{ VK_NEXT,		AG_KEY_PAGEDOWN },

	{ VK_NUMPAD0,		AG_KEY_KP0 },
	{ VK_NUMPAD1,		AG_KEY_KP1 },
	{ VK_NUMPAD2,		AG_KEY_KP2 },
	{ VK_NUMPAD3,		AG_KEY_KP3 },
	{ VK_NUMPAD4,		AG_KEY_KP4 },
	{ VK_NUMPAD5,		AG_KEY_KP5 },
	{ VK_NUMPAD6,		AG_KEY_KP6 },
	{ VK_NUMPAD7,		AG_KEY_KP7 },
	{ VK_NUMPAD8,		AG_KEY_KP8 },
	{ VK_NUMPAD9,		AG_KEY_KP9 },
	{ VK_DECIMAL,		AG_KEY_KP_PERIOD },
	{ VK_DIVIDE,		AG_KEY_KP_DIVIDE },
	{ VK_MULTIPLY,		AG_KEY_KP_MULTIPLY },
	{ VK_SUBTRACT,		AG_KEY_KP_MINUS },
	{ VK_ADD,		AG_KEY_KP_PLUS },

	{ VK_NUMLOCK,		AG_KEY_NUMLOCK },
	{ VK_CAPITAL,		AG_KEY_CAPSLOCK },
	{ VK_SCROLL,		AG_KEY_SCROLLOCK },
	{ VK_SHIFT,		AG_KEY_RSHIFT },
	{ VK_RCONTROL,		AG_KEY_RCTRL },
	{ VK_LCONTROL,		AG_KEY_LCTRL },
	{ VK_RMENU,		AG_KEY_RALT },
	{ VK_LMENU,		AG_KEY_LALT },
	{ VK_RWIN,		AG_KEY_RSUPER },
	{ VK_LWIN,		AG_KEY_LSUPER },
	{ VK_HELP,		AG_KEY_HELP },
#ifdef VK_PRINT
	{ VK_PRINT,		AG_KEY_PRINT },
#endif
	{ VK_SNAPSHOT,		AG_KEY_PRINT },
	{ VK_CANCEL,		AG_KEY_BREAK },
	{ VK_APPS,		AG_KEY_MENU },
	{ 0xBA,			AG_KEY_SEMICOLON },
	{ 0xBC,			AG_KEY_COMMA },
	{ 0xBD,			AG_KEY_MINUS },
	{ 0xBE,			AG_KEY_PERIOD },
	{ 0xBF,			AG_KEY_SLASH },
	{ 0xBB,			AG_KEY_EQUALS },
	{ 0xC0,			AG_KEY_BACKQUOTE },
	{ 0xDB,			AG_KEY_LEFTBRACKET },
	{ 0xDC,			AG_KEY_BACKSLASH },
	{ 0xDD,			AG_KEY_RIGHTBRACKET },
	{ 0xDE,			AG_KEY_QUOTE },
	{ 0xDF,			AG_KEY_BACKQUOTE },
	{ 0xE2,			AG_KEY_LESS },
};
static const int agWindowsKeyCount = sizeof(agWindowsKeyMappings) /
                                     sizeof(agWindowsKeyMappings[0]);

/* Initialize keysym translation tables. */
static void
InitKeymaps(void)
{
	int i;

	memset(agWindowsKeymap, 0, sizeof(agWindowsKeymap));
	for (i = 0; i < agWindowsKeyCount; i++)
		agWindowsKeymap[agWindowsKeyMappings[i].kcode] =
		    agWindowsKeyMappings[i].key;
}
