/*	$Csoft: objq.c,v 1.61 2003/04/12 01:45:38 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/asprintf.h>
#include <engine/engine.h>

#include <engine/map.h>
#include <engine/view.h>
#include <engine/world.h>

#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/text.h>
#include <engine/widget/window.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

enum {
	OBJQ_INSERT_LEFT,
	OBJQ_INSERT_RIGHT,
	OBJQ_INSERT_UP,
	OBJQ_INSERT_DOWN
};

static void
objq_update(struct tlist *tl)
{
	struct object *ob;

	tlist_clear_items(tl);
	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		SDL_Surface *icon = NULL;

		if (ob->art == NULL)
			continue;

		if (ob->art != NULL && ob->art->nsprites > 0)
			icon = ob->art->sprites[0];
		tlist_insert_item(tl, icon, ob->name, ob);
	}
	pthread_mutex_unlock(&world->lock);
	tlist_restore_selections(tl);
}

static void
objq_tmap_option(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int opt = argv[2].i;

	switch (opt) {
	case MAPEDIT_TOOL_GRID:
		if (mv->flags & MAPVIEW_GRID) {
			mv->flags &= ~(MAPVIEW_GRID);
		} else {
			mv->flags |= MAPVIEW_GRID;
		}
		break;
	case MAPEDIT_TOOL_PROPS:
		if (mv->flags & MAPVIEW_PROPS) {
			mv->flags &= ~(MAPVIEW_PROPS);
		} else {
			mv->flags |= MAPVIEW_PROPS;
		}
		break;
	case MAPEDIT_TOOL_EDIT:
		if (mv->flags & MAPVIEW_EDIT) {
			mv->flags &= ~(MAPVIEW_EDIT);
			window_hide(mv->constr.win);
		} else {
			mv->flags |= MAPVIEW_EDIT;
			window_show(mv->constr.win);
		}
		break;
	case MAPEDIT_TOOL_NODEEDIT:
		if (mv->nodeed.win->flags & WINDOW_SHOWN) {
			window_hide(mv->nodeed.win);
		} else {
			window_show(mv->nodeed.win);
		}
		break;
	}
}

static void
objq_insert_tiles(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct mapview *mv = argv[2].p;
	struct mapview_constr *con = &mv->constr;
	int mode = argv[3].i;
	struct tlist_item *it;
	struct map *m = mv->map;
	int sx, sy, dx, dy;

	if (!mv->esel.set) {
		text_msg("Error", "No selection");
		return;
	}

	con->x = mv->esel.x;
	con->y = mv->esel.y;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct node *node;
		struct noderef *nref, *nnref;
		struct object *pobj = it->p1;
		struct map *submap = it->p1;
		int t, xinc, yinc;
		Uint32 ind;
		unsigned int nw, nh;
 
		if (!it->selected || it->text_len < 2)
			continue;

		ind = (Uint32)atoi(it->text + 1);
		switch (it->text[0]) {
		case 's':
			{
				SDL_Surface *srcsu = SPRITE(pobj, ind);

				t = NODEREF_SPRITE;
				xinc = srcsu->w / TILEW;
				yinc = srcsu->h / TILEW;
			}
			break;
		case 'a':
			{
				struct object *pobj = it->p1;
				struct art_anim *anim = ANIM(pobj, ind);
				SDL_Surface *srcsu = anim->frames[0];

				t = NODEREF_ANIM;
				xinc = srcsu->w / TILEW;
				yinc = srcsu->h / TILEW;
			}
			break;
		case 'm':
			t = -1;
			xinc = submap->mapw;
			yinc = submap->maph;
			break;
		default:
			fatal("bad ref");
		}

		nw = con->x + xinc + 1;
		nh = con->y + yinc + 1;

		if (map_resize(m,
		    nw > m->mapw ? nw : m->mapw,
		    nh > m->maph ? nh : m->maph) == -1) {
			text_msg("Error growing map", "%s");
			continue;
		}

		switch (t) {
		case NODEREF_SPRITE:
			node = &m->map[con->y][con->x];
			if (con->replace) {
				node_init(node);
			}
			dprintf("+sprite: %s:%d\n", pobj->name, ind);
			nref = node_add_sprite(node, pobj, ind);
			nref->flags |= NODEREF_SAVEABLE;
			break;
		case NODEREF_ANIM:
			node = &m->map[con->y][con->x];
			if (con->replace) {
				node_init(node);
			}
			dprintf("+anim: %s:%d\n", pobj->name, ind);
			nref = node_add_anim(node, pobj, ind,
			    NODEREF_ANIM_AUTO);
			nref->flags |= NODEREF_SAVEABLE;
			break;
		case -1:					/* Submap */
			dprintf("+submap %u,%u\n", submap->mapw, submap->maph);
			for (sy = 0, dy = con->y;
			     sy < submap->maph && dy < m->maph;
			     sy++, dy++) {
				for (sx = 0, dx = con->x;
				     sx < submap->mapw && dx < m->mapw;
				     sx++, dx++) {
					struct node *srcnode =
					    &submap->map[sy][sx];
					struct node *dstnode = &m->map[dy][dx];

					if (con->replace) {
						node_init(dstnode);
					}
					TAILQ_FOREACH(nref, &srcnode->nrefs,
					    nrefs) {
						nnref = node_copy_ref(nref,
						    dstnode);
						nnref->flags |=
						    NODEREF_SAVEABLE;
					}
					dstnode->flags = srcnode->flags;
				}
			}
			break;
		default:
			fatal("bad nref");
		}

		switch (mode) {
		case OBJQ_INSERT_LEFT:
			if ((con->x -= xinc) < 0)
				con->x = 0;
			break;
		case OBJQ_INSERT_RIGHT:
			con->x += xinc;
			break;
		case OBJQ_INSERT_UP:
			if ((con->y -= yinc) < 0)
				con->y = 0;
			break;
		case OBJQ_INSERT_DOWN:
			con->y += yinc;
			break;
		}
	}
}

