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

static void	mapdir_change(struct mapdir *, struct noderef *);

int
gendir_init(struct gendir *dir)
{
	dir->set = 0;
	dir->clear = 0;
	dir->moved = 0;

	return (0);
}

int
gendir_set(struct gendir *dir, int direction, int set)
{
	if (set) {
		dir->clear &= ~direction;
		dir->set   |=  direction;
	} else {
		dir->clear |=  direction;
		dir->set   &= ~direction;
	}
	return (0);
}

int
gendir_move(struct gendir *dir)
{
	if (dir->current == 0 && dir->set != 0) {
		dir->current |= dir->set;
		dir->set = 0;
	}
	return (dir->current);
}

void
gendir_postmove(struct gendir *dir, int moved)
{
	/* Clear this direction (eg. key release). */
	if (dir->clear != 0) {
		dir->current &= ~(dir->clear);
		dir->clear = 0;
	}

	/* Set this direction (eg. key press). */
	if (dir->set != 0) {
		dir->current |= dir->set;
		dir->set = 0;
	}
}

int
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

	return (0);
}

/*
 * Set the given map direction if set is non-zero, otherwise
 * clear it (asynchronously).
 */
void
mapdir_set(struct mapdir *dir, int direction, int set)
{
	if (set) {
		dir->clear &= ~direction;
		dir->set   |=  direction;
	} else {
		dir->clear |=  direction;
		dir->set   &= ~direction;
	}
}

/*
 * Change map direction if necessary. X/Y velocity values are
 * mutually exclusive, and so are direction flags.
 */
static void
mapdir_change(struct mapdir *dir, struct noderef *nref)
{
	if (dir->set & DIR_UP) {
		dir->set &= ~(DIR_UP);
		nref->yoffs = -1;
		dir->current |= DIR_UP;
		dir->current &= ~(DIR_DOWN);
		nref->xoffs = 0;
		dir->current &= ~(DIR_LEFT);
		dir->current &= ~(DIR_RIGHT);
	}
	if (dir->set & DIR_DOWN) {
		dir->set &= ~(DIR_DOWN);
		nref->yoffs = 1;
		dir->current |= DIR_DOWN;
		dir->current &= ~(DIR_UP);
		nref->xoffs = 0;
		dir->current &= ~(DIR_LEFT);
		dir->current &= ~(DIR_RIGHT);
	}
	if (dir->set & DIR_LEFT) {
		dir->set &= ~(DIR_LEFT);
		nref->xoffs = -1;
		dir->current |= DIR_LEFT;
		dir->current &= ~(DIR_RIGHT);
		nref->yoffs = 0;
		dir->current &= ~(DIR_UP);
		dir->current &= ~(DIR_DOWN);
	}
	if (dir->set & DIR_RIGHT) {
		dir->set &= ~(DIR_RIGHT);
		nref->xoffs = 1;
		dir->current |= DIR_RIGHT;
		dir->current &= ~(DIR_LEFT);
		nref->yoffs = 0;
		dir->current &= ~(DIR_UP);
		dir->current &= ~(DIR_DOWN);
	}
}

/*
 * Update a map direction, and return a non-zero value if the map
 * coordinates have changed (so that the caller can move the reference).
 */
