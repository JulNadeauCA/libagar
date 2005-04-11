/*	$Csoft: config.c,v 1.138 2005/03/09 06:39:15 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <config/sharedir.h>
#include <config/ttfdir.h>
#include <config/have_getpwuid.h>
#include <config/have_getuid.h>
#include <config/have_freetype.h>

#include <compat/dir.h>

#include <engine/engine.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/map.h>
#include <engine/prop.h>
#include <engine/input.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/tlist.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/hsvpal.h>

#ifdef EDITION
#include <engine/mapedit/mapedit.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif
#include <unistd.h>

const struct version config_ver = {
	"agar config",
	6, 6
};

const struct object_ops config_ops = {
	NULL,
	NULL,
	NULL,
	config_load,
	config_save,
	NULL
};

extern int text_composition;
extern int text_rightleft;
extern int text_tab_width;
extern int window_freescale;
extern int kbd_unitrans;
extern int event_idle;
extern int server_mode;
extern int view_screenshot_quality;

static void
config_set_path(int argc, union evarg *argv)
{
	char path[MAXPATHLEN];
	struct textbox *tbox = argv[0].p;
	char *varname = argv[1].s;

	textbox_copy_string(tbox, path, sizeof(path));
	prop_set_string(config, varname, "%s", path);
	WIDGET(tbox)->flags &= ~(WIDGET_FOCUSED);
}

static void
config_set_full_screen(int argc, union evarg *argv)
{
	int enable = argv[1].i;
	SDL_Event vexp;

	if (view == NULL)
		return;

	if ((enable && (view->v->flags & SDL_FULLSCREEN) == 0) ||
	   (!enable && (view->v->flags & SDL_FULLSCREEN))) {
		SDL_WM_ToggleFullScreen(view->v);
		vexp.type = SDL_VIDEOEXPOSE;
		SDL_PushEvent(&vexp);
	}
}

static void
config_set_opengl(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (enable)
		text_msg(MSG_WARNING,
		    _("Save the configuration and restart %s for OpenGL mode "
		      "to take effect"),
		    progname);
}

static void
config_set_async_blits(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (enable)
		text_msg(MSG_WARNING,
		    _("Save the configuration and restart %s for async blits "
		      "to take effect"),
		    progname);
}

static void
config_set_unitrans(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (SDL_EnableUNICODE(enable)) {
		dprintf("disabled unicode translation\n");
	} else {
		dprintf("enabled unicode translation\n");
	}
}

static void
save_config(int argc, union evarg *argv)
{
	if (object_save(config) == -1)
		text_msg(MSG_ERROR, "%s", error_get());

	text_tmsg(MSG_INFO, 750,
	    _("Configuration settings saved successfully."));
}

void
config_init(struct config *con)
{
	char udatadir[MAXPATHLEN];
	struct passwd *pwd;
	struct stat sta;

	object_init(con, "object", "config", &config_ops);
	OBJECT(con)->flags |= OBJECT_RELOAD_PROPS|OBJECT_DATA_RESIDENT;
	OBJECT(con)->save_pfx = NULL;

	prop_set_bool(con, "initial-run", 1);
	prop_set_bool(con, "view.full-screen", 0);
	prop_set_bool(con, "view.async-blits", 0);
	prop_set_bool(con, "view.opengl", 0);
	prop_set_uint16(con, "view.w", 800);
	prop_set_uint16(con, "view.h", 600);
	prop_set_uint16(con, "view.min-w", 320);
	prop_set_uint16(con, "view.min-h", 240);
	prop_set_uint8(con, "view.depth", 32);
	prop_set_uint8(con, "view.fps", 15);
	prop_set_bool(con, "input.joysticks", 1);

	/* Set the save directory path and create it as needed. */
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	pwd = getpwuid(getuid());
	strlcpy(udatadir, pwd->pw_dir, sizeof(udatadir));
	strlcat(udatadir, "/.", sizeof(udatadir));
	strlcat(udatadir, progname, sizeof(udatadir));
