/*	$Csoft: widget.c,v 1.19 2002/07/21 10:58:18 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "widget.h"
#include "window.h"

void
widget_init(struct widget *wid, char *name, char *style, const void *wops,
    int rw, int rh)
{
	static Uint32 widid = 0;
	char *widname;

	/* Prepend parent window's name. */
	widname = object_name(name, widid++);
	object_init(&wid->obj, "widget", widname, style,
	    OBJ_ART|OBJ_KEEPMEDIA, wops);
	free(widname);

	wid->type = strdup(name);
	wid->flags = 0;
	wid->win = NULL;
	wid->x = 0;
	wid->y = 0;
	wid->rw = rw;
	wid->rh = rh;
	wid->w = 0;
	wid->h = 0;
	wid->ncolors = 0;
	SLIST_INIT(&wid->colors);
}

void
widget_map_color(void *p, int ind, char *name, Uint8 r, Uint8 g, Uint8 b,
    Uint8 a)
{
	struct widget *wid = p;
	struct widget_color *col;
	
	OBJECT_ASSERT(wid, "widget");
	
	if (ind > WIDGET_MAXCOLORS)
		fatal("%d colors > %d\n", ind, WIDGET_MAXCOLORS);
	if (ind > wid->ncolors)
		wid->ncolors++;

	wid->color[ind] = SDL_MapRGBA(view->v->format, r, g, b, a);

	col = emalloc(sizeof(struct widget_color));
	col->name = strdup(name);
	col->ind = ind;
	SLIST_INSERT_HEAD(&wid->colors, col, colors);
}

