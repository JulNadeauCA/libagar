/*	$Csoft: config.c,v 1.38 2002/09/13 11:08:29 vedge Exp $	    */

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#include <libfobj/fobj.h>

#include "engine.h"
#include "map.h"
#include "version.h"
#include "config.h"

#include "widget/text.h"
#include "widget/widget.h"
#include "widget/window.h"
#include "widget/label.h"
#include "widget/button.h"
#include "widget/radio.h"
#include "widget/checkbox.h"
#include "widget/textbox.h"
#include "widget/keycodes.h"
#include "widget/primitive.h"
#include "widget/tlist.h"

static const struct version config_ver = {
	"agar config",
	3, 0
};

static const struct object_ops config_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

enum {
	CLOSE_BUTTON,
	SAVE_BUTTON,
	FULLSCREEN_CBOX,
	FONTCACHE_CBOX,
	ASYNCBLIT_CBOX,
	VISREGIONS_CBOX,
	ANYSIZE_CBOX,
	DEBUG_CBOX,
	UDATADIR_TBOX,
	SYSDATADIR_TBOX,
	DATAPATH_TBOX,
	W_TBOX,
	H_TBOX
};

static struct config_prop	*config_get_prop(const char *);
static void			 config_apply_string(int, union evarg *);
static void			 config_apply_int(int, union evarg *);
static void			 config_apply(int, union evarg *);

struct config *
config_new(void)
{
	struct config *con;

	con = emalloc(sizeof(struct config));
	config_init(con);

	return (con);
}

void
config_init(struct config *con)
{
	extern const struct gameinfo *gameinfo;		/* engine.c */
	struct passwd *pwd;
	struct stat sta;
	char *spath;
	char *udatadir, *sysdatadir;

	pwd = getpwuid(getuid());

	object_init(&con->obj, "engine-config", "config", NULL, 0, &config_ops);

	/* Generic engine operation flags. */
	prop_set_uint32(con, "flags", CONFIG_FONT_CACHE);

	/* Graphic settings. */
	prop_set_uint32(con, "view.w",	 800);
	prop_set_uint32(con, "view.h",	 600);
	prop_set_uint32(con, "view.bpp", 32);

	/* Widget settings */
	prop_set_uint32(con, "widgets.flags", 0);

	/* Caching settings */
	prop_set_uint32(con, "caching.surface_kB", 1024);
	prop_set_uint32(con, "caching.glyph_kB", 0);

	/* Pathnames */
	prop_set_string(con, "path.user_data_dir",
	    "%s/.%s", pwd->pw_dir, gameinfo->prog);
	prop_set_string(con, "path.sys_data_dir",
	    "%s", SHAREDIR);
	udatadir = prop_string(con, "path.user_data_dir");
	sysdatadir = prop_string(con, "path.sys_data_dir");
	prop_set_string(con, "path.data_path", "%s:%s", udatadir, sysdatadir);

	if (stat(sysdatadir, &sta) != 0) {
		warning("%s: %s\n", sysdatadir, strerror(errno));
	}
	if (stat(udatadir, &sta) != 0 && mkdir(udatadir, 00700) != 0) {
		warning("created %s\n", udatadir);
		fatal("%s: %s\n", udatadir, strerror(errno));
	}
}

/*
 * The config_init() function is called too early in initialization
 * to create the configure settings windows.
 *
 * Non thread safe.
 */
