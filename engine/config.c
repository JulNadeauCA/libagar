/*	$Csoft: config.c,v 1.70 2003/04/17 08:18:32 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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
#include <config/have_x11.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/map.h>
#include <engine/world.h>
#include <engine/prop.h>

#include <engine/widget/text.h>
#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/primitive.h>
#include <engine/widget/tlist.h>

#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

static const struct version config_ver = {
	"agar config",
	3, 0
};

static const struct object_ops config_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

static struct window *primitives_win;	/* Primitive algorithm switch */

enum {
	CLOSE_BUTTON,
	SAVE_BUTTON,
	PRIMITIVES_BUTTON,
	UDATADIR_TBOX,
	SYSDATADIR_TBOX,
	DATAPATH_TBOX,
	W_TBOX,
	H_TBOX
};

static void	config_apply_string(int, union evarg *);
static void	config_apply_int(int, union evarg *);
static void	config_apply(int, union evarg *);

struct config *
config_new(void)
{
	struct config *con;

	con = Malloc(sizeof(struct config));
	config_init(con);
	return (con);
}

static void
config_prop_modified(int argc, union evarg *argv)
{
	struct prop *prop = argv[1].p;

	if (strcmp(prop->key, "view.full-screen") == 0) {
		if (view != NULL) {
			SDL_Event vexp;

			if ((prop->data.i &&
			    (!(view->v->flags & SDL_FULLSCREEN))) ||
			   (!prop->data.i &&
			    ((view->v->flags & SDL_FULLSCREEN)))) {
				SDL_WM_ToggleFullScreen(view->v);
				vexp.type = SDL_VIDEOEXPOSE;
				SDL_PushEvent(&vexp);
			}
		}
	}
}

void
config_init(struct config *con)
{
	extern const struct engine_proginfo *proginfo;		/* engine.c */
	struct passwd *pwd;
	struct stat sta;
	char *udatadir, *sysdatadir;

	object_init(&con->obj, "engine-config", "config",
	    OBJECT_STATIC|OBJECT_RELOAD_PROPS, &config_ops);

	/* Object settings */
	prop_set_bool(con, "object.art.map-tiles", 0);

	/* Visual settings */
	prop_set_bool(con, "view.full-screen", 0);
	prop_set_bool(con, "view.async-blits", 0);
#ifdef HAVE_X11
	prop_set_bool(con, "view.xsync", 0);
#endif
#ifdef HAVE_OPENGL
	prop_set_bool(con, "view.opengl", 0);
#endif
	prop_set_uint16(con, "view.w", 800);
	prop_set_uint16(con, "view.h", 600);
	prop_set_uint16(con, "view.min-w", 320);
	prop_set_uint16(con, "view.min-h", 240);
	prop_set_uint8(con, "view.depth", 32);

	/* Font engine settings */
	prop_set_bool(con, "font-engine", 1);
	prop_set_string(con, "font-engine.default-font", "zekton");
	prop_set_int(con, "font-engine.default-size", 11);
	prop_set_int(con, "font-engine.default-style", 0);

	/* Widget settings */
	prop_set_bool(con, "widget.reg-borders", 0);
	prop_set_bool(con, "widget.any-size", 0);
	
	/* Input device settings */
	prop_set_bool(con, "input.joysticks", 1);

	/* Data directories. */
	pwd = getpwuid(getuid());
	prop_set_string(con, "path.user_data_dir", "%s/.%s",
	    pwd->pw_dir, proginfo->prog);
	prop_set_string(con, "path.sys_data_dir", "%s", SHAREDIR);

	udatadir = prop_get_string(con, "path.user_data_dir");
	sysdatadir = prop_get_string(con, "path.sys_data_dir");
	if (stat(sysdatadir, &sta) != 0) {
		warning("%s: %s\n", sysdatadir, strerror(errno));
	}
	if (stat(udatadir, &sta) != 0 &&
	    mkdir(udatadir, 00700) != 0) {
		fatal("%s: %s", udatadir, strerror(errno));
	}
	prop_set_string(con, "path.data_path", "%s:%s", udatadir, sysdatadir);
	free(udatadir);
	free(sysdatadir);

	event_new(con, "prop-modified", config_prop_modified, NULL);
}

