/*	$Csoft: config.c,v 1.26 2002/07/22 05:50:38 vedge Exp $	    */

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "engine.h"
#include "map.h"
#include "physics.h"
#include "input.h"
#include "version.h"
#include "config.h"

#include "widget/text.h"
#include "widget/widget.h"
#include "widget/window.h"
#include "widget/label.h"
#include "widget/button.h"
#include "widget/checkbox.h"
#include "widget/textbox.h"
#include "widget/keycodes.h"

static const struct version config_ver = {
	"agar config",
	2, 0
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
	W_TBOX,
	H_TBOX
};

#define CONFIG_DEFAULT_WIDTH 	800
#define CONFIG_DEFAULT_HEIGHT 	600
#define CONFIG_DEFAULT_BPP 	32
#define CONFIG_DEFAULT_FLAGS	(CONFIG_FONT_CACHE)

static struct window	*config_settings_win(struct config *);
static void		 apply(int, union evarg *);

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
	object_init(&con->obj, "engine-config", "config", NULL, 0, &config_ops);
	con->flags = CONFIG_FONT_CACHE;
	con->view.w = CONFIG_DEFAULT_WIDTH;
	con->view.h = CONFIG_DEFAULT_HEIGHT;
	con->view.bpp = CONFIG_DEFAULT_BPP;
	con->widget_flags = 0;
	pthread_mutex_init(&con->lock, NULL);
}

void
config_window(struct config *con)
{
	con->settings_win = config_settings_win(con);
}

void
config_destroy(void *p)
{
	struct config *con = p;

	pthread_mutex_destroy(&con->lock);
}

int
config_load(void *p, int fd)
{
	struct config *con = p;

	pthread_mutex_lock(&con->lock);

	version_read(fd, &config_ver);
	config->flags = fobj_read_uint32(fd);
	config->view.w = fobj_read_uint32(fd);
	config->view.h = fobj_read_uint32(fd);
	config->view.bpp = fobj_read_uint32(fd);
	config->widget_flags = fobj_read_uint32(fd);

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
	fobj_write_uint32(fd, con->flags);
	fobj_write_uint32(fd, con->view.w);
	fobj_write_uint32(fd, con->view.h);
	fobj_write_uint32(fd, con->view.bpp);
	fobj_write_uint32(fd, con->widget_flags);
	pthread_mutex_unlock(&con->lock);
	
	dprintf("saved settings (flags=0x%x)\n", config->flags);
	return (0);
}

static struct window *
config_settings_win(struct config *con)
{
	struct window *win;
	struct region *reg;
	struct button *close_button, *save_button, *debug_button;
	struct textbox *udatadir_tbox, *sysdatadir_tbox, *w_tbox, *h_tbox;
	struct checkbox *fontcache_cbox, *visregions_cbox, *fullscreen_cbox,
	    *showres_cbox;
#ifdef DEBUG
	struct checkbox *debug_cbox;
#endif

	/* Settings window */
	win = window_new("Engine settings", WINDOW_CENTER,
	    0, 0,
	    382, 351,
	    382, 351);

	/*
	 * Flags
	 */
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 40);

	/* Async blit */
	fontcache_cbox = checkbox_new(reg, "Asynchronous blits (restart)", 0,
	    (con->flags & CONFIG_ASYNCBLIT) ? CHECKBOX_PRESSED : 0);
	event_new(fontcache_cbox, "checkbox-changed", 0, apply,
	    "%i", ASYNCBLIT_CBOX);
	/* Full screen */
	fullscreen_cbox = checkbox_new(reg, "Full screen", 0,
	    (con->flags & CONFIG_FULLSCREEN) ? CHECKBOX_PRESSED : 0);
	event_new(fullscreen_cbox, "checkbox-changed", 0, apply,
	    "%i", FULLSCREEN_CBOX);
	/* Font cache */
	fontcache_cbox = checkbox_new(reg, "Font cache", 0,
	    (con->flags & CONFIG_FONT_CACHE) ? CHECKBOX_PRESSED : 0);
	event_new(fontcache_cbox, "checkbox-changed", 0, apply,
	    "%i", FONTCACHE_CBOX);
	/* Debugging */
#ifdef DEBUG
	debug_cbox = checkbox_new(reg, "Debugging enabled", 0,
	    engine_debug ? CHECKBOX_PRESSED : 0);
	event_new(debug_cbox, "checkbox-changed", 0, apply,
	    "%i", DEBUG_CBOX);
	visregions_cbox = checkbox_new(reg, "Visible regions", 0,
	    (con->widget_flags & CONFIG_REGION_BORDERS) ? CHECKBOX_PRESSED : 0);
	event_new(visregions_cbox, "checkbox-changed", 0, apply,
	    "%i", VISREGIONS_CBOX);
	showres_cbox = checkbox_new(reg, "Arbitrary window sizes", 0,
	    (con->widget_flags & CONFIG_WINDOW_ANYSIZE) ? CHECKBOX_PRESSED : 0);
	event_new(showres_cbox, "checkbox-changed", 0, apply,
	    "%i", ANYSIZE_CBOX);
