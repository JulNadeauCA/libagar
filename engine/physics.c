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

static void	direction_change(struct direction *, struct map_aref *);

int
direction_init(struct direction *dir, void *ob, struct map *map,
    int flags, int hiwat, int speed)
{
	dir->set = 0;
	dir->current = 0;
	dir->clear = 0;
	dir->moved = 0;
	dir->flags = flags;

	dir->tick = 0;
	dir->hiwat = hiwat;
	dir->speed = speed;
	dir->ob = ob;
	dir->map = map;

	return (0);
}

/*
 * Set the given direction if set is non-zero, otherwise
 * clear it (asynchronously).
 */
void
direction_set(struct direction *dir, int direction, int set)
{
	if (set) {
		dir->set |= direction;
	} else {
		dir->clear |= direction;
	}
}

/* Change direction if necessary. */
static void
direction_change(struct direction *dir, struct map_aref *aref)
{
	if (dir->set & DIR_UP) {
		dir->set &= ~(DIR_UP);
		aref->yoffs = -1;
		dir->current |= DIR_UP;
		dir->current &= ~(DIR_DOWN);
		aref->xoffs = 0;
		dir->current &= ~(DIR_LEFT);
		dir->current &= ~(DIR_RIGHT);
	}
	if (dir->set & DIR_DOWN) {
		dir->set &= ~(DIR_DOWN);
		aref->yoffs = 1;
		dir->current |= DIR_DOWN;
		dir->current &= ~(DIR_UP);
		aref->xoffs = 0;
		dir->current &= ~(DIR_LEFT);
		dir->current &= ~(DIR_RIGHT);
	}
	if (dir->set & DIR_LEFT) {
		dir->set &= ~(DIR_LEFT);
		aref->xoffs = -1;
		dir->current |= DIR_LEFT;
		dir->current &= ~(DIR_RIGHT);
		aref->yoffs = 0;
		dir->current &= ~(DIR_UP);
		dir->current &= ~(DIR_DOWN);
	}
	if (dir->set & DIR_RIGHT) {
		dir->set &= ~(DIR_RIGHT);
		aref->xoffs = 1;
		dir->current |= DIR_RIGHT;
		dir->current &= ~(DIR_LEFT);
		aref->yoffs = 0;
		dir->current &= ~(DIR_UP);
		dir->current &= ~(DIR_DOWN);
	}
}

/*
 * Update a direction, and return a non-zero value if the map
 * coordinates have changed (so that the caller can move the
 * reference on the map).
 */
int
direction_update(struct direction *dir, int *mapx, int *mapy)
{
	struct map *map;
	struct map_aref *aref;
	struct node *node;
	int moved = 0;

	map = dir->map;
	node = &map->map[*mapx][*mapy];
	aref = node_arefobj(node, (struct object *)dir->ob, -1);

	if (aref->yoffs == 0 && aref->xoffs == 0) {
		/* See if movement is requested. */
		direction_change(dir, aref);
	} else if (dir->moved == 0 || 1) {	/* ... wait */
		dir->tick = 0;

		/* Up */
		if (aref->yoffs < 0) {
			if (aref->yoffs == -1) {	/* Once */
				map->map[*mapx][*mapy - 1].flags |= NODE_ANIM;
			}
			if (aref->yoffs <= (-map->view->tilew + dir->speed)) {
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_UP;
				moved |= DIR_UP;
				decrease(mapy, 1, 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapy - *mapy) >= 0) {
				    	scroll(map, DIR_UP);
				}
			} else {
				aref->yoffs -= dir->speed;
				/* XXX soft scroll */
			}
		}
		/* Down */
		if (aref->yoffs > 0) {
			if (aref->yoffs == 1) {		/* Once */
				map->map[*mapx][*mapy + 1].flags |= NODE_ANIM;
			}
			if (aref->yoffs >= map->view->tilew - dir->speed) {
				aref->yoffs = 1;
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_DOWN;
				moved |= DIR_DOWN;
				increase(mapy, 1, map->maph - 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapy - *mapy) <=
				     -map->view->maph) {
					scroll(map, DIR_DOWN);
				}
			} else {
				aref->yoffs += dir->speed;
				/* XXX soft scroll */
			}
		}
		
		/* Left */
		if (aref->xoffs < 0) {
			if (aref->xoffs == -1) {	/* Once */
				map->map[*mapx - 1][*mapy].flags |= NODE_ANIM;
			}
			if (aref->xoffs <= (-map->view->tilew + dir->speed)) {
				aref->xoffs = -1;
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_LEFT;
				moved |= DIR_LEFT;
				decrease(mapx, 1, 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapx - *mapx) >= 0) {
					scroll(map, DIR_LEFT);
				}
			} else {
				aref->xoffs -= dir->speed;
				/* XXX soft scroll */
			}
		}
		/* Right */
		if (aref->xoffs > 0) {
			if (aref->xoffs == 1) {		/* Once */
				map->map[*mapx + 1][*mapy].flags |= NODE_ANIM;
			}
			if (aref->xoffs >= (map->view->tilew - dir->speed)) {
				aref->xoffs = 1;
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_RIGHT;
				moved |= DIR_RIGHT;
				increase(mapx, 1, map->mapw - 1);
				if ((dir->flags & DIR_SCROLL) &&
				    (map->view->mapx - *mapx) <=
				     -map->view->mapw) {
					scroll(map, DIR_RIGHT);
				}
			} else {
				aref->xoffs += dir->speed;
				/* XXX soft scroll */
			}
		}
	}
	return (moved);
}

/*
 * Called after a movement, to ensure continuation if necessary, or
 * stop moving.
 */
void
direction_moved(struct direction *dir, int *mapx, int *mapy, int moved)
{
	struct node *node;
	struct map_aref *aref;

	node = &dir->map->map[*mapx][*mapy];
	aref = node_arefobj(node, (struct object *)dir->ob, -1);

	/* Clear any direction first (ie. key release). */
	if (dir->clear != 0) {
		if (dir->clear & DIR_UP) {
			dir->current &= ~(DIR_UP);
			dir->clear &= ~(DIR_UP);
			aref->yoffs = 0;
		}
		if (dir->clear & DIR_DOWN) {
			dir->current &= ~(DIR_DOWN);
			dir->clear &= ~(DIR_DOWN);
			aref->yoffs = 0;
		}
		if (dir->clear & DIR_LEFT) {
			dir->current &= ~(DIR_LEFT);
			dir->clear &= ~(DIR_LEFT);
			aref->xoffs = 0;
		}
		if (dir->clear & DIR_RIGHT) {
			dir->current &= ~(DIR_RIGHT);
			dir->clear &= ~(DIR_RIGHT);
			aref->xoffs = 0;
		}
	}

	if (dir->current != 0) {
		if (dir->set == 0) {
			/* Continue moving in the same direction. */
			dir->set = moved;
		} else {
			/* Change directions. */
		}
		direction_change(dir, aref);
		dir->current |= dir->set;
		dir->set = 0;
	}
}

