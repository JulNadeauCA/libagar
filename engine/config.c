/*	$Csoft: config.c,v 1.117 2004/04/26 03:21:17 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
	5, 0
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
extern int window_freescale;
extern int kbd_unitrans;

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
		    proginfo->progname);
}

static void
config_set_async_blits(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (enable)
		text_msg(MSG_WARNING,
		    _("Save the configuration and restart %s for async blits "
		      "to take effect"),
		    proginfo->progname);
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

	text_msg(MSG_INFO, _("The configuration settings have been saved."));
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
	strlcat(udatadir, proginfo->progname, sizeof(udatadir));
#else
	udatadir[0] = '.';
	strlcpy(&udatadir[1], proginfo->progname, sizeof(udatadir)-1);
#endif
	if (stat(udatadir, &sta) != 0 &&
	    Mkdir(udatadir) != 0) {
		fatal("%s: %s", udatadir, strerror(errno));
	}
	prop_set_string(con, "save-path", "%s", udatadir);

	prop_set_string(con, "den-path", "%s", SHAREDIR);
	prop_set_string(con, "load-path", "%s:%s", udatadir, SHAREDIR);
	prop_set_string(con, "font-path", "%s/fonts:%s", udatadir, TTFDIR);

#ifdef HAVE_FREETYPE
# ifdef WIN32
	prop_set_bool(con, "font-engine", 1);
	prop_set_string(con, "font-engine.default-font", "verdana");
	prop_set_int(con, "font-engine.default-size", 14);
	prop_set_int(con, "font-engine.default-style", 0);
# else
	prop_set_bool(con, "font-engine", 1);
	prop_set_string(con, "font-engine.default-font", "zekton");
	prop_set_int(con, "font-engine.default-size", 14);
	prop_set_int(con, "font-engine.default-style", 0);
# endif
#else
	prop_set_bool(con, "font-engine", 1);
	prop_set_string(con, "font-engine.default-font", "bitmap");
	prop_set_int(con, "font-engine.default-size", -1);
	prop_set_int(con, "font-engine.default-style", -1);
#endif /* HAVE_FREETYPE */
}

int
config_load(void *p, struct netbuf *buf)
{
	if (version_read(buf, &config_ver, NULL) != 0)
		return (-1);

	kbd_unitrans = read_uint8(buf);
	text_composition = read_uint8(buf);
	window_freescale = read_uint8(buf);
	text_rightleft = read_uint8(buf);
	event_idle = read_uint8(buf);
#ifdef DEBUG
	engine_debug = read_uint8(buf);
#endif
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
	return (0);
}

static void
poll_input_devs(int argc, union evarg *argv)
{
	extern struct input_devq input_devs;
	extern pthread_mutex_t input_lock;
	struct tlist *tl = argv[0].p;
	struct input *in;

	pthread_mutex_lock(&input_lock);
	tlist_clear_items(tl);
	SLIST_FOREACH(in, &input_devs, inputs) {
		tlist_insert_item(tl, NULL, in->name, in);
	}
	tlist_restore_selections(tl);
	pthread_mutex_unlock(&input_lock);
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

	win = window_new("config-engine-settings");
	window_set_caption(win, _("Engine settings"));
	window_set_closure(win, WINDOW_HIDE);
	hb = hbox_new(win, 0);
	vb = vbox_new(hb, 0);
	vbox_set_spacing(vb, 2);

	cbox = checkbox_new(vb, _("Full screen"));
	widget_bind(cbox, "state", WIDGET_PROP, config, "view.full-screen");
	event_new(cbox, "checkbox-changed", config_set_full_screen, NULL);

	cbox = checkbox_new(vb, _("Asynchronous blits"));
	widget_bind(cbox, "state", WIDGET_PROP, config, "view.async-blits");
	event_new(cbox, "checkbox-changed", config_set_async_blits, NULL);

	cbox = checkbox_new(vb, _("OpenGL rendering context"));
	widget_bind(cbox, "state", WIDGET_PROP, config, "view.opengl");
	event_new(cbox, "checkbox-changed", config_set_opengl, NULL);

	cbox = checkbox_new(vb, _("Unicode keyboard translation"));
	widget_bind(cbox, "state", WIDGET_INT, &kbd_unitrans);
	event_new(cbox, "checkbox-changed", config_set_unitrans, NULL);

	cbox = checkbox_new(vb, _("Input composition"));
	widget_bind(cbox, "state", WIDGET_INT, &text_composition);

	cbox = checkbox_new(vb, _("Unrestricted window resize"));
	widget_bind(cbox, "state", WIDGET_INT, &window_freescale);

	cbox = checkbox_new(vb, _("Right->left (Arabic, Hebrew, ...)"));
	widget_bind(cbox, "state", WIDGET_INT, &text_rightleft);

	cbox = checkbox_new(vb, _("Idle time prediction"));
	widget_bind(cbox, "state", WIDGET_INT, &event_idle);

#ifdef DEBUG
	cbox = checkbox_new(vb, _("Debugging"));
	widget_bind(cbox, "state", WIDGET_INT, &engine_debug);
#endif

	vb = vbox_new(hb, 0);
	vbox_set_spacing(vb, 2);
	{
		struct tlist *tl;

		label_new(vb, LABEL_STATIC, _("Input devices:"));
		tl = tlist_new(vb, TLIST_POLL);
		tlist_prescale(tl, "keyboard0", 6);
		event_new(tl, "tlist-poll", poll_input_devs, NULL);
	}

	vb = vbox_new(win, VBOX_WFILL);
	{
		char path[MAXPATHLEN];

		tbox = textbox_new(vb, _("Data save dir: "));
		prop_copy_string(config, "save-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "save-path");
	
		tbox = textbox_new(vb, _("Data load path: "));
		prop_copy_string(config, "load-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "load-path");
	
		tbox = textbox_new(vb, _("Font path: "));
		prop_copy_string(config, "font-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "font-path");
		
		tbox = textbox_new(vb, _("Den path: "));
		prop_copy_string(config, "den-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_set_path, "%s",
		    "den-path");
	}

	hb = hbox_new(win, HBOX_WFILL|HBOX_HOMOGENOUS);
	{
		struct mspinbutton *msb;

		msb = mspinbutton_new(hb, "x", _("Default resolution: "));
		widget_bind(msb, "xvalue", WIDGET_PROP, config, "view.w");
		widget_bind(msb, "yvalue", WIDGET_PROP, config, "view.h");
		mspinbutton_set_range(msb, 320, 4096);
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL|HBOX_HFILL);
	button = button_new(hb, _("Close"));
	event_new(button, "button-pushed", window_generic_hide, "%p", win);
	button = button_new(hb, _("Save"));
	event_new(button, "button-pushed", save_config, NULL);
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
		strlcat(file, ".", sizeof(file));
		strlcat(file, ext, sizeof(file));
		if (stat(file, &sta) == 0) {
			if (strlcpy(path, file, path_len) >= path_len) {
				error_set(_("The search path is too big."));
				return (-1);
			}
			return (0);
		}
	}
	prop_copy_string(config, path_key, path, path_len);
	error_set(_("Cannot find `%s.%s' in %s (%s)."), name, ext, path_key,
	    path);
	return (-1);
}