int
mapdir_move(struct mapdir *dir, int *mapx, int *mapy)
{
	struct map *map;
	struct noderef *nref;
	struct node *node;
	int moved = 0;

	map = dir->map;
	node = &map->map[*mapx][*mapy];
	nref = node_findref(node, dir->ob, -1);
	if (nref == NULL) {
		dprintf("%s not at %s:%dx%d\n", dir->ob->name,
		    dir->map->obj.name, *mapx, *mapy);
		return (0);
	}

	if (nref->yoffs == 0 && nref->xoffs == 0) {
		mapdir_change(dir, nref);
	} else {
		/* Up */
		if (nref->yoffs < 0) {
			if (nref->yoffs == -1) {	/* Once */
				map->map[*mapx][*mapy - 1].flags |= NODE_ANIM;
			}
			if (nref->yoffs <= (-map->tilew + dir->speed)) {
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_UP;
				moved |= DIR_UP;
				decrease(mapy, 1, 1);
				if ((dir->flags & DIR_SCROLLVIEW) &&
				    (map->view->mapy - *mapy) >= 0) {
				    	scroll(map, DIR_UP);
				}
			} else {
				if (dir->flags & DIR_SOFTSCROLL) {
					nref->yoffs -= dir->speed;
				} else {
					nref->yoffs = -map->tileh;
				}
			}
		}
		/* Down */
		if (nref->yoffs > 0) {
			if (nref->yoffs == 1) {		/* Once */
				map->map[*mapx][*mapy + 1].flags |= NODE_ANIM;
			}
			if (nref->yoffs >= map->tilew - dir->speed) {
				nref->yoffs = 1;
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_DOWN;
				moved |= DIR_DOWN;
				increase(mapy, 1, map->maph - 1);
				if ((dir->flags & DIR_SCROLLVIEW) &&
				    (map->view->mapy - *mapy) <=
				     -map->view->maph) {
					scroll(map, DIR_DOWN);
				}
			} else {
				if (dir->flags & DIR_SOFTSCROLL) {
					nref->yoffs += dir->speed;
				} else {
					nref->yoffs = map->tileh;
				}
			}
		}
		
		/* Left */
		if (nref->xoffs < 0) {
			if (nref->xoffs == -1) {	/* Once */
				map->map[*mapx - 1][*mapy].flags |= NODE_ANIM;
			}
			if (nref->xoffs <= (-map->tilew + dir->speed)) {
				nref->xoffs = -1;
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_LEFT;
				moved |= DIR_LEFT;
				decrease(mapx, 1, 1);
				if ((dir->flags & DIR_SCROLLVIEW) &&
				    (map->view->mapx - *mapx) >= 0) {
					scroll(map, DIR_LEFT);
				}
			} else {
				if (dir->flags & DIR_SOFTSCROLL) {
					nref->xoffs -= dir->speed;
				} else {
					nref->xoffs = -map->tilew;
				}
			}
		}
		/* Right */
		if (nref->xoffs > 0) {
			if (nref->xoffs == 1) {		/* Once */
				map->map[*mapx + 1][*mapy].flags |= NODE_ANIM;
			}
			if (nref->xoffs >= (map->tilew - dir->speed)) {
				nref->xoffs = 1;
				node->flags &= ~(NODE_ANIM);
				dir->moved |= DIR_RIGHT;
				moved |= DIR_RIGHT;
				increase(mapx, 1, map->mapw - 1);
				if ((dir->flags & DIR_SCROLLVIEW) &&
				    (map->view->mapx - *mapx) <=
				     -map->view->mapw) {
					scroll(map, DIR_RIGHT);
				}
			} else {
				if (dir->flags & DIR_SOFTSCROLL) {
					nref->xoffs += dir->speed;
				} else {
					nref->xoffs = map->tilew;
				}
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
mapdir_postmove(struct mapdir *dir, int *mapx, int *mapy, int moved)
{
	struct node *node;
	struct noderef *nref;

	node = &dir->map->map[*mapx][*mapy];
	nref = node_findref(node, dir->ob, -1);

	/* Clear any direction first (ie. key release). */
	if (dir->clear != 0) {
		if (dir->clear & DIR_UP) {
			dir->current &= ~(DIR_UP);
			dir->clear &= ~(DIR_UP);
			nref->yoffs = 0;
		}
		if (dir->clear & DIR_DOWN) {
			dir->current &= ~(DIR_DOWN);
			dir->clear &= ~(DIR_DOWN);
			nref->yoffs = 0;
		}
		if (dir->clear & DIR_LEFT) {
			dir->current &= ~(DIR_LEFT);
			dir->clear &= ~(DIR_LEFT);
			nref->xoffs = 0;
		}
		if (dir->clear & DIR_RIGHT) {
			dir->current &= ~(DIR_RIGHT);
			dir->clear &= ~(DIR_RIGHT);
			nref->xoffs = 0;
		}
	}

	if (dir->current != 0) {
		if (dir->set == 0) {
			/* Continue moving in the same direction. */
			dir->set = moved;
		} else {
			/* Change directions. */
		}
		mapdir_change(dir, nref);
		dir->current |= dir->set;
		dir->set = 0;
	}
}

