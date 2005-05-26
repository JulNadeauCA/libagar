/*	$Csoft: sketch.c,v 1.11 2005/05/24 05:34:32 vedge Exp $	*/

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
#include <engine/input.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/combo.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/radio.h>

#include "tileset.h"
#include "tileview.h"

void
texture_init(struct texture *tex, struct tileset *ts, const char *name)
{
	strlcpy(tex->name, name, sizeof(tex->name));
	tex->tileset[0] = '\0';
	tex->pixmap[0] = '\0';
	tex->wrap_s = TEXTURE_REPEAT;
	tex->wrap_t = TEXTURE_REPEAT;
	tex->flags = 0;
	tex->blend_func = ALPHA_OVERLAY;
}

void
texture_destroy(struct texture *tex)
{
}

int
texture_load(struct texture *tex, struct netbuf *buf)
{
	copy_string(tex->tileset, buf, sizeof(tex->tileset));
	copy_string(tex->pixmap, buf, sizeof(tex->pixmap));
	tex->flags = (int)read_uint32(buf);
	tex->wrap_s = (enum texture_wrap_mode)read_uint8(buf);
	tex->wrap_t = (enum texture_wrap_mode)read_uint8(buf);
	tex->blend_func = (enum view_blend_func)read_uint8(buf);
	tex->alpha = read_uint8(buf);
	return (0);
}

void
texture_save(struct texture *tex, struct netbuf *buf)
{
	write_string(buf, tex->tileset);
	write_string(buf, tex->pixmap);
	write_uint32(buf, (Uint32)tex->flags);
	write_uint8(buf, (Uint8)tex->wrap_s);
	write_uint8(buf, (Uint8)tex->wrap_t);
	write_uint8(buf, (Uint8)tex->blend_func);
	write_uint8(buf, tex->alpha);
}

static void
poll_tilesets(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts;

	tlist_clear_items(tl);
	lock_linkage();
	OBJECT_FOREACH_CHILD(ts, world, tileset) {
		if (!OBJECT_TYPE(ts, "tileset"))
			continue;
		
		tlist_insert_item(tl, ICON(TILESET_ICON),
		    OBJECT(ts)->name, ts);
	}
	unlock_linkage();
	tlist_restore_selections(tl);
}

static void
poll_pixmaps(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct texture *tex = argv[1].p;
	struct pixmap *px;
	struct tileset *ts;

	tlist_clear_items(tl);
	if (tex->tileset[0] != '\0' &&
	    (ts = object_find(tex->tileset)) != NULL &&
	    OBJECT_TYPE(ts, "tileset")) {
		TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
			tlist_insert_item(tl, px->su, px->name, px);
		}
	}
	tlist_restore_selections(tl);
}

static void
select_tileset(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct texture *tex = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct tileset *ts = it->p1;

	object_copy_name(ts, tex->tileset, sizeof(tex->tileset));
	tex->pixmap[0] = '\0';
}

static void
select_pixmap(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct texture *tex = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct pixmap *px = it->p1;

	strlcpy(tex->pixmap, px->name, sizeof(tex->pixmap));
}

static int
compare_pixmap_ents(const struct tlist_item *it1, const struct tlist_item *it2)
{
	return (0);
}

struct window *
texture_edit(struct texture *tex)
{
	const char *wrap_modes[] ={
		N_("Repeat"),
		N_("Clamp"),
		N_("Clamp to edge"),
		N_("Clamp to border"),
		NULL
	};
	struct window *win;
	struct box *bo;
	struct combo *com;
	struct tlist *tl;
	struct spinbutton *sb;
	struct notebook *nb;
	struct notebook_tab *ntab;
	struct radio *rad;
	
	win = window_new(0, NULL);
	window_set_caption(win, "%s", tex->name);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	com = combo_new(win, COMBO_POLL, _("Tileset: "));
	event_new(com->list, "tlist-poll", poll_tilesets, NULL);
	event_new(com, "combo-selected", select_tileset, "%p", tex);
	combo_select_text(com, tex->tileset);
	textbox_printf(com->tbox, "%s", tex->tileset);

	tl = tlist_new(win, TLIST_POLL);
	event_new(tl, "tlist-poll", poll_pixmaps, "%p", tex);
	event_new(tl, "tlist-selected", select_pixmap, "%p", tex);
	tlist_select_text(tl, tex->pixmap);

	nb = notebook_new(win, NOTEBOOK_WFILL);
	ntab = notebook_add_tab(nb, _("Wrapping"), BOX_VERT);
	{
		label_static(ntab, _("S-coordinate: "));
		rad = radio_new(ntab, wrap_modes);
		widget_bind(rad, "value", WIDGET_INT, &tex->wrap_s);
		
		label_static(ntab, _("T-coordinate: "));
		rad = radio_new(ntab, wrap_modes);
		widget_bind(rad, "value", WIDGET_INT, &tex->wrap_t);
	}

	ntab = notebook_add_tab(nb, _("Blending"), BOX_VERT);
	{
		label_static(ntab, _("Blending function: "));
		rad = radio_new(ntab, view_blend_func_txt);
		widget_bind(rad, "value", WIDGET_INT, &tex->blend_func);

		sb = spinbutton_new(ntab, _("Overall alpha: "));
		widget_bind(sb, "value", WIDGET_UINT8, &tex->alpha);
	}
	return (win);
}

