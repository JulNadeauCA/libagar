/*	$Csoft: merge.c,v 1.52 2004/03/30 15:56:53 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/mapedit/mapedit.h>

#include <engine/widget/hbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/toolbar.h>

#include <string.h>

const struct version merge_ver = {
	"agar merge tool",
	6, 0
};

static void merge_init(struct tool *);
static void merge_destroy(struct tool *);
static int merge_load(struct tool *, struct netbuf *);
static int merge_save(struct tool *, struct netbuf *);
static int merge_cursor(struct tool *, SDL_Rect *);
static void merge_effect(struct tool *, struct node *);

static void merge_create_brush(int, union evarg *);
static void merge_edit_brush(int, union evarg *);
static void merge_remove_brush(int, union evarg *);
static void merge_interpolate(struct map *, struct node *,
	                      struct noderef *, struct map *, struct node *,
			      struct noderef *);

const struct tool merge_tool = {
	N_("Merge tool"),
	N_("Apply patterns, interpolating edge tiles."),
	MERGE_TOOL_ICON,
	-1,
	merge_init,
	merge_destroy,
	merge_load,
	merge_save,
	merge_cursor,
	merge_effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

static TAILQ_HEAD(, object) brushes = TAILQ_HEAD_INITIALIZER(brushes);
static struct tlist *brushes_tl;

enum {
	NODE, FILL,
	EDNW, EDNO, EDNE,
	EDWE, EDEA,
	EDSW, EDSO, EDSE
};

static const int
merge_table[9][9] = {
       /* nw    n     ne    w     fill  e     sw    s     se */
	{ EDNW, EDNO, EDNO, EDWE, NODE, FILL, FILL, FILL, FILL }, /* nw */
	{ EDNO, EDNO, EDNO, FILL, NODE, FILL, FILL, FILL, FILL }, /* n */
	{ EDNO, EDNO, NODE, 0   , NODE, EDEA, FILL, FILL, EDEA }, /* ne */
	{ EDWE, FILL, FILL, EDWE, NODE, FILL, EDWE, FILL, FILL }, /* w */
	{ FILL, FILL, FILL, FILL, NODE, FILL, FILL, FILL, FILL }, /* fill */
	{ FILL, FILL, EDEA, FILL, NODE, EDEA, FILL, FILL, EDEA }, /* e */
	{ EDWE, 0,    EDEA, EDWE, NODE, 0,    EDSW, EDSO, EDSO }, /* sw */
	{ FILL, FILL, FILL, FILL, NODE, FILL, EDSO, EDSO, EDSO }, /* s */
	{ EDWE, 0,    EDEA, EDNO, NODE, EDNO, EDSO, EDSO, EDSE }  /* se */
};

