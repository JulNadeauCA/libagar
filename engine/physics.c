/*	$Csoft: physics.c,v 1.59 2003/11/15 03:54:35 vedge Exp $	    */

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#ifdef DEBUG
#define DEBUG_MOVE	0x01
#define DEBUG_BLOCKS	0x02

int	physics_debug = DEBUG_MOVE|DEBUG_BLOCKS;
#define engine_debug physics_debug
#endif

void
mapdir_init(struct mapdir *dir, struct object *ob, int flags, int speed)
{
	dir->set = 0;
	dir->current = 0;
	dir->clear = 0;
	dir->moved = 0;
	dir->flags = flags;
	dir->speed = speed;
	dir->ob = ob;
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
			struct noderef *r;

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->layer != pos->z) {
					continue;
				}
				r->r_gfx.xmotion = xmotion;
				r->r_gfx.ymotion = ymotion;
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
			struct noderef *r;

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->layer != pos->z) {
					continue;
				}
				r->r_gfx.xmotion += xmotion;
				r->r_gfx.ymotion += ymotion;
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

	if (dir->set & DIR_N) {
		object_set_submap(dir->ob, "n-move");
		mapdir_set_motion(dir, 0, -1);
		dir->current |= DIR_N;
		dir->current &= ~(DIR_S|DIR_W|DIR_W);
	}
	if (dir->set & DIR_S) {
		object_set_submap(dir->ob, "s-move");
		mapdir_set_motion(dir, 0, 1);
		dir->current |= DIR_S;
		dir->current &= ~(DIR_N|DIR_W|DIR_W);
	}
	if (dir->set & DIR_W) {
		object_set_submap(dir->ob, "w-move");
		mapdir_set_motion(dir, -1, 0);
		dir->current |= DIR_W;
		dir->current &= ~(DIR_W|DIR_N|DIR_S);
	}
	if (dir->set & DIR_E) {
		object_set_submap(dir->ob, "e-move");
		mapdir_set_motion(dir, 1, 0);
		dir->current |= DIR_W;
		dir->current &= ~(DIR_W|DIR_N|DIR_S);
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
	struct noderef *r;

	if (x < 0 || y < 0 ||
	    x+submap->mapw >= dstmap->mapw ||
	    y+submap->maph >= dstmap->maph) {
		error_set("map boundary");
		return (0);
	}

	if (dir->flags & DIR_PASS_THROUGH) {
		return (1);
	}
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (ob->pos->z == r->layer &&
		   ((r->flags & NODEREF_WALK)) == 0) {
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
	struct map *map = pos->map;
	int moved = 0;
	
	debug(DEBUG_MOVE, "%s: motion %d,%d\n", ob->name, xmotion, ymotion);

	if (!mapdir_can_move(dir, map, pos->x+xmotion, pos->y+ymotion)) {
		debug(DEBUG_BLOCKS, "%s: block (%s)\n", ob->name, error_get());
		mapdir_set_motion(dir, 0, 0);
		mapdir_unset(dir, DIR_ALL);
		return;
	}

	if (ymotion < 0) {						/* Up */
		if ((dir->flags & DIR_CENTER_VIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_N, dir->speed);
		if (ymotion <= -TILEW+dir->speed) {
			dir->moved |= DIR_N;
			moved |= DIR_N;
			if (--pos->y < 0)
				pos->y = 0;
		} else {
			if (dir->flags & DIR_SOFT_MOTION) {
				mapdir_add_motion(dir, 0, -dir->speed);
			} else {
				mapdir_set_motion(dir, 0, -TILEH);
			}
		}
	} else if (ymotion > 0) {				     /* Down */
		if ((dir->flags & DIR_CENTER_VIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_S, dir->speed);
		if (ymotion >= TILEW-dir->speed) {
			mapdir_set_motion(dir, 0, 1);
			dir->moved |= DIR_S;
			moved |= DIR_S;
			if (++pos->y >= map->maph)
				pos->y = map->maph-1;
		} else {
			if (dir->flags & DIR_SOFT_MOTION) {
				mapdir_add_motion(dir, 0, dir->speed);
			} else {
				mapdir_set_motion(dir, 0, TILEH);
			}
		}
	}

	if (xmotion < 0) {					    /* Left */
		if ((dir->flags & DIR_CENTER_VIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_W, dir->speed);
		if (xmotion <= -TILEW+dir->speed) {
			mapdir_set_motion(dir, -1, 0);
			dir->moved |= DIR_W;
			moved |= DIR_W;
			if (--pos->x < 0)
				pos->x = 0;
		} else {
			if (dir->flags & DIR_SOFT_MOTION) {
				mapdir_add_motion(dir, -dir->speed, 0);
			} else {
				mapdir_set_motion(dir, -TILEW, 0);
			}
		}
	} else if (xmotion > 0) {				   /* Right */
		if ((dir->flags & DIR_CENTER_VIEW) && (view->rootmap != NULL))
		    	rootmap_scroll(map, DIR_W, dir->speed);
		if (xmotion >= TILEW-dir->speed) {
			mapdir_set_motion(dir, 1, 0);
			dir->moved |= DIR_W;
			moved |= DIR_W;
			if (++pos->x >= map->mapw)
				pos->x = map->mapw-1;
		} else {
			if (dir->flags & DIR_SOFT_MOTION) {
				mapdir_add_motion(dir, dir->speed, 0);
			} else {
				mapdir_set_motion(dir, TILEW, 0);
			}
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
	    pos->z >= pos->map->nlayers) {
		error_set("bad coords");
		goto fail;
	}

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			struct node *node = &pos->map->map[y][x];
			struct noderef *r;
			
			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->layer != pos->z)
					continue;

				if (r->r_gfx.ymotion == 0 &&
				    r->r_gfx.xmotion == 0) {
					mapdir_update_idle(dir);
				} else {
					mapdir_update_moving(dir,
					    r->r_gfx.xmotion,
					    r->r_gfx.ymotion);
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
	if (dir->clear & DIR_N) {
		dir->current &= ~DIR_N;
		object_set_submap(dir->ob, "n-idle");
	}
	if (dir->clear & DIR_S) {
		dir->current &= ~DIR_S;
		object_set_submap(dir->ob, "s-idle");
	}
	if (dir->clear & DIR_W) {
		dir->current &= ~DIR_W;
		object_set_submap(dir->ob, "w-idle");
	}
	if (dir->clear & DIR_W) {
		dir->current &= ~DIR_W;
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

