/*	$Csoft: sprite_browser.c,v 1.4 2002/11/16 00:57:40 vedge Exp $	*/

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

#include <engine/engine.h>

#ifdef DEBUG

#include <engine/world.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/bitmap.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static void	lookup_object(int, union evarg *);

struct window *
sprite_browser_window(void)
{
	struct window *win;
	struct region *reg;
	struct textbox *obj_tbox, *offs_tbox;
	struct button *button;
	struct bitmap *bmp;

	if ((win = window_generic_new(184, 228, "monitor-sprite-browser"))
	    == NULL) {
		return (NULL);	/* Exists */
	}
	window_set_caption(win, "Sprite browser");

	/* Input */
	reg = region_new(win, REGION_VALIGN, 0, 0, 80, 30);
	obj_tbox = textbox_new(reg, "Object: ", 0, 100, 50);
	WIDGET_FOCUS(obj_tbox);
	offs_tbox = textbox_new(reg, "Offset: ", 0, 100, 50);
	reg = region_new(win, REGION_VALIGN, 80, 0, 20, 30);
	button = button_new(reg, "Update", NULL, 0, 100, 100);

	/* Display */
	reg = region_new(win, REGION_VALIGN, 0, 30, 80, 70);
	bmp = bitmap_new(reg, NULL, 100, 100);
	
	event_new(button, "button-pushed",
	    lookup_object, "%p, %p, %p", obj_tbox, offs_tbox, bmp);

	return (win);
}

static void
lookup_object(int argc, union evarg *argv)
{
	struct textbox *obj_tbox = argv[1].p;
	struct textbox *offs_tbox = argv[2].p;
	struct bitmap *bmp = argv[3].p;
	struct object *ob;
	char *name, *cause;
	Uint32 offs;

	name = textbox_string(obj_tbox);
	offs = (Uint32)textbox_int(offs_tbox);

	pthread_mutex_lock(&world->lock);

	ob = world_find(name);
	if (ob == NULL) {
		cause = "Unattached object";
		goto err;
	}
	if (ob->art == NULL) {
		cause = "Object has no art";
		goto err;
	}

	if (offs > ob->art->nsprites) {
		cause = "Bad offset";
		goto err;
	}
	bmp->surface = SPRITE(ob, offs);
	pthread_mutex_unlock(&world->lock);

	window_resize(WIDGET(bmp)->win);
	free(name);
	return;
err:
	bmp->surface = text_render(NULL, -1,
	    SDL_MapRGB(view->v->format, 255, 255, 255), cause);
	window_resize(WIDGET(bmp)->win);
	free(name);
}

#endif	/* DEBUG */
