/*	Public domain	*/
/*
 * Miscellaneous mappings for Win32 -> Agar keysym translation.
 */

static AG_KeySym agKeymapMisc[256];
static const struct ag_key_mapping agMiscKeys[] = {
	{ VK_BACK,			0xff,	AG_KEY_BACKSPACE },
	{ VK_TAB,			0xff,	AG_KEY_TAB },
	{ VK_CLEAR,			0xff,	AG_KEY_CLEAR },
	{ VK_RETURN,		0xff,	AG_KEY_RETURN },
	{ VK_PAUSE,			0xff,	AG_KEY_PAUSE },
	{ VK_ESCAPE,		0xff,	AG_KEY_ESCAPE },
	{ VK_DELETE,		0xff,	AG_KEY_DELETE },
	{ VK_NUMPAD0,		0xff,	AG_KEY_KP0 },
	{ VK_NUMPAD1,		0xff,	AG_KEY_KP1 },
	{ VK_NUMPAD2,		0xff,	AG_KEY_KP2 },
	{ VK_NUMPAD3,		0xff,	AG_KEY_KP3 },
	{ VK_NUMPAD4,		0xff,	AG_KEY_KP4 },
	{ VK_NUMPAD5,		0xff,	AG_KEY_KP5 },
	{ VK_NUMPAD6,		0xff,	AG_KEY_KP6 },
	{ VK_NUMPAD7,		0xff,	AG_KEY_KP7 },
	{ VK_NUMPAD8,		0xff,	AG_KEY_KP8 },
	{ VK_NUMPAD9,		0xff,	AG_KEY_KP9 },
#ifdef VK_OEM_PERIOD
	{ VK_OEM_PERIOD,	0xff,	AG_KEY_KP_PERIOD },
#endif
	{ VK_DIVIDE,		0xff,	AG_KEY_KP_DIVIDE },
	{ VK_MULTIPLY,		0xff,	AG_KEY_KP_MULTIPLY },
	{ VK_SUBTRACT,		0xff,	AG_KEY_KP_MINUS },
	{ VK_ADD,			0xff,	AG_KEY_KP_PLUS },
	{ VK_RETURN,		0xff,	AG_KEY_KP_ENTER },
	{ VK_UP,			0xff,	AG_KEY_UP },
	{ VK_DOWN,			0xff,	AG_KEY_DOWN },
	{ VK_RIGHT,			0xff,	AG_KEY_RIGHT },
	{ VK_LEFT,			0xff,	AG_KEY_LEFT },
	{ VK_INSERT,		0xff,	AG_KEY_INSERT },
	{ VK_HOME,			0xff,	AG_KEY_HOME },
	{ VK_END,			0xff,	AG_KEY_END },
	{ VK_PRIOR,			0xff,	AG_KEY_PAGEUP },
	{ VK_NEXT,			0xff,	AG_KEY_PAGEDOWN },
	{ VK_F1,			0xff,	AG_KEY_F1 },
	{ VK_F2,			0xff,	AG_KEY_F2 },
	{ VK_F3,			0xff,	AG_KEY_F3 },
	{ VK_F4,			0xff,	AG_KEY_F4 },
	{ VK_F5,			0xff,	AG_KEY_F5 },
	{ VK_F6,			0xff,	AG_KEY_F6 },
	{ VK_F7,			0xff,	AG_KEY_F7 },
	{ VK_F8,			0xff,	AG_KEY_F8 },
	{ VK_F9,			0xff,	AG_KEY_F9 },
	{ VK_F10,			0xff,	AG_KEY_F10 },
	{ VK_F11,			0xff,	AG_KEY_F11 },
	{ VK_F12,			0xff,	AG_KEY_F12 },
	{ VK_F13,			0xff,	AG_KEY_F13 },
	{ VK_F14,			0xff,	AG_KEY_F14 },
	{ VK_F15,			0xff,	AG_KEY_F15 },
	{ VK_NUMLOCK,		0xff,	AG_KEY_NUMLOCK },
	{ VK_SCROLL,		0xff,	AG_KEY_SCROLLOCK },
	{ VK_RSHIFT,		0xff,	AG_KEY_RSHIFT },
	{ VK_LSHIFT,		0xff,	AG_KEY_LSHIFT },
	{ VK_RCONTROL,		0xff,	AG_KEY_RCTRL },
	{ VK_LCONTROL,		0xff,	AG_KEY_LCTRL },
	{ VK_PRINT,			0xff,	AG_KEY_PRINT }
};
static const int agMiscKeyCount = sizeof(agMiscKeys) / sizeof(agMiscKeys[0]);


/* Initialize keysym translation tables. */
static void
InitKeymaps(void)
{
	int i;

	memset(agKeymapMisc, 0, sizeof(agKeymapMisc));
	for (i = 0; i < agMiscKeyCount; i++)
		agKeymapMisc[agMiscKeys[i].kcode & agMiscKeys[i].kclass] =
		    agMiscKeys[i].key;
}
