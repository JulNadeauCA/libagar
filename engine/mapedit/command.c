/*	$Csoft	    */

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
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

#include <engine/engine.h>

#include "mapedit.h"
#include "command.h"

static void	mapedit_setpointer(struct mapedit *, int);

/*
 * All the following operations must be performed
 * on a locked map.
 */

static void
mapedit_setpointer(struct mapedit *med, int enable)
{
	static int oxoffs = 0, oyoffs = 0;
	struct noderef *nref;
	struct node *node = &med->map->map[med->x][med->y];

	if (enable) {
		nref = node_addref(node, med, MAPEDIT_SELECT, MAPREF_ANIM);
		node->flags |= NODE_ANIM;

		/* Restore direction state. */
		nref->xoffs = oxoffs;
		nref->yoffs = oyoffs;
	} else {
		nref = node_findref(node, med, MAPEDIT_SELECT);
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

	mapedit_setpointer(med, 0);

	if ((med->flags & MAPEDIT_INSERT) == 0) {	/* Replace */
		TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
			node_delref(node, nref);
		}
	}

	/* XXX inefficent */
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
		    MAPREF_ANIM|MAPREF_SAVE);
		break;
	}
	node->flags = nflags &= ~(NODE_ORIGIN|NODE_ANIM);
	mapedit_setpointer(med, 1);
	med->map->redraw++;
}

/* Remove the highest reference off the node stack. */
void
mapedit_pop(struct mapedit *med, struct node *node)
{
	struct noderef *nref;
	int i = 0;

	mapedit_setpointer(med, 0);
	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		if (i++ == (node->nnrefs - 1)) {
			node_delref(node, nref);
			med->map->redraw++;
			goto done;
		}
	}
done:
	mapedit_setpointer(med, 1);
}

/* Reinitialize the map. */
void
mapedit_clearmap(struct mapedit *med)
{
	mapedit_setpointer(med, 0);
	map_clean(med->map, NULL, 0, 0, 0);
	mapedit_setpointer(med, 1);
	med->map->redraw++;
	mapedit_objlist(med);
	mapedit_tilelist(med);
}

/* Fill the map with the current reference. */
void
mapedit_fillmap(struct mapedit *med)
{
	struct editref *eref;

	mapedit_setpointer(med, 0);

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
		    MAPREF_SAVE|MAPREF_ANIM);
		break;
	}

	mapedit_setpointer(med, 1);
	med->map->redraw++;
	mapedit_objlist(med);
	mapedit_tilelist(med);
}

void
mapedit_setorigin(struct mapedit *med, int *x, int *y)
{
	struct map *map = med->map;

	map->map[map->defx][map->defy].flags &= ~(NODE_ORIGIN);
	map->defx = *x;
	map->defy = *y;

	map->map[*x][*y].flags |= NODE_ORIGIN;

	*x = map->defx;
	*y = map->defy;
	map->redraw++;
}

void
mapedit_loadmap(struct mapedit *med)
{
	struct map *map = med->map;
	char path[FILENAME_MAX];
	int x, y;

	mapedit_setpointer(med, 0);

	map_freenodes(map);
	
	/* Users must copy maps to udatadir in order to edit them. */
	/* XXX redundant */
	sprintf(path, "%s/%s.m", world->udatadir, med->margs.name);

	if (object_loadfrom(med->map, path) == 0) {
		x = map->defx;
		y = map->defy;
		map->map[x][y].flags |= NODE_ORIGIN;
		med->x = x;
		med->y = y;
		view_center(map->view, x, y);
		mapedit_setcaption(med, path);
	} else {
		/* Reallocate nodes we just freed. */
		map_allocnodes(map, map->mapw, map->maph,
		    map->tilew, map->tileh);
	}
	mapedit_setpointer(med, 1);
	map->redraw++;

	mapedit_objlist(med);
	mapedit_tilelist(med);
}

void
mapedit_savemap(struct mapedit *med)
{
	object_save(med->map);
}

void
mapedit_examine(struct map *em, int x, int y)
{
	int i;
	struct noderef *nref;
	struct node *node;

	node = &em->map[x][y];

	printf("%dx%d < ", x, y);

	/* Keep in sync with map.h */
	if (node->flags == NODE_BLOCK) {
		printf("block ");
	} else {
		if (node->flags & NODE_ORIGIN)
			printf("origin ");
		if (node->flags & NODE_WALK)
			printf("walk ");
		if (node->flags & NODE_CLIMB)
			printf("climb ");
		if (node->flags & NODE_SLIP)
			printf("slippery ");
		if (node->flags & NODE_BIO)
			printf("bio ");
		if (node->flags & NODE_REGEN)
			printf("regen ");
		if (node->flags & NODE_SLOW)
			printf("slow ");
		if (node->flags & NODE_HASTE)
			printf("haste ");
		if (node->v1 > 0)
			printf("(v1=%d) ", node->v1);
	}
	printf(">\n");

	i = 0;
	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		printf("\t[%2d] ", i++);
		printf("%s:%d ", nref->pobj->name, nref->offs);
		if (nref->flags & MAPREF_SAVE)
			printf("saveable ");
		if (nref->flags & MAPREF_SPRITE)
			printf("sprite ");
		if (nref->flags & MAPREF_ANIM)
			printf("anim ");
		if (nref->flags & MAPREF_WARP)
			printf("warp ");
		printf("\n");
	}
}

void
mapedit_editflags(struct mapedit *med, int mask)
{
	if (med->flags & mask) {
		med->flags &= ~mask;
	} else {
		med->flags |= mask;
	}

	med->map->redraw++;
	
}

void
mapedit_nodeflags(struct mapedit *med, struct node *node, int flag)
{
	if (flag == 0) {
		node->flags = 0;
	} else if (flag == NODE_WALK) {
		node->flags |= flag;
	} else {
		if (node->flags & flag) {
			node->flags &= ~(flag);
		} else {
			node->flags |= flag;
		}
	}
	/* XXX pref */
	med->curflags = node->flags;
}