#else
	udatadir[0] = '.';
	strlcpy(&udatadir[1], progname, sizeof(udatadir)-1);
#endif
	if (stat(udatadir, &sta) != 0 &&
	    Mkdir(udatadir) != 0) {
		fatal("%s: %s", udatadir, strerror(errno));
	}
	prop_set_string(con, "save-path", "%s", udatadir);

	prop_set_string(con, "den-path", "%s", SHAREDIR);
	prop_set_string(con, "load-path", "%s:%s", udatadir, SHAREDIR);

#if defined(__APPLE__)
	prop_set_string(con, "font-path", "%s/fonts:%s:%s/Library/Fonts:"
	                                  "/Library/Fonts:"
					  "/System/Library/Fonts",
					  udatadir, TTFDIR, pwd->pw_dir);
#elif defined(WIN32)
	prop_set_string(con, "font-path", "fonts:%s", TTFDIR);
#else
	prop_set_string(con, "font-path", "%s/fonts:%s", udatadir, TTFDIR);
#endif

#ifdef HAVE_FREETYPE
	prop_set_bool(con, "font-engine", 1);
# ifdef __APPLE__
	prop_set_string(con, "font-engine.default-font", "Geneva.dfont");
	prop_set_int(con, "font-engine.default-size", 12);
	prop_set_int(con, "font-engine.default-style", 0);
# else
	prop_set_string(con, "font-engine.default-font", "Vera.ttf");
	prop_set_int(con, "font-engine.default-size", 11);
	prop_set_int(con, "font-engine.default-style", 0);
# endif
#else
	prop_set_bool(con, "font-engine", 1);
	prop_set_string(con, "font-engine.default-font", "bitmap.xcf");
	prop_set_int(con, "font-engine.default-size", -1);
	prop_set_int(con, "font-engine.default-style", -1);
#endif /* HAVE_FREETYPE */
}

int
config_load(void *p, struct netbuf *buf)
{
	struct version rv;

	if (version_read(buf, &config_ver, &rv) != 0)
		return (-1);

	kbd_unitrans = read_uint8(buf);
	text_composition = read_uint8(buf);
	window_freescale = read_uint8(buf);
	text_rightleft = read_uint8(buf);
	event_idle = (int)read_uint8(buf);
#ifdef DEBUG
	engine_debug = read_uint8(buf);
#endif
	kbd_delay = (int)read_uint32(buf);
	kbd_repeat = (int)read_uint32(buf);
	mouse_dblclick_delay = (int)read_uint32(buf);
	if (rv.minor >= 3) {
		mouse_spin_delay = (int)read_uint16(buf);
		mouse_spin_ival = (int)read_uint16(buf);
	}
#ifdef EDITION
	if (rv.minor >= 1 && read_uint8(buf) == 1)
		mapedit_load(buf);
#endif
	if (rv.minor >= 2)
		server_mode = read_uint8(buf);
	if (rv.minor >= 4)
		view_screenshot_quality = (int)read_uint8(buf);
	if (rv.minor >= 5)
		text_tab_width = (int)read_uint16(buf);
	if (vfmt != NULL &&
	    rv.minor >= 6)
		colors_load(buf);

	return (0);
}

int
config_save(void *p, struct netbuf *buf)
{
	version_write(buf, &config_ver);

	write_uint8(buf, (Uint8)kbd_unitrans);
	write_uint8(buf, (Uint8)text_composition);
	write_uint8(buf, (Uint8)window_freescale);
	write_uint8(buf, (Uint8)text_rightleft);
	write_uint8(buf, (Uint8)event_idle);
#ifdef DEBUG
	write_uint8(buf, (Uint8)engine_debug);
#else
	write_uint8(buf, 0);
#endif
	write_uint32(buf, (Uint32)kbd_delay);
	write_uint32(buf, (Uint32)kbd_repeat);
	write_uint32(buf, (Uint32)mouse_dblclick_delay);
	write_uint16(buf, (Uint16)mouse_spin_delay);
	write_uint16(buf, (Uint16)mouse_spin_ival);

#ifdef EDITION
	write_uint8(buf, 1);
	mapedit_save(buf);
#else
	write_uint8(buf, 0);
#endif
	write_uint8(buf, server_mode);
	write_uint8(buf, (Uint8)view_screenshot_quality);
	write_uint16(buf, (Uint16)text_tab_width);
	colors_save(buf);
	return (0);
}

