/*	$Csoft: physics.c,v 1.54 2003/03/25 13:48:00 vedge Exp $	    */

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/input.h>
#include <engine/physics.h>
#include <engine/view.h>

/*
 * These timings have a granularity proportional to the
 * interval of the timer calling gendir_move().
 */
enum {
	GENDIR_REPEAT_DELAY =	30,	/* Repeat delay */
	GENDIR_REPEAT_IVAL =	10	/* Repeat interval */
};

#ifdef DEBUG
#define DEBUG_MOVE	0x01
#define DEBUG_BLOCKS	0x02

int	physics_debug = DEBUG_MOVE|DEBUG_BLOCKS;
#define engine_debug physics_debug
#endif

void
gendir_init(struct gendir *dir)
{
	dir->set = 0;
	dir->current = 0;
	dir->clear = 0;
	dir->moved = 0;
	dir->offs = 0;
	dir->noffs = 0;
}

void
gendir_set(struct gendir *dir, int direction)
{
	dir->clear &= ~direction;
	dir->set   |=  direction;
}

void
gendir_unset(struct gendir *dir, int direction)
{
	dir->clear |=  direction;
	dir->set   &= ~direction;
}

int
gendir_move(struct gendir *dir)
{
	if (dir->clear != 0) {
		int r = dir->current;

		/* Effect a gendir_unset() operation. */
		dir->current &= ~(dir->clear);
		dir->clear = 0;
		dir->noffs = 0;
		dir->offs = 0;
		return (r);
	}

	if (dir->current != 0) {
		/* Keyboard repeat delay */
		if (dir->noffs > 0) {
			if (dir->noffs++ > GENDIR_REPEAT_DELAY) {
				/* chain */
				return (dir->current);
			}
		}

		/* Repeat interval */
		if (dir->offs++ > GENDIR_REPEAT_IVAL) {
			dir->offs = 0;
			return (dir->current);
		} else {
			return (0);
		}
	} else {
		if (dir->set != 0) {
			dir->current |= dir->set;
			dir->set = 0;
		}
	}
	return (0);
}

void
gendir_postmove(struct gendir *dir, int moved)
{
	if (dir->clear != 0) {
		/* Clear this direction (eg. key release). */
		dir->current &= ~(dir->clear);
		dir->clear = 0;
		dir->noffs = 0;
		dir->offs = 0;
	} else {
		/* Increment the key delay counter. */
		dir->noffs++;
	}
}

void
mapdir_init(struct mapdir *dir, struct object *ob, struct map *map,
    int flags, int speed)
{
	dir->set = 0;
	dir->current = 0;
	dir->clear = 0;
	dir->moved = 0;
	dir->flags = flags;
	dir->speed = speed;
	dir->ob = ob;
	dir->map = map;
}

void
mapdir_set(struct mapdir *dir, int direction)
{
	dir->clear &= ~direction;
	dir->set   |=  direction;
}

void
mapdir_unset(struct mapdir *dir, int direction)
{
	dir->clear |=  direction;
	dir->set   &= ~direction;
}

static void
mapdir_set_motion(struct mapdir *dir, int xmotion, int ymotion)
{
	struct object_position *pos = dir->ob->pos;
	int x, y;

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			struct node *node = &pos->map->map[y][x];
			struct noderef *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != pos->layer)
					continue;
				nref->xmotion = xmotion;
				nref->ymotion = ymotion;
			}
		}
	}
}

static void
mapdir_add_motion(struct mapdir *dir, int xmotion, int ymotion)
{
	struct object_position *pos = dir->ob->pos;
	int x, y;

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			struct node *node = &pos->map->map[y][x];
			struct noderef *nref;

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != pos->layer)
					continue;
				nref->xmotion += xmotion;
				nref->ymotion += ymotion;
			}
		}
	}
}