static void
objq_close_tmap(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct button *nodeedit_button = argv[2].p;

	if (mv->constr.win != NULL) {
		window_hide(mv->constr.win);
	}
	window_hide(mv->nodeed.win);
	window_hide(win);

	widget_set_int(nodeedit_button, "state", 0);
}

static void
tl_objs_toggle_replace(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int state = argv[2].i;

	mv->constr.replace = state;
}

static void
tl_objs_toggle_big(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	int big = argv[2].i;

	tlist_set_item_height(tl, big ? TILEH : text_font_height(font));
}

static void
tl_objs_selected(int argc, union evarg *argv)
{
	struct tlist_item *eob_item = argv[1].p;
	struct object *ob = eob_item->p1;
	struct window *win;
	struct region *reg;
	struct mapview *mv;
	struct button *bu;
	static int cury = 140;

	/* Create the tile map window. */
	win = window_generic_new(202, 365, "mapedit-tmap-%s", ob->name);
	if (win == NULL) {
		return;		/* Exists */
	}
	win->rd.x = view->w - 88;
	win->rd.y = cury;
	win->minw = 56;
	win->minh = 117;
	if ((cury += 32) + 264 > view->h) {
		cury = 140;
	}
	window_set_caption(win, "%s", ob->name);
	window_set_spacing(win, 1, 4);

	mv = Malloc(sizeof(struct mapview));
	mapview_init(mv, ob->art->tile_map, MAPVIEW_TILEMAP|MAPVIEW_PROPS,
	    100, 100);
	mapview_set_selection(mv, 0, 0, 1, 1);

	object_load(ob->art->tile_map, NULL);

	/* Map operation buttons */
	reg = region_new(win, REGION_HALIGN|REGION_CLIPPING, 0, 0, 100, -1);
	region_set_spacing(reg, 1, 1);
	{
		bu = button_new(reg, NULL,		/* Load map */
		    SPRITE(&mapedit, MAPEDIT_TOOL_LOAD_MAP), 0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_revert_map, "%p", mv);

		bu = button_new(reg, NULL,		/* Save map */
		    SPRITE(&mapedit, MAPEDIT_TOOL_SAVE_MAP), 0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_save_map, "%p", mv);
		
		bu = button_new(reg, NULL,		/* Clear map */
		    SPRITE(&mapedit, MAPEDIT_TOOL_CLEAR_MAP), 0, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_clear_map, "%p", mv);
		
		bu = button_new(reg, NULL,		/* Toggle grid */
		    SPRITE(&mapedit, MAPEDIT_TOOL_GRID), BUTTON_STICKY, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    objq_tmap_option, "%p, %i", mv, MAPEDIT_TOOL_GRID);

		bu = button_new(reg, NULL,		/* Toggle props */
		    SPRITE(&mapedit, MAPEDIT_TOOL_PROPS), BUTTON_STICKY,
		    -1, -1);
		widget_set_bool(bu, "state", 1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    objq_tmap_option, "%p, %i", mv, MAPEDIT_TOOL_PROPS);
	
		bu = button_new(reg, NULL,		/* Toggle map edition */
		    SPRITE(&mapedit, MAPEDIT_TOOL_EDIT), BUTTON_STICKY, -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    objq_tmap_option, "%p, %i", mv, MAPEDIT_TOOL_EDIT);
		
		bu = button_new(reg, NULL,	       /* Toggle node edition */
		    SPRITE(&mapedit, MAPEDIT_TOOL_NODEEDIT), BUTTON_STICKY,
		    -1, -1);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    objq_tmap_option, "%p, %i", mv, MAPEDIT_TOOL_NODEEDIT);
		mv->nodeed.trigger = bu;

		event_new(win, "window-close", objq_close_tmap,
		    "%p, %p", mv, bu);
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	region_set_spacing(reg, 0, 0);
	{
		/* Map view */
		region_attach(reg, mv);
	}

	window_show(win);

	/* Create the source tile selection window. */
	win = window_generic_new(152, 287, "mapedit-tmap-%s-tiles", ob->name);
	if (win == NULL)
		return;					/* Exists */
	mv->constr.win = win;
	event_new(win, "window-close", window_generic_hide, "%p", win);
	window_set_caption(win, "%s", ob->name);
	{
		struct tlist *tl;
		struct region *reg_buttons1, *reg_buttons2;
		Uint32 i;
		
		reg_buttons1 = region_new(win, REGION_HALIGN, 0, 0, 100, -1);
		reg_buttons2 = region_new(win, REGION_HALIGN, 0, 0, 100, -1);

		reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
		{
			char *s;

			tl = tlist_new(reg, 100, 0, TLIST_MULTI);

			for (i = 0; i < ob->art->nsubmaps; i++) {
				Asprintf(&s, "m%d", i);
				tlist_insert_item(tl, NULL, s,
				    ob->art->submaps[i]);
				free(s);
			}

			for (i = 0; i < ob->art->nsprites; i++) {
				Asprintf(&s, "s%d", i);
				tlist_insert_item(tl, ob->art->sprites[i],
				    s, ob);
				free(s);
			}
			
			for (i = 0; i < ob->art->nanims; i++) {
				struct art_anim *an = ob->art->anims[i];
			
				Asprintf(&s, "a%d", i);
				tlist_insert_item(tl, (an->nframes > 0) ?
				    an->frames[0] : NULL, s, ob);
				free(s);
			}
		}
		
		{
			int i;
			int icons[] = {
				MAPEDIT_TOOL_LEFT,
				MAPEDIT_TOOL_RIGHT,
				MAPEDIT_TOOL_UP,
				MAPEDIT_TOOL_DOWN
			};
			struct button *bu;

			for (i = 0; i < 4; i++) {
				bu = button_new(reg_buttons1, NULL,
				    SPRITE(&mapedit, icons[i]), 0, 26, -1);
				event_new(bu, "button-pushed",
				    objq_insert_tiles, "%p, %p, %i", tl, mv, i);
			}
			bu = button_new(reg_buttons2, "Replace", NULL,
			    BUTTON_STICKY, 50, -1);
			event_new(bu, "button-pushed",
			    tl_objs_toggle_replace, "%p", mv);
			bu = button_new(reg_buttons2, "Big", NULL,
			    BUTTON_STICKY, 50, -1);
			event_new(bu, "button-pushed",
			    tl_objs_toggle_big, "%p", tl);
		}
	}
}

struct window *
objq_window(void)
{
	struct window *win;
	struct region *reg;

	win = window_generic_new(221, 112, "mapedit-objq");
	if (win == NULL)
		return (NULL);		/* Exists */
	event_new(win, "window-close", window_generic_hide, "%p", win);
	win->rd.x = view->w - 88;
	win->rd.y = 0;
	window_set_caption(win, "Objects");

	reg = region_new(win, 0, 0, 0, 100, 100);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		objq_update(tl);
		event_new(tl, "tlist-changed", tl_objs_selected, NULL);
	}
	return (win);
}

