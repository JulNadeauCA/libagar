/*	$Csoft$	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/version.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "radio.h"

static struct widget_ops radio_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	radio_draw,
	NULL		/* animate */
};

static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */

static void	radio_event(int, union evarg *);

struct radio *
radio_new(struct region *reg, const char **items, int selitem, int flags)
{
	struct radio *rad;

	rad = emalloc(sizeof(struct radio));
	radio_init(rad, items, selitem, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, rad);
	pthread_mutex_unlock(&reg->win->lock);

	return (rad);
}

void
radio_init(struct radio *rad, const char **items, int selitem, int flags)
{
	const char *s;
	int maxw;

	widget_init(&rad->wid, "radio", "widget", &radio_ops, -1, -1);
	rad->flags = flags;
	rad->items = items;
	rad->selitem = 0;
	rad->xspacing = 6;
	rad->yspacing = 6;
	rad->radio.w = font_h;
	rad->radio.h = font_h;
	rad->justify = RADIO_LEFT;

	for (rad->nitems = 0, maxw = 0; (s = *items++) != NULL;) {
		SDL_Surface *surf;
	
		surf = TTF_RenderText_Solid(font, s, white);
		if (surf->w > maxw) {
			maxw = surf->w;
		}
		SDL_FreeSurface(surf);
		rad->nitems++;
	}

#ifdef DEBUG
	if (selitem > rad->nitems) {
		fatal("item %d > %d items\n", selitem, rad->nitems);
	}
#endif

	WIDGET(rad)->h = rad->nitems * (rad->yspacing + rad->radio.h);
	WIDGET(rad)->w = rad->radio.w + rad->xspacing + maxw;

	event_new(rad, "window-mousebuttonup", 0,
	    radio_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(rad, "window-mousebuttondown", 0,
	    radio_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(rad, "window-keyup", 0,
	    radio_event, "%i", WINDOW_KEYUP);
	event_new(rad, "window-keydown", 0,
	    radio_event, "%i", WINDOW_KEYDOWN);
}

void
radio_draw(void *p)
{
	struct radio *rad = p;
	SDL_Surface *su;
	int x = 0, y = 0, i;
	
	su = primitive_frame(rad, WIDGET(rad)->w, WIDGET(rad)->h,
	    WIDGET_FOCUSED(rad) ? 1 : 0);
	WIDGET_DRAW(rad, su, 0, 0);
	SDL_FreeSurface(su);

	for (i = 0; i < rad->nitems; i++) {
		const char *s;
		SDL_Surface *ls;

		s = rad->items[i];
		/* Radio button */
		su = primitive_circle(rad, rad->radio.w, rad->radio.h, 5, 1);
		WIDGET_DRAW(rad, su, 0, (i * rad->radio.h) + rad->yspacing);
		SDL_FreeSurface(su);

		if (rad->selitem == i) {
			su = primitive_circle(rad, rad->radio.w, rad->radio.h,
			    3, 1);
			WIDGET_DRAW(rad, su, 0, (i * rad->radio.h) +
			    rad->yspacing);
			SDL_FreeSurface(su);
			
			su = primitive_circle(rad, rad->radio.w, rad->radio.h,
			    2, 1);
			WIDGET_DRAW(rad, su, 0, (i * rad->radio.h) +
			    rad->yspacing);
			SDL_FreeSurface(su);
		}

		ls = TTF_RenderText_Solid(font, s, white);
		WIDGET_DRAW(rad, ls, rad->radio.w,
		    (i * rad->radio.h) + rad->yspacing);
		SDL_FreeSurface(ls);
	}
}

static void
radio_event(int argc, union evarg *argv)
{
	struct radio *rad = argv[0].p;
	int type = argv[1].i;
	int button, keysym;
	int x, y, sel = rad->selitem;

	OBJECT_ASSERT(argv[0].p, "widget");

	switch (type) {
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		x = argv[3].i;
		y = argv[4].i;
	
		sel = y / rad->radio.h;

		WIDGET_FOCUS(rad);
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		switch ((SDLKey)keysym) {
		case SDLK_DOWN:
			if (++sel > rad->nitems) {
				sel = 0;
			}
			break;
		case SDLK_UP:
			if (--sel < 0) {
				sel = 0;
			}
			break;
		default:
		}
		break;
	default:
		return;
	}

	if (sel >= rad->nitems) {
		sel = rad->nitems - 1;
	}

	rad->selitem = sel;
	event_post(rad, "radio-changed", "%c, %i", '*', sel);
}