static void
merge_init(struct tool *t)
{
	struct window *win;
	struct hbox *hb;
	struct button *bu;
	struct textbox *tb;

	win = tool_window(t, "mapedit-tool-merge");

	hb = hbox_new(win, HBOX_WFILL);
	{
		tb = textbox_new(hb, _("Name: "));
		event_new(tb, "textbox-return", merge_create_brush, "%p", tb);

		bu = button_new(hb, _("Create"));
		button_set_padding(bu, 6);			/* Align */
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", merge_create_brush, "%p", tb);
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	{
#if 0
		bu = button_new(hb, _("Load set"));
		event_new(bu, "button-pushed", merge_load_brush_set, NULL);
		bu = button_new(hb, _("Save set"));
		event_new(bu, "button-pushed", merge_save_brush_set, NULL);
#endif
		bu = button_new(hb, _("Edit"));
		event_new(bu, "button-pushed", merge_edit_brush, NULL);
		bu = button_new(hb, _("Remove"));
		event_new(bu, "button-pushed", merge_remove_brush, NULL);
	}

	brushes_tl = tlist_new(win, TLIST_MULTI);
}

static void
merge_free_brushes(void)
{
	struct object *ob, *nob;

	for (ob = TAILQ_FIRST(&brushes);
	     ob != TAILQ_END(&brushes);
	     ob = nob) {
		nob = TAILQ_NEXT(ob, cobjs);
		object_destroy(ob);
		Free(ob, M_OBJECT);
	}
	TAILQ_INIT(&brushes);
}

static void
merge_destroy(struct tool *t)
{
	merge_free_brushes();
}

static void
merge_create_brush(int argc, union evarg *argv)
{
	char brush_name[OBJECT_NAME_MAX];
	char m_name[OBJECT_NAME_MAX];
	struct textbox *name_tbox = argv[1].p;
	struct map *m;

	textbox_copy_string(name_tbox, brush_name,
	    sizeof(brush_name) - sizeof("brush()"));
	if (brush_name[0] == '\0') {
		text_msg(MSG_ERROR, _("No brush name was given."));
		return;
	}
	
	snprintf(m_name, sizeof(m_name), "brush(%s)", brush_name);
	if (tlist_item_text(brushes_tl, m_name) != NULL) {
		text_msg(MSG_ERROR, _("A `%s' brush exists."), m_name);
		return;
	}

	m = map_new(NULL, m_name);
	if (object_load(m) == -1) {
		if (map_alloc_nodes(m,
		    prop_get_uint32(&mapedit, "default-brush-width"),
		    prop_get_uint32(&mapedit, "default-brush-height")) == -1) {
			text_msg(MSG_ERROR, "map_alloc_nodes: %s", error_get());
			goto fail;
		}
	}

	TAILQ_INSERT_HEAD(&brushes, OBJECT(m), cobjs);
	tlist_unselect_all(brushes_tl);
	tlist_select(brushes_tl, tlist_insert_item_head(brushes_tl, NULL,
	    m_name, m));
	textbox_printf(name_tbox, " ");
	return;
fail:
	object_destroy(m);
	Free(m, M_OBJECT);
}

static void
merge_edit_brush(int argc, union evarg *argv)
{
	struct tlist_item *it;

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		struct map *brush = it->p1;
		struct window *win;
		struct toolbar *tbar;

		if (!it->selected)
			continue;

		if ((win = window_new("mapedit-tool-merge-%s",
		    OBJECT(brush)->name)) == NULL)
			continue;

		tbar = toolbar_new(win, TOOLBAR_HORIZ, 1);
		mapview_new(win, brush, MAPVIEW_EDIT|MAPVIEW_GRID|
		                        MAPVIEW_PROPS, tbar);
		window_show(win);
	}
}

static void
merge_remove_brush(int argc, union evarg *argv)
{
	struct tlist_item *it, *nit;

	for (it = TAILQ_FIRST(&brushes_tl->items);
	     it != TAILQ_END(&brushes_tl->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		if (it->selected) {
			char wname[OBJECT_NAME_MAX];
			struct object *brush = it->p1;
			struct window *win;

			snprintf(wname, sizeof(wname),
			    "win-mapedit-tool-merge-%s", OBJECT(brush)->name);
			if ((win = view_window_exists(wname)) != NULL) {
				window_hide(win);
				view_detach(win);
			}

			TAILQ_REMOVE(&brushes, brush, cobjs);
			tlist_remove_item(brushes_tl, it);
			object_destroy(brush);
			Free(brush, M_OBJECT);
		}
	}
}

static void
merge_interpolate(struct map *sm, struct node *sn, struct noderef *sr,
    struct map *dm, struct node *dn, struct noderef *dr)
{
	int sedge = sr->r_gfx.edge;
	int dedge = dr->r_gfx.edge;
	int op = merge_table[sedge][dedge];
	struct noderef *r;
	int x, y;

	dprintf("interp %d <-> %d\n", sedge, dedge);

	switch (op) {
	case NODE:
		node_copy_ref(sr, dm, dn, dm->cur_layer);
		break;
	case FILL:
		for (y = 0; y < sm->maph; y++) {
			for (x = 0; x < sm->mapw; x++) {
				struct node *fn = &sm->map[y][x];
			
				/* XXX randomize/increment */
				TAILQ_FOREACH(r, &fn->nrefs, nrefs) {
					if (r->r_gfx.edge != 0) {
						continue;
					}
					node_copy_ref(r, dm, dn, dm->cur_layer);
				}
				return;
			}
		}
		break;
	case EDNW:
	case EDNO:
	case EDNE:
	case EDWE:
	case EDEA:
	case EDSW:
	case EDSO:
	case EDSE:
		/* Copy the first edge with the requested orientation. */
		for (y = 0; y < sm->maph; y++) {
			for (x = 0; x < sm->mapw; x++) {
				struct node *en = &sm->map[y][x];

				/* XXX randomize/increment */
				TAILQ_FOREACH(r, &en->nrefs, nrefs) {
					if (r->r_gfx.edge != op) {
						continue;
					}
					node_copy_ref(r, dm, dn, dm->cur_layer);
				}
				return;
			}
		}
		break;
	}
}

static void
merge_effect(struct tool *t, struct node *n)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	struct tlist_item *it;
	
	/* Avoid circular references. XXX ugly */
	if (strncmp(OBJECT(m)->name, "brush(", 6) == 0)
		return;
	
	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		struct map *sm;
		int sx, sy, dx, dy;

		if (!it->selected)
			continue;

		sm = it->p1;
		for (sy = 0, dy = mv->cy - sm->maph/2;
		     sy < sm->maph && dy < m->maph;
		     sy++, dy++) {
			for (sx = 0, dx = mv->cx - sm->mapw/2;
			     sx < sm->mapw && dx < m->mapw;
			     sx++, dx++) {
				struct node *sn = &sm->map[sy][sx];
				struct node *dn = &m->map[dy][dx];
				struct noderef *sr, *dr;

				TAILQ_FOREACH(sr, &sn->nrefs, nrefs) {
					TAILQ_FOREACH(dr, &dn->nrefs, nrefs) {
						merge_interpolate(sm, sn, sr,
						    m, dn, dr);
					}
				}
			}
		}
	}
}