/* Update directions assuming the object is not already moving. */
static void
mapdir_update_idle(struct mapdir *dir)
{
	if (dir->set == 0)
		return;

	if (dir->set & DIR_UP) {
		object_set_submap(dir->ob, "n-move");
		mapdir_set_motion(dir, 0, -1);
		dir->current |= DIR_UP;
		dir->current &= ~(DIR_DOWN|DIR_LEFT|DIR_RIGHT);
	}
	if (dir->set & DIR_DOWN) {
		object_set_submap(dir->ob, "s-move");
		mapdir_set_motion(dir, 0, 1);
		dir->current |= DIR_DOWN;
		dir->current &= ~(DIR_UP|DIR_LEFT|DIR_RIGHT);
	}
	if (dir->set & DIR_LEFT) {
		object_set_submap(dir->ob, "w-move");
		mapdir_set_motion(dir, -1, 0);
		dir->current |= DIR_LEFT;
		dir->current &= ~(DIR_RIGHT|DIR_UP|DIR_DOWN);
	}
	if (dir->set & DIR_RIGHT) {
		object_set_submap(dir->ob, "e-move");
		mapdir_set_motion(dir, 1, 0);
		dir->current |= DIR_RIGHT;
		dir->current &= ~(DIR_LEFT|DIR_UP|DIR_DOWN);
	}
	dir->set = 0;
}

/* See if the object can move to m:x,y. */
static int
mapdir_can_move(struct mapdir *dir, struct map *dstmap, int x, int y)
{
	struct object *ob = dir->ob;
	struct map *submap = ob->pos->submap;
	struct node *node = &dstmap->map[y][x];
	struct noderef *nref;

	if (x < 0 || y < 0 ||
	    x+submap->mapw >= dstmap->mapw || y+submap->maph >= dstmap->maph) {
		error_set("map boundary");
		return (0);
	}

	if (dir->flags & DIR_PASSTHROUGH)
		return (1);
	if ((node->flags & NODE_WALK) == 0) {
		error_set("node block");
		return (0);
	}
	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		if (ob->pos->layer == nref->layer &&
		   (nref->flags & NODEREF_BLOCK)) {
			error_set("nref block");
			return (0);
		}
	}
	return (1);
}

static void
mapdir_update_moving(struct mapdir *dir, int xmotion, int ymotion)
{
	struct object *ob = dir->ob;
	struct object_position *pos = ob->pos;
	struct map *map = dir->map;
	int moved = 0;
	
	debug(DEBUG_MOVE, "%s: vel %d,%d\n", ob->name, xmotion, ymotion);

	if (!mapdir_can_move(dir, map, pos->x+xmotion, pos->y+ymotion)) {
		debug(DEBUG_BLOCKS, "%s: blocked (%s)\n", ob->name,
		    error_get());
		mapdir_set_motion(dir, 0, 0);
		mapdir_unset(dir, DIR_ALL);
		return;
	}

	if (ymotion < 0) {						/* Up */
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_UP, dir->speed);
		if (ymotion <= -TILEW+dir->speed) {
			dir->moved |= DIR_UP;
			moved |= DIR_UP;
			if (--pos->y < 0)
				pos->y = 0;
		} else {
			if (dir->flags & DIR_SOFTSCROLL)
				mapdir_add_motion(dir, 0, -dir->speed);
			else
				mapdir_set_motion(dir, 0, -TILEH);
		}
	} else if (ymotion > 0) {				     /* Down */
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_DOWN, dir->speed);
		if (ymotion >= TILEW-dir->speed) {
			mapdir_set_motion(dir, 0, 1);
			dir->moved |= DIR_DOWN;
			moved |= DIR_DOWN;
			if (++pos->y >= map->maph)
				pos->y = map->maph-1;
		} else {
			if (dir->flags & DIR_SOFTSCROLL)
				mapdir_add_motion(dir, 0, dir->speed);
			else
				mapdir_set_motion(dir, 0, TILEH);
		}
	}

	if (xmotion < 0) {					    /* Left */
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_LEFT, dir->speed);
		if (xmotion <= -TILEW+dir->speed) {
			mapdir_set_motion(dir, -1, 0);
			dir->moved |= DIR_LEFT;
			moved |= DIR_LEFT;
			if (--pos->x < 0)
				pos->x = 0;
		} else {
			if (dir->flags & DIR_SOFTSCROLL)
				mapdir_add_motion(dir, -dir->speed, 0);
			else
				mapdir_set_motion(dir, -TILEW, 0);
		}
	} else if (xmotion > 0) {				   /* Right */
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_RIGHT, dir->speed);
		if (xmotion >= TILEW-dir->speed) {
			mapdir_set_motion(dir, 1, 0);
			dir->moved |= DIR_RIGHT;
			moved |= DIR_RIGHT;
			if (++pos->x >= map->mapw)
				pos->x = map->mapw-1;
		} else {
			if (dir->flags & DIR_SOFTSCROLL)
				mapdir_add_motion(dir, dir->speed, 0);
			else
				mapdir_set_motion(dir, TILEW, 0);
		}
	}
}

