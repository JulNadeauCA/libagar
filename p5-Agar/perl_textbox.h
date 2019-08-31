/*	Public domain	*/

static const AP_FlagNames apTextboxFlagNames[] = {
	{ "multiLine",		AG_TEXTBOX_MULTILINE },
	{ "password",		AG_TEXTBOX_PASSWORD },
	{ "abandonFocus",	AG_TEXTBOX_ABANDON_FOCUS },
/*	{ "combo",		AG_TEXTBOX_COMBO }, */
	{ "readOnly",		AG_TEXTBOX_READONLY },
	{ "intOnly",		AG_TEXTBOX_INT_ONLY },
	{ "floatOnly",		AG_TEXTBOX_FLT_ONLY },
	{ "catchTab",		AG_TEXTBOX_CATCH_TAB },
/*	{ "cursorMoving",	AG_TEXTBOX_CURSOR_MOVING }, */
	{ "excl",		AG_TEXTBOX_EXCL },
	{ "noEmacs",		AG_TEXTBOX_NOEMACS },
	{ "noLatin1",		AG_TEXTBOX_NOLATIN1 },
	{ "wordWrap",		AG_TEXTBOX_WORDWRAP },
	{ "noPopup",		AG_TEXTBOX_NOPOPUP },
	{ "multilingual",	AG_TEXTBOX_MULTILINGUAL },
	{ NULL,			0 }
};

typedef AG_Textbox * Agar__Textbox;
