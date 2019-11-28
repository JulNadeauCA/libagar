/*	Public domain	*/

static const AP_FlagNames agWindowFlagNames[] = {
	{ "modal",         AG_WINDOW_MODAL },
	{ "keepAbove",     AG_WINDOW_KEEPABOVE },
	{ "keepBelow",     AG_WINDOW_KEEPBELOW },
	{ "denyFocus",     AG_WINDOW_DENYFOCUS },
	{ "noTitle",       AG_WINDOW_NOTITLE },
	{ "noBorders",     AG_WINDOW_NOBORDERS },
	{ "noHResize",     AG_WINDOW_NOHRESIZE },
	{ "noVResize",     AG_WINDOW_NOVRESIZE },
	{ "noResize",      AG_WINDOW_NORESIZE },
	{ "noClose",       AG_WINDOW_NOCLOSE },
	{ "noMinimize",    AG_WINDOW_NOMINIMIZE },
	{ "noMaximize",    AG_WINDOW_NOMAXIMIZE },
	{ "noBackground",  AG_WINDOW_NOBACKGROUND },
	{ "main",          AG_WINDOW_MAIN },
	{ "focusOnAttach", AG_WINDOW_FOCUSONATTACH },
	{ "hMaximize",     AG_WINDOW_HMAXIMIZE },
	{ "vMaximize",     AG_WINDOW_VMAXIMIZE },
	{ "noMove",        AG_WINDOW_NOMOVE },
	{ "modKeyEvents",  AG_WINDOW_MODKEYEVENTS },
	{ "noCursorChg",   AG_WINDOW_NOCURSORCHG },
	{ "fadeIn",        AG_WINDOW_FADEIN },
	{ "fadeOut",       AG_WINDOW_FADEOUT },
	/* Shorthands */
	{ "noResize",      AG_WINDOW_NORESIZE },
	{ "noButtons",     AG_WINDOW_NOBUTTONS },
	{ "plain",         AG_WINDOW_PLAIN },
	{ NULL,            0 }
};

typedef AG_Window * Agar__Window;
