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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/version.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "tilestack.h"

static const struct widget_ops tilestack_ops = {
	{
		NULL,	/* destroy */
		NULL,	/* load */
		NULL	/* save */
	},
	tilestack_draw,
	NULL		/* animate */
};

static void	tilestack_scaled(int, union evarg *);

struct tilestack *
tilestack_new(struct region *reg, int flags, int rw, int rh)
{
	struct tilestack *tilestack;

	tilestack = emalloc(sizeof(struct tilestack));
	tilestack_init(tilestack, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, tilestack);
	pthread_mutex_unlock(&reg->win->lock);

	return (tilestack);
}

void
tilestack_init(struct tilestack *ts, int flags, int rw, int rh)
{
	widget_init(&ts->wid, "tilestack", "widget", &tilestack_ops, rw, rh);
	ts->offs = 0;
	ts->flags = (flags != 0) ? flags : TILESTACK_VERT;
	
	event_new(ts, "window-widget-scaled", 0, tilestack_scaled, NULL);
}

static void
tilestack_scaled(int argc, union evarg *argv)
{
	struct tilestack *ts = argv[0].p;

	dprintf("scaled\n");
}

void
tilestack_draw(void *p)
{
	struct tilestack *ts = p;

}

