/*	$Csoft$	*/

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

#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "mapedit.h"
#include "mapview.h"
#include "edcursor.h"

static const struct object_ops edcursor_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

static void	edcursor_key(int, union evarg *);
static void	edcursor_push(struct edcursor *, struct node *, int, int);
static void	edcursor_attached(int, union evarg *);
static Uint32	edcursor_update(Uint32, void *);

struct edcursor *
edcursor_new(int flags, struct mapview *mv, struct map *m)
{
	struct edcursor *ed;

	ed = emalloc(sizeof(struct edcursor));
	edcursor_init(ed, flags, mv, m);

	pthread_mutex_lock(&world->lock);
	world_attach(world, ed);
	pthread_mutex_unlock(&world->lock);

	return (ed);
}

void
edcursor_init(struct edcursor *ed, int flags, struct mapview *mv,
    struct map *m)
{
	object_init(&ed->obj, "map-editor-cursor", "edcursor", "edcursor",
	    OBJ_ART, &edcursor_ops);
	ed->flags = flags;
	ed->med = mv->med;
	ed->map = m;
	ed->x = m->defx;
	ed->y = m->defy;
	ed->mv = mv;
	mapdir_init(&ed->dir, OBJECT(ed), m,
	    DIR_PASSTHROUGH|DIR_STATIC, 1);

	event_new(ed, "attached", 0,
	    edcursor_attached, NULL);
	event_new(ed, "window-keyup", 0,
	    edcursor_key, "%i", WINDOW_KEYUP);
	event_new(ed, "window-keydown", 0,
	    edcursor_key, "%i", WINDOW_KEYDOWN);
}

static void
edcursor_attached(int argc, union evarg *argv)
{
	struct edcursor *ed = argv[0].p;

	/* Make the cursor visible. */
	edcursor_set(ed, 1);
	mapview_center(ed->mv, ed->x, ed->y);
	WIDGET(ed)->win->redraw++;

#if 0
	SDL_AddTimer(70, edcursor_update, ed);
#endif
}

static Uint32
edcursor_update(Uint32 ival, void *p)
{
	struct edcursor *ed = p;
	Uint32 x, y;
	int moved;

	x = ed->x;
	y = ed->y;

	moved = mapdir_move(&ed->dir, &x, &y);
	if (moved != 0) {
		edcursor_move(ed, x, y);
		mapdir_postmove(&ed->dir, &x, &y, moved);
//		edcursor_sticky(ed);
		WIDGET(ed)->win->redraw++;
	}

	return (ival);
}

/*
 * Move the map edition cursor.
 * Map editor and map must be locked.
 */
void
edcursor_move(struct edcursor *ed, Uint32 x, Uint32 y)
{
	struct node *node;

	node = &ed->map->map[ed->y][ed->x];
	node_delref(node, node_findref(node, ed, EDCURSOR_SELECT, MAPREF_ANIM));
	node->flags &= ~(NODE_ANIM);
	
	node = &ed->map->map[y][x];
	node_addref(node, ed, EDCURSOR_SELECT,
	    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
	node->flags |= NODE_ANIM;

	ed->x = x;
	ed->y = y;
}

static void
edcursor_key(int argc, union evarg *argv)
{
	struct edcursor *ed = argv[0].p;
	int ev = argv[1].i;
	SDLKey keysym = (SDLKey)argv[2].i;
	int keymod = argv[3].i;
	const int set = (ev == WINDOW_KEYDOWN) ? 1 : 0;

	switch (keysym) {
	case SDLK_LEFT:
		if (ed->x > 0) {
			mapdir_set(&ed->dir, DIR_LEFT, set);
		}
		break;
	case SDLK_UP:
		if (ed->y > 0) {
			mapdir_set(&ed->dir, DIR_UP, set);
		}
		break;
	case SDLK_RIGHT:
		if (ed->x < ed->map->mapw) {
			mapdir_set(&ed->dir, DIR_RIGHT, set);
		}
		break;
	case SDLK_DOWN:
		if (ed->y < ed->map->maph) {
			mapdir_set(&ed->dir, DIR_DOWN, set);
		}
		break;
	default:
	}

	if (ev == WINDOW_KEYDOWN) {
		switch (keysym) {
		case SDLK_a:
			edcursor_push(ed, &ed->map->map[ed->y][ed->x],
			    ed->med->curoffs, ed->med->curflags);
			break;
		default:
		}
	}
}

/* Add or remove the reference to the cursor on the map. */
void
edcursor_set(struct edcursor *ed, int enable)
{
	static int oxoffs = 0, oyoffs = 0;
	struct node *node = &ed->map->map[ed->y][ed->x];
	struct noderef *nref;

#if 1
	return;
#endif

	if (enable) {
		nref = node_addref(node, ed, EDCURSOR_SELECT,
		    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
		node->flags |= NODE_ANIM;

		/* Restore direction state. */
		nref->xoffs = oxoffs;
		nref->yoffs = oyoffs;
	} else {
		nref = node_findref(node, ed, EDCURSOR_SELECT, MAPREF_ANIM);
		if (nref == NULL) {
			dprintf("nothing at %dx%d\n", ed->x, ed->y);
			return;
		}
		node->flags &= ~(NODE_ANIM);
	
		/* Save direction state. */
		oxoffs = nref->xoffs;
		oyoffs = nref->yoffs;

		node_delref(node, nref);
	}
}

/* Push a reference onto the node stack. */
static void
edcursor_push(struct edcursor *ed, struct node *node, int refn, int nflags)
{
	struct mapedit *med = ed->med;
	struct editref *eref;
	struct noderef *nref;
	
	edcursor_set(ed, 0);

#if 0
	if ((ed->flags & EDCURSOR_INSERT) == 0) {	/* Replace */
		TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
			node_delref(node, nref);
		}
	}
#endif
	SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs, refn);

	switch (eref->type) {
	case EDITREF_SPRITE:
		node_addref(node, med->curobj->pobj, eref->spritei,
		    MAPREF_SPRITE|MAPREF_SAVE);
		break;
	case EDITREF_ANIM:
		node_addref(node, med->curobj->pobj, eref->animi,
		    MAPREF_ANIM|MAPREF_SAVE|MAPREF_ANIM_DELTA);
		break;
	}

	node->flags = nflags &= ~(NODE_ORIGIN|NODE_ANIM);
	edcursor_set(ed, 1);
	ed->map->redraw++;
}

