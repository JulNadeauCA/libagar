/*	$Csoft: widget.c,v 1.17 2002/05/25 08:21:53 vedge Exp $	*/

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

#include "window.h"
#include "widget.h"

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
}

