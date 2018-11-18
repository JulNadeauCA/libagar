/*	Public domain	*/
/*
 * Miscellaneous mappings for X -> Agar keysym translation.
 */

#ifdef XK_MISCELLANY
static AG_KeySym agKeymapMisc[256];
static const struct ag_glx_key_mapping agMiscKeys[] = {
	{ XK_BackSpace,			0xff,	AG_KEY_BACKSPACE },
	{ XK_Tab,			0xff,	AG_KEY_TAB },
	{ XK_Clear,			0xff,	AG_KEY_CLEAR },
	{ XK_Return,			0xff,	AG_KEY_RETURN },
	{ XK_Pause,			0xff,	AG_KEY_PAUSE },
	{ XK_Escape,			0xff,	AG_KEY_ESCAPE },
	{ XK_Delete,			0xff,	AG_KEY_DELETE },
	{ XK_KP_0,			0xff,	AG_KEY_KP0 },
	{ XK_KP_1,			0xff,	AG_KEY_KP1 },
	{ XK_KP_2,			0xff,	AG_KEY_KP2 },
	{ XK_KP_3,			0xff,	AG_KEY_KP3 },
	{ XK_KP_4,			0xff,	AG_KEY_KP4 },
	{ XK_KP_5,			0xff,	AG_KEY_KP5 },
	{ XK_KP_6,			0xff,	AG_KEY_KP6 },
	{ XK_KP_7,			0xff,	AG_KEY_KP7 },
	{ XK_KP_8,			0xff,	AG_KEY_KP8 },
	{ XK_KP_9,			0xff,	AG_KEY_KP9 },
	{ XK_KP_Insert,			0xff,	AG_KEY_KP0 },
	{ XK_KP_End,			0xff,	AG_KEY_KP1 },
	{ XK_KP_Down,			0xff,	AG_KEY_KP2 },
	{ XK_KP_Page_Down,		0xff,	AG_KEY_KP3 },
	{ XK_KP_Left,			0xff,	AG_KEY_KP4 },
	{ XK_KP_Begin,			0xff,	AG_KEY_KP5 },
	{ XK_KP_Right,			0xff,	AG_KEY_KP6 },
	{ XK_KP_Home,			0xff,	AG_KEY_KP7 },
	{ XK_KP_Up,			0xff,	AG_KEY_KP8 },
	{ XK_KP_Page_Up,		0xff,	AG_KEY_KP9 },
	{ XK_KP_Delete,			0xff,	AG_KEY_KP_PERIOD },
	{ XK_KP_Decimal,		0xff,	AG_KEY_KP_PERIOD },
	{ XK_KP_Divide,			0xff,	AG_KEY_KP_DIVIDE },
	{ XK_KP_Multiply,		0xff,	AG_KEY_KP_MULTIPLY },
	{ XK_KP_Subtract,		0xff,	AG_KEY_KP_MINUS },
	{ XK_KP_Add,			0xff,	AG_KEY_KP_PLUS },
	{ XK_KP_Enter,			0xff,	AG_KEY_KP_ENTER },
	{ XK_KP_Equal,			0xff,	AG_KEY_KP_EQUALS },
	{ XK_Up,			0xff,	AG_KEY_UP },
	{ XK_Down,			0xff,	AG_KEY_DOWN },
	{ XK_Right,			0xff,	AG_KEY_RIGHT },
	{ XK_Left,			0xff,	AG_KEY_LEFT },
	{ XK_Insert,			0xff,	AG_KEY_INSERT },
	{ XK_Home,			0xff,	AG_KEY_HOME },
	{ XK_End,			0xff,	AG_KEY_END },
	{ XK_Page_Up,			0xff,	AG_KEY_PAGEUP },
	{ XK_Page_Down,			0xff,	AG_KEY_PAGEDOWN },
	{ XK_F1,			0xff,	AG_KEY_F1 },
	{ XK_F2,			0xff,	AG_KEY_F2 },
	{ XK_F3,			0xff,	AG_KEY_F3 },
	{ XK_F4,			0xff,	AG_KEY_F4 },
	{ XK_F5,			0xff,	AG_KEY_F5 },
	{ XK_F6,			0xff,	AG_KEY_F6 },
	{ XK_F7,			0xff,	AG_KEY_F7 },
	{ XK_F8,			0xff,	AG_KEY_F8 },
	{ XK_F9,			0xff,	AG_KEY_F9 },
	{ XK_F10,			0xff,	AG_KEY_F10 },
	{ XK_F11,			0xff,	AG_KEY_F11 },
	{ XK_F12,			0xff,	AG_KEY_F12 },
	{ XK_F13,			0xff,	AG_KEY_F13 },
	{ XK_F14,			0xff,	AG_KEY_F14 },
	{ XK_F15,			0xff,	AG_KEY_F15 },
	{ XK_Num_Lock,			0xff,	AG_KEY_NUMLOCK },
	{ XK_Caps_Lock,			0xff,	AG_KEY_CAPSLOCK },
	{ XK_Scroll_Lock,		0xff,	AG_KEY_SCROLLOCK },
	{ XK_Shift_R,			0xff,	AG_KEY_RSHIFT },
	{ XK_Shift_L,			0xff,	AG_KEY_LSHIFT },
	{ XK_Control_R,			0xff,	AG_KEY_RCTRL },
	{ XK_Control_L,			0xff,	AG_KEY_LCTRL },
	{ XK_Alt_R,			0xff,	AG_KEY_RALT },
	{ XK_Alt_L,			0xff,	AG_KEY_LALT },
	{ XK_Meta_R,			0xff,	AG_KEY_RMETA },
	{ XK_Meta_L,			0xff,	AG_KEY_LMETA },
	{ XK_Super_L,			0xff,	AG_KEY_LSUPER },
	{ XK_Super_R,			0xff,	AG_KEY_RSUPER },
	{ XK_Mode_switch,		0xff,	AG_KEY_MODE },
	{ XK_Multi_key,			0xff,	AG_KEY_COMPOSE },
	{ XK_Help,			0xff,	AG_KEY_HELP },
	{ XK_Print,			0xff,	AG_KEY_PRINT },
	{ XK_Sys_Req,			0xff,	AG_KEY_SYSREQ },
	{ XK_Break,			0xff,	AG_KEY_BREAK },
	{ XK_Menu,			0xff,	AG_KEY_MENU },
	{ XK_Hyper_R,			0xff,	AG_KEY_MENU },
};
static const int agMiscKeyCount = sizeof(agMiscKeys) / sizeof(agMiscKeys[0]);
#endif /* XK_MISCELLANY */