#endif

	/*
	 * Directories
	 */
	reg = region_new(win, REGION_VALIGN,  0, 45, 100, 18);

	/* Data directories */
	udatadir_tbox = textbox_new(reg, "  User datadir: ", 0, 100, 50);
	event_new(udatadir_tbox, "textbox-changed", 0, apply,
	    "%i", UDATADIR_TBOX);
	sysdatadir_tbox = textbox_new(reg, "System datadir: ", 0, 100, 50);
	event_new(sysdatadir_tbox, "textbox-changed", 0, apply,
	    "%i", SYSDATADIR_TBOX);
	
	/*
	 * Resolution
	 */
	reg = region_new(win, REGION_HALIGN,  0, 70, 100, 10);

	w_tbox = textbox_new(reg, "Width : ", 0, 50, 100);
	event_new(w_tbox, "textbox-changed", 0, apply,
	    "%i", W_TBOX);
	h_tbox = textbox_new(reg, "Height: ", 0, 50, 100);
	event_new(h_tbox, "textbox-changed", 0, apply,
	    "%i", H_TBOX);
	
	/*
	 * Buttons
	 */
	reg = region_new(win, REGION_HALIGN, 0,  90, 100, 10);

	/* Close button */
	close_button = button_new(reg, "Close", NULL, 0, 50, 90);
	event_new(close_button, "button-pushed", 0, apply,
	    "%i", CLOSE_BUTTON);
	/* Save button */
	save_button = button_new(reg, "Save", NULL, 0, 50, 90);
	event_new(save_button, "button-pushed", 0, apply,
	    "%i", SAVE_BUTTON);
	win->focus = WIDGET(close_button);
	
	pthread_mutex_lock(&win->lock);
	textbox_printf(udatadir_tbox, "%s", world->udatadir);
	textbox_printf(sysdatadir_tbox, "%s", world->sysdatadir);
	textbox_printf(w_tbox, "%d", con->view.w);
	textbox_printf(h_tbox, "%d", con->view.h);
	pthread_mutex_unlock(&win->lock);

	return (win);
}

#define CONFIG_SETFLAG(con, _field, flag, val) do {	\
	pthread_mutex_lock(&(con)->lock);		\
	if ((val)) {					\
		(con)->_field |= (flag);		\
	} else {					\
		(con)->_field &= ~(flag);		\
	}						\
	pthread_mutex_unlock(&(con)->lock);		\
} while (/*CONSTCOND*/ 0)

static void
apply(int argc, union evarg *argv)
{
	struct textbox *tbox;
	struct widget *wid = argv[0].p;

	switch (argv[1].i) {
	case CLOSE_BUTTON:
		window_hide_locked(wid->win);
		break;
	case SAVE_BUTTON:
		object_save(config);
		break;
	case UDATADIR_TBOX:
	case SYSDATADIR_TBOX:
		/* XXX */
		break;
	case W_TBOX:
		tbox = (struct textbox *)wid;
		pthread_mutex_lock(&config->lock);
		config->view.w = atoi(tbox->text);
		pthread_mutex_unlock(&config->lock);
		break;
	case H_TBOX:
		tbox = (struct textbox *)wid;
		pthread_mutex_lock(&config->lock);
		config->view.h = atoi(tbox->text);
		pthread_mutex_unlock(&config->lock);
		break;
	case FONTCACHE_CBOX:
		CONFIG_SETFLAG(config, flags, CONFIG_FONT_CACHE, argv[2].i);
		if (argv[2].i) {
			keycodes_loadglyphs();
		} else {
			keycodes_freeglyphs();
		}
		break;
	case FULLSCREEN_CBOX:
		CONFIG_SETFLAG(config, flags, CONFIG_FULLSCREEN, argv[2].i);
		SDL_WM_ToggleFullScreen(view->v);
		VIEW_REDRAW();
		break;
	case ASYNCBLIT_CBOX:
		CONFIG_SETFLAG(config, flags, CONFIG_ASYNCBLIT, argv[2].i);
		break;
#ifdef DEBUG
	case DEBUG_CBOX:
		engine_debug = argv[2].i;	/* XXX unsafe */
		break;
	case VISREGIONS_CBOX:
		CONFIG_SETFLAG(config, widget_flags, CONFIG_REGION_BORDERS,
		    argv[2].i);
		break;
	case ANYSIZE_CBOX:
		CONFIG_SETFLAG(config, widget_flags, CONFIG_WINDOW_ANYSIZE,
		    argv[2].i);
		break;
#endif
	}
}

