/*	$Csoft	    */

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
#include "widget/window.h"
#include "widget/widget.h"
#include "widget/label.h"
#include "widget/button.h"
#include "widget/checkbox.h"
#include "widget/textbox.h"
#include "widget/keycodes.h"

static const struct version config_ver = {
	"agar config",
	1, 0
};

static const struct object_ops config_ops = {
	config_destroy,
	config_load,
	config_save,
	NULL,		/* onattach */
	NULL,		/* ondetach */
	NULL,		/* attach */
	NULL		/* detach */
};

static const enum {
	CLOSE_BUTTON,
	SAVE_BUTTON,
	FULLSCRN_CBOX,
	FONTCACHE_CBOX
} widgets;

static struct window	*config_settings_win(struct config *);
static void		 apply(int, union evarg *);

static void
apply(int argc, union evarg *argv)
{
	switch (argv[1].i) {
	case CLOSE_BUTTON:
		window_hide(WIDGET(argv[0].p)->win);
		break;
	case SAVE_BUTTON:
		object_save(config);
		text_msg(4, TEXT_SLEEP, "Saved settings.\n");
		break;
	case FULLSCRN_CBOX:
		view_fullscreen(mainview, argv[2].i);
		break;
	case FONTCACHE_CBOX:
		pthread_mutex_lock(&config->lock);
		if (argv[2].i) {
			config->flags |= CONFIG_FONT_CACHE;
		} else {
			config->flags &= ~(CONFIG_FONT_CACHE);
		}
		config_apply(config);
		pthread_mutex_unlock(&config->lock);
		break;
	}
}

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
	config_apply(config);

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
	
	pthread_mutex_unlock(&con->lock);
	
	dprintf("saved settings (flags=0x%x)\n", config->flags);
	return (0);
}

static struct window *
config_settings_win(struct config *con)
{
	struct window *win;
	struct region *body_reg, *buttons_reg;
	struct button *close_button, *save_button;
	struct checkbox *fullscr_cbox;
	struct checkbox *fontcache_cbox;
	struct textbox *udatadir_tbox, *sysdatadir_tbox;

	/* Settings window */
	win = window_new("Engine settings", WINDOW_TITLEBAR, WINDOW_GRADIENT,
	    20, 20,  60, 60);
	body_reg = region_new(win, REGION_VALIGN|REGION_RIGHT,
	    0,  0,  100, 80);
	buttons_reg = region_new(win, REGION_HALIGN|REGION_CENTER,
	    0,  80, 100, 20);

	fullscr_cbox = checkbox_new(body_reg, "Full-screen mode", 0);
	event_new(fullscr_cbox, "checkbox-changed", 0, apply,
	    "%d", FULLSCRN_CBOX);
	
	fontcache_cbox = checkbox_new(body_reg, "Font cache",
	    (con->flags & CONFIG_FONT_CACHE) ?
	    CHECKBOX_PRESSED : 0);
	event_new(fontcache_cbox, "checkbox-changed", 0, apply,
	    "%d", FONTCACHE_CBOX);

	udatadir_tbox = textbox_new(body_reg,   "  User datadir: ", 0, 100);
	sysdatadir_tbox = textbox_new(body_reg, "System datadir: ", 0, 100);

	pthread_mutex_lock(&win->lock);
	textbox_printf(udatadir_tbox, "%s", world->udatadir);
	textbox_printf(sysdatadir_tbox, "%s", world->sysdatadir);
	pthread_mutex_unlock(&win->lock);

	close_button = button_new(buttons_reg, "Close", 0, 50, 100);
	event_new(close_button, "button-pushed", 0, apply,
	    "%d", CLOSE_BUTTON);

	save_button = button_new(buttons_reg, "Save", 0, 50, 100);
	event_new(save_button, "button-pushed", 0, apply,
	    "%d", SAVE_BUTTON);
	
	win->focus = WIDGET(udatadir_tbox);
	return (win);
}

/*
 * Apply configuration settings.
 * config structure must be locked.
 */
void
config_apply(struct config *con)
{
	if (con->flags & CONFIG_FONT_CACHE) {
		keycodes_loadglyphs();
	} else {
		keycodes_freeglyphs();
	}
}

