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

#include <unistd.h>

#include <engine/engine.h>

#include "mapedit.h"
#include "command.h"

static void	mapedit_setpointer(struct mapedit *, int);

static void
mapedit_setpointer(struct mapedit *med, int enable)
{
	static int oxoffs = 0, oyoffs = 0;
	struct map_aref *aref;
	struct node *node = &med->map->map[med->x][med->y];

	if (enable) {
		aref = node_addref(node,
		    (struct object *)med, MAPEDIT_SELECT, MAPREF_ANIM);
		node->flags |= NODE_ANIM;

		/* Restore direction state. */
		aref->xoffs = oxoffs;
		aref->yoffs = oyoffs;
	} else {
		aref = node_arefobj(node,
		    (struct object *)med, MAPEDIT_SELECT);
		node->flags &= ~(NODE_ANIM);
	
		/* Save direction state. */
		oxoffs = aref->xoffs;
		oyoffs = aref->yoffs;

		node_delref(node, aref);
	}
}

/* Push a reference onto the node stack. */
void
mapedit_push(struct mapedit *med, struct node *node)
{
	struct editref *eref;

	mapedit_setpointer(med, 0);

	/* XXX inefficent */
	pthread_mutex_lock(&med->curobj->lock);
	SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs, med->curoffs);
	pthread_mutex_unlock(&med->curobj->lock);

#ifdef DEBUG
	/* XXX should not happen */
	if (eref == NULL) {
		fatal("no editor ref at %d\n", med->curoffs);
	}
#endif

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
	node->flags = med->curflags;
	mapedit_setpointer(med, 1);
	med->map->redraw++;
}

/* Remove the highest reference off the node stack. */
void
mapedit_pop(struct mapedit *med, struct node *node)
{
	struct map_aref *aref;

	if (node->narefs < 2) {
		dprintf("empty node\n");
		return;
	}

	aref = node_arefindex(node, node->narefs - 2);
	node_delref(node, aref);
	med->map->redraw++;
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
		    med->curflags & ~NODE_ORIGIN,
		    MAPREF_SAVE|MAPREF_SPRITE);
		break;
	case EDITREF_ANIM:
		map_clean(med->map, med->curobj->pobj, eref->animi,
		    med->curflags & ~NODE_ORIGIN,
		    MAPREF_SAVE|MAPREF_ANIM);
		break;
	}

	mapedit_setpointer(med, 1);
	med->map->redraw++;
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
	view_center(map->view, *x, *y);
	map->redraw++;
}

void
mapedit_load(struct mapedit *med)
{
	char path[FILENAME_MAX];
	struct map *map = med->map;
	int x, y;

	mapedit_setpointer(med, 0);
	map_clean(map, NULL, 0, 0, 0);
	sprintf(path, "%s.map", map->obj.name);
	if (map->obj.load(med->map, path) == 0) {
		x = map->defx;
		y = map->defy;
		map->map[x][y].flags |= NODE_ORIGIN;
		med->x = x;
		med->y = y;
		view_center(map->view, x, y);
	}
	mapedit_setpointer(med, 1);
	map->redraw++;
}

void
mapedit_save(struct mapedit *med)
{
	char path[FILENAME_MAX];

	sprintf(path, "%s.map", med->map->obj.name);
	dprintf("saving %s...\n", path);
	med->map->obj.save((struct object *)med->map, path);
	dprintf("done\n");
}

void
mapedit_examine(struct map *em, int x, int y)
{
	int i;
	struct map_aref *aref;
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
	TAILQ_FOREACH(aref, &node->arefsh, marefs) {
		printf("\t[%2d] ", i++);
		printf("%s:%d ", aref->pobj->name, aref->offs);
		if (aref->flags & MAPREF_SAVE)
			printf("saveable ");
		if (aref->flags & MAPREF_SPRITE)
			printf("sprite ");
		if (aref->flags & MAPREF_ANIM)
			printf("anim ");
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

	if (mask & MAPEDIT_TILELIST) {
		view_setmode(med->map->view);	/* XXX hack */
	}

	med->map->redraw++;
	
}

void
mapedit_nodeflags(struct mapedit *med, struct node *me, int flag)
{
	if (me->narefs < 2) {
		dprintf("empty tile\n");
		return;
	}

	if (flag == 0) {
		me->flags = 0;
	} else {
		if (me->flags & flag) {
			me->flags &= ~(flag);
		} else {
			me->flags |= flag;
		}
	}
	med->curflags = me->flags;
}