void
config_window(struct config *con)
{
	struct window *win;
	struct region *reg;
	struct button *button;
	struct textbox *tbox;
	struct checkbox *cbox;

	win = window_generic_new(388, 362, "config-engine-settings");
	event_new(win, "window-close", window_generic_hide, "%p", win);
	window_set_caption(win, "Engine settings");
	window_set_spacing(win, 2, 8);

	/* Flags */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		const struct {
			char *name;
			char *descr;
		} settings[] = {
			{ "view.full-screen", "Full screen" },
			{ "view.async-blits", "Asynchronous blits (restart)" },
#ifdef DEBUG
			{ "widget.reg-borders",	"Region borders" },
			{ "widget.any-size", "Arbitrary window sizes" },
# ifdef HAVE_X11
			{ "view.xsync",	"Synchronous X events (restart)" },
# endif
#endif
#ifdef HAVE_OPENGL
			{ "view.opengl", "OpenGL rendering context (restart)" },
#endif
		};
		const int nsettings = sizeof(settings) / sizeof(settings[0]);
		int i;

		for (i = 0; i < nsettings; i++) {
			cbox = checkbox_new(reg, -1, "%s", settings[i].descr);
			widget_bind(cbox, "state", WIDGET_PROP,
			    config, settings[i].name);
		}

#ifdef DEBUG
		/* XXX thread unsafe */
		cbox = checkbox_new(reg, -1, "Debugging");
		widget_bind(cbox, "state", WIDGET_BOOL, NULL, &engine_debug);
		
		cbox = checkbox_new(reg, -1, "Node signature checking");
		widget_bind(cbox, "state", WIDGET_INT, NULL, &map_nodesigs);
#endif

		cbox = checkbox_new(reg, -1, "Idle time prediction");
		widget_bind(cbox, "state", WIDGET_INT, NULL, &event_idle);
	}
	
	/* Directories */
	reg = region_new(win, REGION_VALIGN, 0, -1, 100, -1);
	{
		char *s;

		tbox = textbox_new(reg, "  User datadir: ", 0, 100, -1);
		s = prop_get_string(config, "path.user_data_dir");
		textbox_printf(tbox, "%s", s);
		free(s);
		event_new(tbox, "textbox-changed",
		    config_apply_string, "%s", "path.user_data_dir");
	
		tbox = textbox_new(reg, "System datadir: ", 0, 100, -1);
		s = prop_get_string(config, "path.sys_data_dir");
		textbox_printf(tbox, "%s", s);
		free(s);
		event_new(tbox, "textbox-changed",
		    config_apply_string, "%s", "path.sys_data_dir");
		
		tbox = textbox_new(reg, "Data file path: ", 0, 100, -1);
		s = prop_get_string(config, "path.data_path");
		textbox_printf(tbox, "%s", s);
		free(s);
		event_new(tbox, "textbox-changed",
		    config_apply_string, "%s", "path.data_path");
	}

	/* Resolution */
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		tbox = textbox_new(reg, "Width : ", 0, 50, -1);
		textbox_printf(tbox, "%d", prop_get_uint16(config, "view.w"));
		event_new(tbox, "textbox-changed",
		    config_apply_int, "%s", "view.w");

		tbox = textbox_new(reg, "Height: ", 0, 50, -1);
		textbox_printf(tbox, "%d", prop_get_uint16(config, "view.h"));
		event_new(tbox, "textbox-changed",
		    config_apply_int, "%s", "view.h");
	}

	/* Buttons */
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	{
		button = button_new(reg, "Close", NULL, 0, 33, 100);
		event_new(button, "button-pushed",
		    config_apply, "%i", CLOSE_BUTTON);
		win->focus = WIDGET(button);

		button = button_new(reg, "Save", NULL, 0, 34, 100);
		event_new(button, "button-pushed",
		    config_apply, "%i", SAVE_BUTTON);
		
		button = button_new(reg, "Primitives", NULL, 0, 33, 100);
		event_new(button, "button-pushed",
		    config_apply, "%i", PRIMITIVES_BUTTON);
	}

	/* Primitive algorithm switch */
	primitives_win = primitive_config_window();
	config->settings = win;
}

static void
config_apply_string(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	char *varname = argv[1].s;
	char *s;

	s = textbox_string(tbox);
	prop_set_string(config, varname, "%s", s);
	free(s);
}

static void
config_apply_int(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	char *varname = argv[1].s;
	int i;

	i = textbox_int(tbox);
	prop_set_int(config, varname, i);
}

static void
config_apply(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct textbox *tbox = argv[0].p;
	int i;

	switch (argv[1].i) {
	case CLOSE_BUTTON:
		window_hide(wid->win);
		return;
	case SAVE_BUTTON:
		if (object_save(config, NULL) == -1)
			text_msg("Error saving", "%s", error_get());
		return;
	case PRIMITIVES_BUTTON:
		window_show(primitives_win);
		return;
	}

	switch (argv[1].i) {
	case UDATADIR_TBOX:
		break;
	case SYSDATADIR_TBOX:
		prop_set_string(config, "path.sys_data_dir", "%s", tbox->text);
		break;
	case DATAPATH_TBOX:
		prop_set_string(config, "path.data_path", "%s", tbox->text);
		break;
	case W_TBOX:
		i = textbox_int(tbox);
		prop_set_int(config, "view.w", i);
		break;
	case H_TBOX:
		i = textbox_int(tbox);
		prop_set_int(config, "view.h", i);
		break;
	}
}