/* Update a map direction and move its associated object if necessary. */
int
mapdir_move(struct mapdir *dir)
{
	struct object_position *pos;
	int x, y;

	pthread_mutex_lock(&dir->ob->lock);
	pos = dir->ob->pos;

	pthread_mutex_lock(&pos->map->lock);
	pthread_mutex_lock(&pos->submap->lock);

	dprintf("%s at %s:[%d,%d]+%dx%d\n", dir->ob->name,
	    OBJECT(pos->submap)->name, pos->x, pos->y,
	    pos->submap->mapw, pos->submap->maph);

	if (pos->x < 0 || pos->y < 0 ||
	    pos->x+pos->submap->mapw >= pos->map->mapw ||
	    pos->y+pos->submap->maph >= pos->map->maph ||
	    pos->layer >= pos->map->nlayers) {
		error_set("bad coords");
		goto fail;
	}

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			struct node *node = &pos->map->map[y][x];
			struct noderef *nref;
			
			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != pos->layer)
					continue;

				if (nref->ymotion == 0 && nref->xmotion == 0) {
					/* Effect changes in direction. */
					mapdir_update_idle(dir);
				} else {
					/* Update the direction. */
					mapdir_update_moving(dir, nref->xmotion,
					    nref->ymotion);
				}
				/*
				 * Other nrefs in this rectangle/layer are
				 * assumed to have the same [xy]motion.
				 */
				goto out;
			}
		}
	}
out:
	pthread_mutex_unlock(&pos->submap->lock);
	pthread_mutex_unlock(&pos->map->lock);
	pthread_mutex_unlock(&dir->ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&pos->submap->lock);
	pthread_mutex_unlock(&pos->map->lock);
	pthread_mutex_unlock(&dir->ob->lock);
	return (-1);
}

/*
 * Called after a movement, to ensure continuation if necessary, or
 * stop moving.
 */
void
mapdir_postmove(struct mapdir *dir, int *mapx, int *mapy, int moved)
{
	if (dir->clear & DIR_UP) {
		dir->current &= ~DIR_UP;
		object_set_submap(dir->ob, "n-idle");
	}
	if (dir->clear & DIR_DOWN) {
		dir->current &= ~DIR_DOWN;
		object_set_submap(dir->ob, "s-idle");
	}
	if (dir->clear & DIR_LEFT) {
		dir->current &= ~DIR_LEFT;
		object_set_submap(dir->ob, "w-idle");
	}
	if (dir->clear & DIR_RIGHT) {
		dir->current &= ~DIR_RIGHT;
		object_set_submap(dir->ob, "e-idle");
	}
	dir->clear = 0;
	mapdir_set_motion(dir, 0, 0);

	if (dir->current != 0) {
		if (dir->set == 0) {
			/* Continue moving in the same direction. */
			dir->set = moved;
		} else {
			/* Change directions. */
		}

		/* Apply direction changes. */
		mapdir_update_idle(dir);

		dir->current |= dir->set;
		dir->set = 0;
	}
}

