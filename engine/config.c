/*	$Csoft: config.c,v 1.33 2002/09/03 09:30:18 vedge Exp $	    */

/*
 * Copyright (c) 2002 CubeSoft Communications <http://www.csoft.org>
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

static const struct version config_ver = {
	"agar config",
	3, 0
};

static const struct object_ops config_ops = {
	config_destroy,
	config_load,
	config_save
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
	H_TBOX,
	AL_BOX_RADIO,
	AL_FRAME_RADIO,
	AL_CIRCLE_RADIO,
	AL_LINE_RADIO,
	AL_SQUARE_RADIO
};

#define CONFIG_DEFAULT_WIDTH 	800
#define CONFIG_DEFAULT_HEIGHT 	600
#define CONFIG_DEFAULT_BPP 	32
#define CONFIG_DEFAULT_FLAGS	(CONFIG_FONT_CACHE)

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

	pwd = getpwuid(getuid());

	object_init(&con->obj, "engine-config", "config", NULL, 0, &config_ops);
	con->flags = CONFIG_FONT_CACHE;
	con->view.w = CONFIG_DEFAULT_WIDTH;
	con->view.h = CONFIG_DEFAULT_HEIGHT;
	con->view.bpp = CONFIG_DEFAULT_BPP;
	con->widget_flags = 0;

	/* User data directory */
	asprintf(&con->path.user_data_dir, "%s/.%s", pwd->pw_dir,
	    gameinfo->prog);

	/* System data directory */
	asprintf(&con->path.sys_data_dir, "%s", SHAREDIR);

	/* Datafile path */
	asprintf(&con->path.data_path, "%s:%s",
	    con->path.user_data_dir,
	    con->path.sys_data_dir);
	
	if (stat(con->path.sys_data_dir, &sta) != 0) {
		warning("%s: %s\n", con->path.sys_data_dir, strerror(errno));
	}
	
	if (stat(con->path.user_data_dir, &sta) != 0 &&
	    mkdir(con->path.user_data_dir, 00700) != 0) {
		warning("created %s\n", con->path.user_data_dir);
		fatal("%s: %s\n", con->path.user_data_dir, strerror(errno));
	}

	pthread_mutex_init(&con->lock, NULL);
}

void
config_destroy(void *p)
{
	struct config *con = p;

	free(con->path.data_path);
	free(con->path.user_data_dir);
	free(con->path.sys_data_dir);

	pthread_mutex_destroy(&con->lock);
}

int
config_load(void *p, int fd)
{
	struct config *con = p;

	pthread_mutex_lock(&con->lock);

	version_read(fd, &config_ver);
	config->flags = read_uint32(fd);
	config->view.w = read_uint32(fd);
	config->view.h = read_uint32(fd);
	config->view.bpp = read_uint32(fd);
	config->widget_flags = read_uint32(fd);
	config->path.data_path = read_string(fd);
	config->path.user_data_dir = read_string(fd);
	config->path.sys_data_dir = read_string(fd);

	pthread_mutex_unlock(&con->lock);

	dprintf("loaded settings (flags=0x%x)\n", config->flags);
	return (0);
}