static int
merge_load(struct tool *t, struct netbuf *buf)
{
	Uint32 i, nbrushes;
	
	if (version_read(buf, &merge_ver, NULL) != 0)
		return (-1);
	merge_free_brushes();
	tlist_clear_items(brushes_tl);

	nbrushes = read_uint32(buf);
	for (i = 0; i < nbrushes; i++) {
		char m_name[OBJECT_NAME_MAX];
		struct map *nbrush;

		copy_string(m_name, buf, sizeof(m_name));
		nbrush = Malloc(sizeof(struct map), M_OBJECT);
		map_init(nbrush, m_name);
		map_load(nbrush, buf);

		TAILQ_INSERT_HEAD(&brushes, OBJECT(nbrush), cobjs);
		tlist_insert_item_head(brushes_tl, NULL, m_name, nbrush);
	}
	return (0);
}

static int
merge_save(struct tool *t, struct netbuf *buf)
{
	struct object *ob;
	Uint32 nbrushes = 0;
	off_t count_offs;

	version_write(buf, &merge_ver);

	count_offs = netbuf_tell(buf);				/* Skip count */
	write_uint32(buf, 0);	
	TAILQ_FOREACH(ob, &brushes, cobjs) {
		struct brush *brush = (struct brush *)ob;

		write_string(buf, ob->name);
		map_save(brush, buf);
		nbrushes++;
	}
	pwrite_uint32(buf, nbrushes, count_offs);	/* Write count */
	return (0);
}

static int
merge_cursor(struct tool *t, SDL_Rect *rd)
{
	struct mapview *mv = t->mv;
	struct noderef *r;
	struct map *sm;
	int sx, sy, dx, dy;
	struct tlist_item *it;
	int rv = -1;
	
	/* XXX ugly work around circular ref */
	if (strncmp(OBJECT(mv->map)->name, "brush(", 6) == 0)
		return (-1);

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		if (!it->selected)
			continue;
		sm = it->p1;
		for (sy = 0, dy = rd->y-(sm->maph * mv->map->tilesz)/2;
		     sy < sm->maph;
		     sy++, dy += mv->map->tilesz) {
			for (sx = 0, dx = rd->x-(sm->mapw * mv->map->tilesz)/2;
			     sx < sm->mapw;
			     sx++, dx += mv->map->tilesz) {
				struct node *sn = &sm->map[sy][sx];

				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					noderef_draw(mv->map, r,
					    WIDGET(mv)->cx+dx,
					    WIDGET(mv)->cy+dy);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS) {
					mapview_draw_props(mv, sn, dx, dy,
					    -1, -1);
				}
			}
		}
	}
	return (rv);
}
