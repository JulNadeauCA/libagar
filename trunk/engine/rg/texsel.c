/*	$Csoft: texsel.c,v 1.25 2005/05/24 08:12:48 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/view.h>

#include "texsel.h"

struct texsel *
texsel_new(void *parent, struct tileset *tset, int flags)
{
	struct texsel *ts;

	ts = Malloc(sizeof(struct texsel), M_OBJECT);
	texsel_init(ts, tset, flags);
	object_attach(parent, ts);
	return (ts);
}

static void
poll_textures(int argc, union evarg *argv)
{
	struct texsel *ts = argv[0].p;
	struct tlist *tl = (struct tlist *)ts;
	struct texture *tex;
	struct tlist_item *it;

	tlist_clear_items(tl);

	TAILQ_FOREACH(tex, &ts->tset->textures, textures) {
		struct pixmap *px;

		if (tex->tileset[0] != '\0' && tex->pixmap[0] != '\0' &&
		    (px = tileset_resolve_pixmap(tex->tileset, tex->pixmap))
		     != NULL) {
			it = tlist_insert(tl, px->su, "%s (<%s> %ux%u)",
			    tex->name, px->name, px->su->w, px->su->h);
			it->class = "texture";
			it->p1 = tex;
		}
	}
	tlist_restore_selections(tl);
}

static void
select_texture(int argc, union evarg *argv)
{
	struct texsel *ts = argv[0].p;
	struct tlist *tl = (struct tlist *)ts;
	struct tlist_item *it;
	struct texture *tex;
	struct widget_binding *bTexname;
	char *texname;

	bTexname = widget_get_binding(ts, "texture-name", &texname);
	if ((it = tlist_selected_item(tl)) != NULL) {
		tex = it->p1;
		strlcpy(texname, tex->name, bTexname->size);
	}
	widget_binding_unlock(bTexname);
}

void
texsel_init(struct texsel *ts, struct tileset *tset, int flags)
{
	tlist_init(&ts->tl, TLIST_POLL);
	tlist_set_item_height(&ts->tl, TILESZ);
	event_new(&ts->tl, "tlist-poll", poll_textures, NULL);
	event_new(&ts->tl, "tlist-selected", select_texture, NULL);
	
	ts->tset = tset;
	ts->flags = flags;
	ts->texname[0] = '\0';

	widget_bind(ts, "texture-name", WIDGET_STRING, ts->texname,
	    sizeof(ts->texname));
}

void
texsel_destroy(void *p)
{
	struct texsel *ts = p;

	tlist_destroy(ts);
}