int
config_save(void *p, int fd)
{
	struct config *con = p;

	pthread_mutex_lock(&con->lock);

	version_write(fd, &config_ver);
	write_uint32(fd, con->flags);
	write_uint32(fd, con->view.w);
	write_uint32(fd, con->view.h);
	write_uint32(fd, con->view.bpp);
	write_uint32(fd, con->widget_flags);
	write_string(fd, con->path.data_path);
	write_string(fd, con->path.user_data_dir);
	write_string(fd, con->path.sys_data_dir);
	pthread_mutex_unlock(&con->lock);
	
	dprintf("saved settings (flags=0x%x)\n", config->flags);
	return (0);
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

	/*
	 * Engine settings window
	 */
	win = window_new("config-engine-settings", "Engine settings",
	    WINDOW_CENTER, 0, 0,
	    480, 467,
	    273, 467);
	con->windows.settings = win;

	/* Flags */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 40);
	{
		cbox = checkbox_new(reg, "Asynchronous blits (restart)", 0,
		    (con->flags & CONFIG_ASYNCBLIT) ? CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", 0, config_apply,
		    "%i", ASYNCBLIT_CBOX);

		cbox = checkbox_new(reg, "Full screen", 0,
		    (con->flags & CONFIG_FULLSCREEN) ? CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", 0, config_apply,
		    "%i", FULLSCREEN_CBOX);
		
		cbox = checkbox_new(reg, "Font cache", 0,
		    (con->flags & CONFIG_FONT_CACHE) ? CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", 0, config_apply,
		    "%i", FONTCACHE_CBOX);

#ifdef DEBUG
		cbox = checkbox_new(reg, "Debugging enabled", 0,
		    engine_debug ? CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", 0, config_apply,
		    "%i", DEBUG_CBOX);

		cbox = checkbox_new(reg, "Visible regions", 0,
		    (con->widget_flags & CONFIG_REGION_BORDERS) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", 0, config_apply,
		    "%i", VISREGIONS_CBOX);

		cbox = checkbox_new(reg, "Arbitrary window sizes", 0,
		    (con->widget_flags & CONFIG_WINDOW_ANYSIZE) ?
		     CHECKBOX_PRESSED : 0);
		event_new(cbox, "checkbox-changed", 0, config_apply,
		    "%i", ANYSIZE_CBOX);
#endif
	}

	/* Directories */
	reg = region_new(win, REGION_VALIGN,  0, 45, 100, 18);
	{
		tbox = textbox_new(reg, "  User datadir: ",
		    0, 100, 33);
		textbox_printf(tbox, "%s", config->path.user_data_dir);
		event_new(tbox, "textbox-changed", 0,
		    config_apply, "%i", UDATADIR_TBOX);

		tbox = textbox_new(reg, "System datadir: ",
		    0, 100, 33);
		textbox_printf(tbox, "%s", config->path.sys_data_dir);
		event_new(tbox, "textbox-changed", 0,
		    config_apply, "%i", SYSDATADIR_TBOX);
		
		tbox = textbox_new(reg, "Data file path: ",
		    0, 100, 33);
		textbox_printf(tbox, "%s", config->path.data_path);
		event_new(tbox, "textbox-changed", 0,
		    config_apply, "%i", DATAPATH_TBOX);
	}

	/* Resolution */
	reg = region_new(win, REGION_HALIGN,  0, 70, 100, 10);
	{
		tbox = textbox_new(reg, "Width : ", 0, 50, 100);
		textbox_printf(tbox, "%d", con->view.w);
		event_new(tbox, "textbox-changed", 0,
		    config_apply, "%i", W_TBOX);

		tbox = textbox_new(reg, "Height: ", 0, 50, 100);
		textbox_printf(tbox, "%d", con->view.h);
		event_new(tbox, "textbox-changed", 0,
		    config_apply, "%i", H_TBOX);
	}

	/* Buttons */
	reg = region_new(win, REGION_HALIGN, 0,  90, 100, 10);
	{
		button = button_new(reg, "Close", NULL, 0, 50, 90);
		event_new(button, "button-pushed", 0,
		    config_apply, "%i", CLOSE_BUTTON);
		win->focus = WIDGET(button);

		button = button_new(reg, "Save", NULL, 0, 50, 90);
		event_new(button, "button-pushed", 0,
		    config_apply, "%i", SAVE_BUTTON);
	}

	/* Primitive drawing algorithm switch */
	win = window_new("config-primitive-algorithm-sw",
	    "Primitive algorithm switch", WINDOW_CENTER,
	    0, 0,
	    247, 180,
	    247, 180);
	con->windows.algorithm_sw = win;
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		rad = radio_new(reg, primitive_box_sw, 0, 0);
		event_new(rad, "radio-changed", 0, config_apply,
		    "%i", AL_BOX_RADIO);
		
		rad = radio_new(reg, primitive_frame_sw, 0, 0);
		event_new(rad, "radio-changed", 0, config_apply,
		    "%i", AL_FRAME_RADIO);
		
		rad = radio_new(reg, primitive_circle_sw, 0, 0);
		event_new(rad, "radio-changed", 0, config_apply,
		    "%i", AL_CIRCLE_RADIO);
		
		rad = radio_new(reg, primitive_line_sw, 0, 0);
		event_new(rad, "radio-changed", 0, config_apply,
		    "%i", AL_LINE_RADIO);
		
		rad = radio_new(reg, primitive_square_sw, 0, 0);
		event_new(rad, "radio-changed", 0, config_apply,
		    "%i", AL_SQUARE_RADIO);
	}
}

