/*	$Csoft: command.c,v 1.45 2002/09/06 01:26:41 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
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

#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/text.h>

#include "mapedit.h"
#include "command.h"

static void	 mapedit_setcursor(struct mapedit *, int);

/*
 * All the following operations must be performed
 * on a locked map editor and map.
 */

/* Toggle the map edition cursor on the map. */
static void
mapedit_setcursor(struct mapedit *med, int enable)
{
	static int oxoffs = 0, oyoffs = 0;
	struct noderef *nref;
	struct node *node = &med->map->map[med->y][med->x];

	if (enable) {
		nref = node_addref(node, med, MAPEDIT_SELECT,
		    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
		node->flags |= NODE_ANIM;

		/* Restore direction state. */
		nref->xoffs = oxoffs;
		nref->yoffs = oyoffs;
	} else {
		nref = node_findref(node, med, MAPEDIT_SELECT, MAPREF_ANIM);
		if (nref == NULL) {
			dprintf("nothing at %dx%d\n", med->x, med->y);
		}
		node->flags &= ~(NODE_ANIM);
	
		/* Save direction state. */
		oxoffs = nref->xoffs;
		oyoffs = nref->yoffs;

		node_delref(node, nref);
	}
}

/* Push a reference onto the node stack. */
void
mapedit_push(struct mapedit *med, struct node *node, int refn, int nflags)
{
	struct editref *eref;
	struct noderef *nref;
	
	mapedit_setcursor(med, 0);

	if ((med->flags & MAPEDIT_INSERT) == 0) {	/* Replace */
		TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
			node_delref(node, nref);
		}
	}

	/* XXX inefficient */
	pthread_mutex_lock(&med->curobj->lock);
	SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs, refn);
	pthread_mutex_unlock(&med->curobj->lock);

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
	mapedit_setcursor(med, 1);
	med->map->redraw++;
}

/* Remove the highest reference off the node stack. */
void
mapedit_pop(struct mapedit *med, struct node *node, int clear)
{
	mapedit_setcursor(med, 0);
	if (clear) {
		struct noderef *nref, *nextnref;

		for (nref = TAILQ_FIRST(&node->nrefsh);
		     nref != TAILQ_END(&node->nrefsh);
		     nref = nextnref) {
			nextnref = TAILQ_NEXT(nref, nrefs);
			free(nref);
		}
		TAILQ_INIT(&node->nrefsh);
	} else {
		if (!TAILQ_EMPTY(&node->nrefsh)) {
			node_delref(node, TAILQ_LAST(&node->nrefsh, noderefq));
		}
	}
	mapedit_setcursor(med, 1);
}

/* Reinitialize the map. */
void
mapedit_clearmap(struct mapedit *med)
{
	mapedit_setcursor(med, 0);
	map_clean(med->map, NULL, 0, 0, 0);
	mapedit_setcursor(med, 1);

	med->redraw++;

	text_msg(2, TEXT_SLEEP,
	    "New %dx%d map.\n", med->map->mapw, med->map->maph);
}

/* Fill the map with the current reference. */
void
mapedit_fillmap(struct mapedit *med)
{
	struct editref *eref;

	mapedit_setcursor(med, 0);

	pthread_mutex_lock(&med->curobj->lock);
	SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs, med->curoffs);
	pthread_mutex_unlock(&med->curobj->lock);

	switch (eref->type) {
	case EDITREF_SPRITE:
		map_clean(med->map, med->curobj->pobj, eref->spritei,
		    med->curflags & ~(NODE_ORIGIN|NODE_ANIM),
		    MAPREF_SAVE|MAPREF_SPRITE);
		break;
	case EDITREF_ANIM:
		map_clean(med->map, med->curobj->pobj, eref->animi,
		    med->curflags & ~(NODE_ORIGIN|NODE_ANIM),
		    MAPREF_SAVE|MAPREF_ANIM|MAPREF_ANIM_DELTA);
		break;
	}

	mapedit_setcursor(med, 1);
	med->redraw++;
	
	text_msg(2, TEXT_SLEEP,
	    "Initialized %dx%d map.\n", med->map->mapw, med->map->maph);
}

/* Move the origin of the map to x,y. */
void
mapedit_set_origin(struct mapedit *med, int *x, int *y)
{
	struct map *map = med->map;
	
	map->map[map->defy][map->defx].flags &= ~(NODE_ORIGIN);
	map->defx = *x;
	map->defy = *y;

	map->map[*y][*x].flags |= NODE_ORIGIN;

	*x = map->defx;
	*y = map->defy;
	map->redraw++;
	
	text_msg(2, TEXT_SLEEP, "Set origin at %dx%d.\n", *x, *y);
}

/* Toggle map editor flags. */
void
mapedit_editflags(struct mapedit *med, int mask)
{
	if (med->flags & mask) {
		med->flags &= ~mask;
	} else {
		med->flags |= mask;
	}
	med->redraw++;	/* Update the states display */
}

/* Toggle node flags. */
void
mapedit_nodeflags(struct mapedit *med, struct node *node, Uint32 flag)
{
	if (flag == 0) {
		node->flags &= NODE_DONTSAVE;
	} else if (flag == NODE_WALK) {
		node->flags &= ~(NODE_BLOCK);
		node->flags |= flag;
	} else if (flag == NODE_BLOCK) {
		node->flags &= ~(NODE_WALK);
		node->flags |= flag;
	} else {
		if (node->flags & flag) {
			node->flags &= ~(flag);
		} else {
			node->flags |= flag;
		}
	}
	med->curflags = node->flags;
	med->redraw++;	/* Update the states display */
}