static void
selected_color(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct hsvpal *hsv = argv[1].p;
	struct tlist_item *it = argv[2].p;
	Uint32 *c = it->p1;

	widget_bind(hsv, "pixel", WIDGET_UINT32, c);
}

void
config_window(struct config *con)
{
	struct window *win;
	struct vbox *vb;
	struct hbox *hb;
	struct button *button;
	struct textbox *tbox;
	struct checkbox *cbox;
	struct notebook *nb;
	struct notebook_tab *tab;
	struct mspinbutton *msb;
	struct spinbutton *sbu;

	win = window_new(0, "config-engine-settings");
	window_set_caption(win, _("Engine settings"));

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	tab = notebook_add_tab(nb, _("Video"), BOX_VERT);
	notebook_select_tab(nb, tab);
	{
		cbox = checkbox_new(tab, _("Full screen"));
		widget_bind(cbox, "state", WIDGET_PROP, config,
		    "view.full-screen");
		event_new(cbox, "checkbox-changed", config_set_full_screen,
		    NULL);

		cbox = checkbox_new(tab, _("Asynchronous blits"));
		widget_bind(cbox, "state", WIDGET_PROP, config,
		    "view.async-blits");
		event_new(cbox, "checkbox-changed", config_set_async_blits,
		    NULL);

		cbox = checkbox_new(tab, _("OpenGL mode"));
		widget_bind(cbox, "state", WIDGET_PROP, config, "view.opengl");
		event_new(cbox, "checkbox-changed", config_set_opengl, NULL);
	
		msb = mspinbutton_new(tab, "x", _("Default resolution: "));
		widget_bind(msb, "xvalue", WIDGET_UINT16, &view->w);
		widget_bind(msb, "yvalue", WIDGET_UINT16, &view->h);
		mspinbutton_set_range(msb, 320, 4096);
		
		sbu = spinbutton_new(tab, _("Screenshot quality (%%): "));
		widget_bind(sbu, "value", WIDGET_INT, &view_screenshot_quality);
		spinbutton_set_min(sbu, 1);
		spinbutton_set_max(sbu, 100);
	
		cbox = checkbox_new(tab, _("Idle when possible"));
		widget_bind(cbox, "state", WIDGET_INT, &event_idle);
	
		cbox = checkbox_new(tab, _("Unrestricted window resize"));
		widget_bind(cbox, "state", WIDGET_INT, &window_freescale);
	}

	tab = notebook_add_tab(nb, _("Input devices"), BOX_VERT);
	{
		cbox = checkbox_new(tab, _("Unicode keyboard translation"));
		widget_bind(cbox, "state", WIDGET_INT, &kbd_unitrans);
		event_new(cbox, "checkbox-changed", config_set_unitrans, NULL);

		cbox = checkbox_new(tab, _("Input composition"));
		widget_bind(cbox, "state", WIDGET_INT, &text_composition);

		cbox = checkbox_new(tab,
		    _("Right->left (Arabic, Hebrew, ...)"));
		widget_bind(cbox, "state", WIDGET_INT, &text_rightleft);
		
		sbu = spinbutton_new(tab, _("Mouse double click delay (ms): "));
		widget_bind(sbu, "value", WIDGET_INT, &mouse_dblclick_delay);
		spinbutton_set_min(sbu, 1);
		
		sbu = spinbutton_new(tab, _("Mouse spin delay (ms): "));
		widget_bind(sbu, "value", WIDGET_INT, &mouse_spin_delay);
		spinbutton_set_min(sbu, 1);

		sbu = spinbutton_new(tab, _("Mouse spin interval (ms): "));
		widget_bind(sbu, "value", WIDGET_INT, &mouse_spin_ival);
		spinbutton_set_min(sbu, 1);

		sbu = spinbutton_new(tab, _("Keyboard repeat delay (ms): "));
		widget_bind(sbu, "value", WIDGET_INT, &kbd_delay);
		spinbutton_set_min(sbu, 1);
		
		sbu = spinbutton_new(tab, _("Keyboard repeat interval (ms): "));
		widget_bind(sbu, "value", WIDGET_INT, &kbd_repeat);
		spinbutton_set_min(sbu, 1);
	}

	tab = notebook_add_tab(nb, _("Directories"), BOX_VERT);
	{
		char path[MAXPATHLEN];

		tbox = textbox_new(tab, _("Data save dir: "));
		prop_copy_string(config, "save-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "save-path");
	
		tbox = textbox_new(tab, _("Data load path: "));
		prop_copy_string(config, "load-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "load-path");
	
		tbox = textbox_new(tab, _("Font path: "));
		prop_copy_string(config, "font-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "font-path");
		
		tbox = textbox_new(tab, _("Den path: "));
		prop_copy_string(config, "den-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "den-path");
	}
	
	tab = notebook_add_tab(nb, _("Colors"), BOX_HORIZ);
	{
		struct hsvpal *hsv;
		struct tlist *tl;
		struct tlist_item *it;
		int i;

		tl = tlist_new(tab, 0);
		WIDGET(tl)->flags &= ~WIDGET_WFILL;
		for (i = 0; i < LAST_COLOR; i++) {
			it = tlist_insert(tl, NULL, _(colors_names[i]));
			it->p1 = &colors[i];
		}
		for (i = 0; i < colors_border_size; i++) {
			it = tlist_insert(tl, NULL, _("Window border #%i"),
			    i);
			it->p1 = &colors_border[i];
		}

		hsv = hsvpal_new(tab, vfmt);
		WIDGET(hsv)->flags |= WIDGET_WFILL|WIDGET_HFILL;
		
		event_new(tl, "tlist-selected", selected_color, "%p", hsv);
	}

#ifdef DEBUG
	tab = notebook_add_tab(nb, _("Debug"), BOX_VERT);
	{
		cbox = checkbox_new(tab, _("Debugging"));
		widget_bind(cbox, "state", WIDGET_INT, &engine_debug);

		cbox = checkbox_new(tab, _("Server mode"));
		widget_bind(cbox, "state", WIDGET_INT, &server_mode);
	}
#endif


	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	{
		button = button_new(hb, _("Close"));
		event_new(button, "button-pushed", window_generic_hide, "%p",
		    win);

		button = button_new(hb, _("Save"));
		event_new(button, "button-pushed", save_config, NULL);
	}
	config->settings = win;
}

/* Copy the full pathname to a data file to a fixed-size buffer. */
int
config_search_file(const char *path_key, const char *name, const char *ext,
    char *path, size_t path_len)
{
	char file[MAXPATHLEN];
	struct stat sta;
	char *dir, *pathp = path;

	prop_copy_string(config, path_key, path, path_len);

	for (dir = strsep(&pathp, ":");
	     dir != NULL;
	     dir = strsep(&pathp, ":")) {
		strlcpy(file, dir, sizeof(file));

		if (name[0] != '/') {
			strlcat(file, "/", sizeof(file));
		}
		strlcat(file, name, sizeof(file));
		if (ext != NULL) {
			strlcat(file, ".", sizeof(file));
			strlcat(file, ext, sizeof(file));
		}
		if (stat(file, &sta) == 0) {
			if (strlcpy(path, file, path_len) >= path_len) {
				error_set(_("The search path is too big."));
				return (-1);
			}
			return (0);
		}
	}
	prop_copy_string(config, path_key, path, path_len);
	error_set(_("Cannot find %s.%s (in <%s>:%s)."), name,
	    ext != NULL ? ext : "", path_key, path);
	return (-1);
}