#ifdef XK_XKB_KEYS
static AG_KeySym agKeymapXKB[256];
static const struct ag_glx_key_mapping agXkbKeys[] = {
	{ XK_dead_grave,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_acute,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_tilde,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_macron,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_breve,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_abovedot,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_diaeresis,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_abovering,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_doubleacute,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_caron,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_cedilla,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_ogonek,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_iota,			0xff,	AG_KEY_COMPOSE },
	{ XK_dead_voiced_sound,		0xff,	AG_KEY_COMPOSE },
	{ XK_dead_semivoiced_sound,	0xff,	AG_KEY_COMPOSE },
	{ XK_dead_belowdot,		0xff,	AG_KEY_COMPOSE },
# ifdef XK_dead_hook
	{ XK_dead_hook,			0xff,	AG_KEY_COMPOSE },
# endif
# ifdef XK_dead_horn
	{ XK_dead_horn,			0xff,	AG_KEY_COMPOSE },
# endif
# ifdef XK_dead_circumflex
	{ XK_dead_circumflex,		0xff,	AG_KEY_CARET },
# endif
# ifdef XK_ISO_Level3_Shift
	{ XK_ISO_Level3_Shift,		0xff,	AG_KEY_MODE },
# endif
};
static const int agXkbKeyCount = sizeof(agXkbKeys) / sizeof(agXkbKeys[0]);
#endif /* XK_XKB_KEYS */

/* Initialize keysym translation tables. */
static void
InitKeymaps(void)
{
#if defined(XK_XKB_KEYS) || defined(XK_MISCELLANY)
	int i;
#endif
#ifdef XK_XKB_KEYS
	memset(agKeymapXKB, 0, sizeof(agKeymapXKB));
	for (i = 0; i < agXkbKeyCount; i++)
		agKeymapXKB[agXkbKeys[i].kcode & agXkbKeys[i].kclass] =
		    agXkbKeys[i].key;
#endif
#ifdef XK_MISCELLANY
	memset(agKeymapMisc, 0, sizeof(agKeymapMisc));
	for (i = 0; i < agMiscKeyCount; i++)
		agKeymapMisc[agMiscKeys[i].kcode & agMiscKeys[i].kclass] =
		    agMiscKeys[i].key;
#endif
}

/*
 * Check to see if this is a repeated key.
 * (stolen from SDL)
 * (idea shamelessly lifted from GII -- thanks guys! :)
 */
static int
IsKeyRepeat(XEvent *_Nonnull xev)
{
	XEvent pev;
	int repeated;

	repeated = 0;
	if (XPending(agDisplay)) {
		XPeekEvent(agDisplay, &pev);
		if ((pev.type == KeyPress) &&
		    (pev.xkey.keycode == xev->xkey.keycode) &&
		    ((pev.xkey.time - xev->xkey.time) < 2)) {
			repeated = 1;
			XNextEvent(agDisplay, &pev);
		}
	}
	return (repeated);
}
