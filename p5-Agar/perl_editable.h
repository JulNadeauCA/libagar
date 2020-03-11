/*	Public domain	*/

static const AP_FlagNames apEditableFlagNames[] = {
	{ "multiLine",		AG_EDITABLE_MULTILINE },
	{ "password",		AG_EDITABLE_PASSWORD },
	{ "abandonFocus",	AG_EDITABLE_ABANDON_FOCUS },
	{ "intOnly",		AG_EDITABLE_INT_ONLY },
	{ "floatOnly",		AG_EDITABLE_FLT_ONLY },
	{ "catchTab",		AG_EDITABLE_CATCH_TAB },
	{ "keepVisCursor",	AG_EDITABLE_KEEPVISCURSOR },
	{ "excl",		AG_EDITABLE_EXCL },
	{ "noKillYank",		AG_EDITABLE_NO_KILL_YANK },
	{ "noAltLatin1",	AG_EDITABLE_NO_ALT_LATIN1 },
	{ "wordWrap",		AG_EDITABLE_WORDWRAP },
	{ "noPopup",		AG_EDITABLE_NOPOPUP },
	{ "wordSelect",		AG_EDITABLE_WORDSELECT },
	{ "readOnly",		AG_EDITABLE_READONLY },
	{ "multilingual",	AG_EDITABLE_MULTILINGUAL },
#ifdef AG_LEGACY
	{ "noEmacs",		AG_EDITABLE_NO_KILL_YANK },
	{ "noLatin1",		AG_EDITABLE_NO_ALT_LATIN1 },
#endif
	{ NULL,			0 }
};

typedef AG_Editable * Agar__Editable;
