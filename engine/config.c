/*	$Csoft: config.c,v 1.79 2003/05/26 03:03:29 vedge Exp $	    */

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

#include <engine/compat/asprintf.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/map.h>
#include <engine/prop.h>

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

#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

const struct version config_ver = {
	"agar config",
	3, 0
};

struct config *
config_new(void)
{
	struct config *con;

	con = Malloc(sizeof(struct config));
	config_init(con);
	return (con);
}

static void
config_change_path(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	char *varname = argv[1].s;
	char *s;

	s = textbox_string(tbox);
	prop_set_string(config, varname, "%s", s);
	free(s);
}

static void
config_change_res(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	char *varname = argv[1].s;
	int i;

	i = textbox_int(tbox);
	prop_set_int(config, varname, i);
}

static void
config_save(int argc, union evarg *argv)
{
	if (object_save(config) == -1)
		text_msg("Error saving", "%s", error_get());
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
#if 0
	} else if (strcmp(prop->key, "view.opengl") == 0) {
		text_msg("Warning", "Restart %s for OpenGL mode to take effect",
		    proginfo->progname);
	} else if (strcmp(prop->key, "view.async-blits") == 0) {
		text_msg("Warning", "Restart %s for async blits to take effect",
		    proginfo->progname);
#endif
	}
}

void
config_init(struct config *con)
{
	char *savepath;
	struct passwd *pwd;
	struct stat sta;

	object_init(con, "engine-config", "config", NULL);
	OBJECT(con)->flags |= OBJECT_RELOAD_PROPS;

	prop_set_bool(con, "view.full-screen", 0);
	prop_set_bool(con, "view.async-blits", 0);
	prop_set_bool(con, "view.opengl", 0);
	prop_set_uint16(con, "view.w", 800);
	prop_set_uint16(con, "view.h", 600);
	prop_set_uint16(con, "view.min-w", 320);
	prop_set_uint16(con, "view.min-h", 240);
	prop_set_uint8(con, "view.depth", 32);
	
	prop_set_bool(con, "widget.noinitscale", 0);

	prop_set_bool(con, "font-engine", 1);
	prop_set_string(con, "font-engine.default-font", "zekton");
	prop_set_int(con, "font-engine.default-size", 12);
	prop_set_int(con, "font-engine.default-style", 0);

	prop_set_bool(con, "input.joysticks", 1);

	pwd = getpwuid(getuid());
	Asprintf(&savepath, "%s/.%s", pwd->pw_dir, proginfo->progname);

	prop_set_string(con, "save-path", "%s", savepath);
	prop_set_string(con, "load-path", "%s:%s", savepath, SHAREDIR);

	if (stat(savepath, &sta) != 0 &&
	    mkdir(savepath, 0700) != 0) {
		fatal("%s: %s", savepath, strerror(errno));
	}

	event_new(con, "prop-modified", config_prop_modified, NULL);
	free(savepath);
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
	window_set_caption(win, "Engine settings");
	window_set_closure(win, WINDOW_HIDE);

	vb = vbox_new(win, 0);
	vbox_set_spacing(vb, 2);
	vbox_set_padding(vb, 20);
	{
		const struct {
			char *name;
			char *descr;
		} settings[] = {
			{ "view.full-screen", "Full screen" },
			{ "view.async-blits", "Asynchronous blits" },
#ifdef HAVE_OPENGL
			{ "view.opengl", "OpenGL rendering context" },
#endif
			{ "widget.noinitscale", "Skip initial widget scaling" },
		};
		const int nsettings = sizeof(settings) / sizeof(settings[0]);
		int i;

		for (i = 0; i < nsettings; i++) {
			cbox = checkbox_new(vb, "%s", settings[i].descr);
			widget_bind(cbox, "state", WIDGET_PROP, config,
			    settings[i].name);
		}

#ifdef DEBUG
		/* Thread unsafe, but not very dangerous. */
		cbox = checkbox_new(vb, "Debugging");
		widget_bind(cbox, "state", WIDGET_INT, NULL, &engine_debug);
		
		cbox = checkbox_new(vb, "Node signature checking");
		widget_bind(cbox, "state", WIDGET_INT, NULL, &map_nodesigs);
#endif
		cbox = checkbox_new(vb, "Idle time prediction");
		widget_bind(cbox, "state", WIDGET_INT, NULL, &event_idle);
	}
	
	vb = vbox_new(win, VBOX_WFILL);
	{
		char path[FILENAME_MAX];

		tbox = textbox_new(vb, "Data save dir: ");
		prop_copy_string(config, "save-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_change_path, "%s",
		    "save-path");
	
		tbox = textbox_new(vb, "Data load path: ");
		prop_copy_string(config, "load-path", path, sizeof(path));
		textbox_printf(tbox, "%s", path);
		event_new(tbox, "textbox-return", config_change_path, "%s",
		    "load-path");
	}

	hb = hbox_new(win, HBOX_WFILL|HBOX_HOMOGENOUS);
	{
		/* XXX propose some default resolutions. */

		tbox = textbox_new(hb, "Width: ");
		textbox_printf(tbox, "%d", prop_get_uint16(config, "view.w"));
		event_new(tbox, "textbox-return", config_change_res, "%s",
		    "view.w");
		WIDGET(tbox)->flags &= ~(WIDGET_WFILL);

		tbox = textbox_new(hb, "Height: ");
		textbox_printf(tbox, "%d", prop_get_uint16(config, "view.h"));
		event_new(tbox, "textbox-return", config_change_res, "%s",
		    "view.h");
		WIDGET(tbox)->flags &= ~(WIDGET_WFILL);
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL|HBOX_HFILL);
	hbox_set_spacing(hb, 0);
	hbox_set_padding(hb, 0);
	{
		button = button_new(hb, "Close");
		event_new(button, "button-pushed", window_generic_hide, 
		    "%p", win);
		widget_set_focus(button);

		button = button_new(hb, "Save");
		event_new(button, "button-pushed", config_save, NULL);
	}
	config->settings = win;
}

/* Return the full pathname to a data file. */
char *
config_search_file(const char *path_key, const char *name, const char *ext)
{
	struct stat sta;
	char *path, *dir, *last;

	path = prop_get_string(config, path_key);
	for (dir = strtok_r(path, ":", &last);
	     dir != NULL;
	     dir = strtok_r(NULL, ":", &last)) {
		char *file;

		Asprintf(&file, "%s%s.%s", dir, name, ext);
		if (stat(file, &sta) == 0) {
			return (file);
		}
		free(file);
	}
	free(path);

	error_set("`%s.%s' not in <%s>", name, ext, path_key);
	return (NULL);
}

