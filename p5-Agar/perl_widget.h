/*	Public domain	*/

static const AP_FlagNames apWidgetFlagNames[] = {
	{ "focusable",			AG_WIDGET_FOCUSABLE },
	{ "focused",			AG_WIDGET_FOCUSED },
	{ "unfocusedMotion",		AG_WIDGET_UNFOCUSED_MOTION },
	{ "unfocusedButtonUp",		AG_WIDGET_UNFOCUSED_BUTTONUP },
	{ "unfocusedButtonDown",	AG_WIDGET_UNFOCUSED_BUTTONDOWN },
	{ "visible",			AG_WIDGET_VISIBLE },
	{ "hFill",			AG_WIDGET_HFILL },
	{ "vFill",			AG_WIDGET_VFILL },
	{ "useOpenGL",			AG_WIDGET_USE_OPENGL },
	{ "hide",			AG_WIDGET_HIDE },
	{ "disabled",			AG_WIDGET_DISABLED },
	{ "mouseOver",			AG_WIDGET_MOUSEOVER },
	{ "catchTab",			AG_WIDGET_CATCH_TAB },
	{ "glReshape",			AG_WIDGET_GL_RESHAPE },
	{ "undersize",			AG_WIDGET_UNDERSIZE },
	{ "unfocusedKeyDown",		AG_WIDGET_UNFOCUSED_KEYDOWN },
	{ "unfocusedKeyUp",		AG_WIDGET_UNFOCUSED_KEYUP },
	{ "updateWindow",		AG_WIDGET_UPDATE_WINDOW },
	{ "queueSurfaceBackup",		AG_WIDGET_QUEUE_SURFACE_BACKUP },
	{ "useText",			AG_WIDGET_USE_TEXT },
	{ "useMouseOver",		AG_WIDGET_USE_MOUSEOVER },
	{ "expand",			AG_WIDGET_EXPAND },
	{ NULL,				0 }
};

typedef AG_Widget * Agar__Widget;