void
config_init_wins(struct config *con)
{
	struct window *win;
	struct region *reg;
	struct button *button;
	struct textbox *tbox;
	struct checkbox *cbox;
	struct radio *rad;
	struct tlist *tl;
	struct label *lab;

	/*
	 * Engine settings window
	 */
	win = window_new("config-engine-settings", "Engine settings",
	    WINDOW_CENTER, 0, 0,
	    267, 319,
	    267, 319);
	con->windows.settings = win;

	/* Flags */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 40);
	{
		cbox = checkbox_new(reg, "Asynchronous blits (restart)", 0,
		    (prop_uint32(config, "flags") & CONFIG_ASYNCBLIT) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", config_apply,
		    "%i", ASYNCBLIT_CBOX);

		cbox = checkbox_new(reg, "Full screen", 0,
		    (prop_uint32(config, "flags") & CONFIG_FULLSCREEN) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", config_apply,
		    "%i", FULLSCREEN_CBOX);
		
		cbox = checkbox_new(reg, "Font cache", 0,
		    (prop_uint32(config, "flags") & CONFIG_FONT_CACHE) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", config_apply,
		    "%i", FONTCACHE_CBOX);

#ifdef DEBUG
		cbox = checkbox_new(reg, "Debugging enabled", 0,
		    engine_debug ? CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", config_apply,
		    "%i", DEBUG_CBOX);

		cbox = checkbox_new(reg, "Visible regions", 0,
		    (prop_uint32(config, "widgets.flags") &
		     CONFIG_REGION_BORDERS) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", config_apply,
		    "%i", VISREGIONS_CBOX);

		cbox = checkbox_new(reg, "Arbitrary window sizes", 0,
		    (prop_uint32(config, "widgets.flags") &
		     CONFIG_WINDOW_ANYSIZE) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", config_apply,
		    "%i", ANYSIZE_CBOX);
#endif
	}

	/* Directories */
	reg = region_new(win, REGION_VALIGN,  0, 41, 100, 34);
	{
		char *s;

		tbox = textbox_new(reg, "  User datadir: ", 0, 100, 33);
		s = prop_string(config, "path.user_data_dir");
		textbox_printf(tbox, "%s", s);
		free(s);
		event_new(tbox, "textbox-changed",
		    config_apply_string, "%s", "path.user_data_dir");
	
		tbox = textbox_new(reg, "System datadir: ", 0, 100, 33);
		s = prop_string(config, "path.sys_data_dir");
		textbox_printf(tbox, "%s", s);
		free(s);
		event_new(tbox, "textbox-changed",
		    config_apply_string, "%s", "path.sys_data_dir");
		
		tbox = textbox_new(reg, "Data file path: ", 0, 100, 33);
		s = prop_string(config, "path.data_path");
		textbox_printf(tbox, "%s", s);
		free(s);
		event_new(tbox, "textbox-changed",
		    config_apply_string, "%s", "path.data_path");
	}

	/* Resolution */
	reg = region_new(win, REGION_HALIGN,  0, 75, 100, 12);
	{
		tbox = textbox_new(reg, "Width : ", 0, 50, 100);
		textbox_printf(tbox, "%d", prop_uint32(config, "view.w"));
		event_new(tbox, "textbox-changed",
		    config_apply_int, "%s", "view.w");

		tbox = textbox_new(reg, "Height: ", 0, 50, 100);
		textbox_printf(tbox, "%d", prop_uint32(config, "view.h"));
		event_new(tbox, "textbox-changed",
		    config_apply_int, "%s", "view.h");
	}

	/* Buttons */
	reg = region_new(win, REGION_HALIGN, 0,  87, 100, 13);
	{
		button = button_new(reg, "Close", NULL, 0, 50, 90);
		event_new(button, "button-pushed",
		    config_apply, "%i", CLOSE_BUTTON);
		win->focus = WIDGET(button);

		button = button_new(reg, "Save", NULL, 0, 50, 90);
		event_new(button, "button-pushed",
		    config_apply, "%i", SAVE_BUTTON);
	}

	con->windows.algorithm_sw = primitive_config_window();
}

#define CONFIG_SET_FLAG(con, var, flag, val) do {		\
	if ((val)) {						\
		prop_set_uint32((con), (var),			\
		    prop_uint32((con), (var)) | (flag));	\
	} else {						\
		prop_set_uint32((con), (var),			\
		    prop_uint32((con), (var)) & ~(flag));	\
	}							\
} while (/*CONSTCOND*/ 0)

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
		object_save(config);
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
	case FONTCACHE_CBOX:
		CONFIG_SET_FLAG(config, "flags", CONFIG_FONT_CACHE, argv[2].i);
		if (argv[2].i) {
			keycodes_loadglyphs();
		} else {
			keycodes_freeglyphs();
		}
		break;
	case FULLSCREEN_CBOX:
		CONFIG_SET_FLAG(config, "flags", CONFIG_FULLSCREEN, argv[2].i);
		SDL_WM_ToggleFullScreen(view->v);
		VIEW_REDRAW();
		break;
	case ASYNCBLIT_CBOX:
		CONFIG_SET_FLAG(config, "flags", CONFIG_ASYNCBLIT, argv[2].i);
		break;
#ifdef DEBUG
	case DEBUG_CBOX:
		engine_debug = argv[2].i;	/* XXX unsafe */
		break;
	case VISREGIONS_CBOX:
		CONFIG_SET_FLAG(config, "widgets.flags", CONFIG_REGION_BORDERS,
		    argv[2].i);
		break;
	case ANYSIZE_CBOX:
		CONFIG_SET_FLAG(config, "widgets.flags", CONFIG_WINDOW_ANYSIZE,
		    argv[2].i);
		break;
#endif
	}
}