#define CONFIG_SET_FLAG(con, _field, flag, val) do {	\
	pthread_mutex_lock(&(con)->lock);		\
	if ((val)) {					\
		(con)->_field |= (flag);		\
	} else {					\
		(con)->_field &= ~(flag);		\
	}						\
	pthread_mutex_unlock(&(con)->lock);		\
} while (/*CONSTCOND*/ 0)

#define CONFIG_SET_STRING(con, _field, text) do {	\
	pthread_mutex_lock(&(con)->lock);		\
	free((con)->_field);				\
	(con)->_field = strdup((text));			\
	pthread_mutex_unlock(&(con)->lock);		\
} while (/*CONSTCOND*/ 0)
			
#define CONFIG_SET_INT(con, _field, i) do {		\
	pthread_mutex_lock(&(con)->lock);		\
	(con)->_field = (i);				\
	pthread_mutex_unlock(&(con)->lock);		\
} while (/*CONSTCOND*/ 0)

void
config_apply(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct textbox *tbox = argv[0].p;

	switch (argv[1].i) {
	case CLOSE_BUTTON:
		window_hide_locked(wid->win);
		return;
	case SAVE_BUTTON:
		object_save(config);
		return;
	}

	switch (argv[1].i) {
	case UDATADIR_TBOX:
		CONFIG_SET_STRING(config, path.user_data_dir, tbox->text);
		break;
	case SYSDATADIR_TBOX:
		CONFIG_SET_STRING(config, path.sys_data_dir, tbox->text);
		break;
	case DATAPATH_TBOX:
		CONFIG_SET_STRING(config, path.data_path, tbox->text);
		break;
	case W_TBOX:
		CONFIG_SET_INT(config, view.w, atoi(tbox->text));
		break;
	case H_TBOX:
		CONFIG_SET_INT(config, view.h, atoi(tbox->text));
		break;
	case FONTCACHE_CBOX:
		CONFIG_SET_FLAG(config, flags, CONFIG_FONT_CACHE, argv[2].i);
		if (argv[2].i) {
			keycodes_loadglyphs();
		} else {
			keycodes_freeglyphs();
		}
		break;
	case FULLSCREEN_CBOX:
		CONFIG_SET_FLAG(config, flags, CONFIG_FULLSCREEN, argv[2].i);
		SDL_WM_ToggleFullScreen(view->v);
		VIEW_REDRAW();
		break;
	case ASYNCBLIT_CBOX:
		CONFIG_SET_FLAG(config, flags, CONFIG_ASYNCBLIT, argv[2].i);
		break;
#ifdef DEBUG
	case DEBUG_CBOX:
		engine_debug = argv[2].i;	/* XXX unsafe */
		break;
	case VISREGIONS_CBOX:
		CONFIG_SET_FLAG(config, widget_flags, CONFIG_REGION_BORDERS,
		    argv[2].i);
		break;
	case ANYSIZE_CBOX:
		CONFIG_SET_FLAG(config, widget_flags, CONFIG_WINDOW_ANYSIZE,
		    argv[2].i);
		break;
#endif
	case AL_BOX_RADIO:
	case AL_FRAME_RADIO:
	case AL_CIRCLE_RADIO:
	case AL_LINE_RADIO:
	case AL_SQUARE_RADIO:
		/* TODO */
		break;
	}
}

