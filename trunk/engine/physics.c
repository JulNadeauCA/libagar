/*	$Csoft: physics.c,v 1.47 2002/12/01 14:41:02 vedge Exp $	    */

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

#include "engine.h"

#include "map.h"
#include "rootmap.h"
#include "input.h"
#include "physics.h"
#include "view.h"

/*
 * These timings have a granularity proportional to the
 * interval of the timer calling gendir_move().
 */
enum {
	GENDIR_REPEAT_DELAY =	30,	/* Repeat delay */
	GENDIR_REPEAT_IVAL =	10	/* Repeat interval */
};

static void	mapdir_change(struct mapdir *, struct noderef *);
static int	mapdir_canmove(struct mapdir *, struct map *, Uint32, Uint32);

#ifdef DEBUG
#define DEBUG_MOVE	0x01
#define DEBUG_BLOCKED	0x02

int	physics_debug = DEBUG_BLOCKED;
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
gendir_set(struct gendir *dir, Uint32 direction)
{
	dir->clear &= ~direction;
	dir->set   |=  direction;
}

void
gendir_unset(struct gendir *dir, Uint32 direction)
{
	dir->clear |=  direction;
	dir->set   &= ~direction;
}

Uint32
gendir_move(struct gendir *dir)
{
	if (dir->clear != 0) {
		Uint32 r = dir->current;

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
gendir_postmove(struct gendir *dir, Uint32 moved)
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
    Uint32 flags, Uint32 speed)
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
mapdir_set(struct mapdir *dir, Uint32 direction)
{
	dir->clear &= ~direction;
	dir->set   |=  direction;
}

void
mapdir_unset(struct mapdir *dir, Uint32 direction)
{
	dir->clear |=  direction;
	dir->set   &= ~direction;
}

/*
 * Change map direction if necessary. X/Y velocity values are
 * mutually exclusive, and so are direction flags.
 *
 * Map must be locked.
 */
static void
mapdir_change(struct mapdir *dir, struct noderef *nref)
{
	Uint32 offs;

	if (dir->set & DIR_UP) {
		if (prop_get(dir->ob,
		    "physics-up-motion-anim", PROP_UINT32, &offs) == 0) {
			nref->offs = offs;
		}
		/* XXX sprite ... */
		dir->set &= ~(DIR_UP);
		nref->ymotion = -1;
		dir->current |= DIR_UP;
		dir->current &= ~(DIR_DOWN);
		nref->xmotion = 0;
		dir->current &= ~(DIR_LEFT);
		dir->current &= ~(DIR_RIGHT);
	}
	if (dir->set & DIR_DOWN) {
		if (prop_get(dir->ob, "physics-down-motion-anim", PROP_UINT32,
		    &offs) == 0) {
			nref->offs = offs;
		}
		dir->set &= ~(DIR_DOWN);
		nref->ymotion = 1;
		dir->current |= DIR_DOWN;
		dir->current &= ~(DIR_UP);
		nref->xmotion = 0;
		dir->current &= ~(DIR_LEFT);
		dir->current &= ~(DIR_RIGHT);
	}
	if (dir->set & DIR_LEFT) {
		if (prop_get(dir->ob, "physics-left-motion-anim", PROP_UINT32,
		    &offs) == 0) {
			nref->offs = offs;
		}

		dir->set &= ~(DIR_LEFT);
		nref->xmotion = -1;
		dir->current |= DIR_LEFT;
		dir->current &= ~(DIR_RIGHT);
		nref->ymotion = 0;
		dir->current &= ~(DIR_UP);
		dir->current &= ~(DIR_DOWN);
	}
	if (dir->set & DIR_RIGHT) {
		if (prop_get(dir->ob, "physics-right-motion-anim", PROP_UINT32,
		    &offs) == 0) {
			nref->offs = offs;
		}

		dir->set &= ~(DIR_RIGHT);
		nref->xmotion = 1;
		dir->current |= DIR_RIGHT;
		dir->current &= ~(DIR_LEFT);
		nref->ymotion = 0;
		dir->current &= ~(DIR_UP);
		dir->current &= ~(DIR_DOWN);
	}
}

/*
 * See if dir can move to m:x,y.
 * Map must be locked.
 */
static int
mapdir_canmove(struct mapdir *dir, struct map *m, Uint32 x, Uint32 y)
{
	struct node *node = &m->map[y][x];
	struct noderef *nref;

	if (x < 2 || y << 2 ||
	    x > m->mapw - 2 || y > m->maph - 2) {	/* Map boundaries */
		return (0);
	}

	if (dir->flags & DIR_PASSTHROUGH) {		/* Pass through */
		return (1);
	}
	if (node->flags & NODE_BLOCK) {			/* Node block */
		return (0);
	}
	/* XXX could be more sophisticated */
	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {	/* Reference block */
		if (nref->flags & NODEREF_BLOCK) {
			return (0);
		}
	}
	return (1);
}

/*
 * Update a map direction, and return a non-zero value if the map
 * coordinates have changed (so that the caller can move the reference).
 *
 * Map must be locked.
 */
int
mapdir_move(struct mapdir *dir, Uint32 *mapx, Uint32 *mapy)
{
	struct map *m = dir->map;
	struct noderef *onref, *nref;
	struct node *node;
	Uint32 moved = 0;

	/* XXX pick the first reference to this object */
#ifdef DEBUG
	if (*mapy > m->maph || *mapx > m->mapw) {
		fatal("bad coordinates\n");
	}
#endif
	node = &m->map[*mapy][*mapx];
	nref = NULL;
	TAILQ_FOREACH(onref, &node->nrefs, nrefs) {	
		if (onref->pobj == dir->ob) {
			nref = onref;
			break;
		}
	}
	if (nref == NULL) {
		fatal("no reference to %s at %s:%d,%d\n", dir->ob->name,
		    OBJECT(m)->name, *mapx, *mapy);
	}

	/* See if the reference is moving. */
	if (nref->ymotion == 0 && nref->xmotion == 0) {
		mapdir_change(dir, nref);
		return (0);
	}

	/* Upward motion */
	if (nref->ymotion < 0) {
		debug(DEBUG_MOVE, "%s moving up (ymotion=%d)\n", dir->ob->name,
		    nref->ymotion);
		if (nref->ymotion == -1 &&
		    !mapdir_canmove(dir, m, *mapx, (*mapy)-1)) {
			debug(DEBUG_BLOCKED, "%s cannot move upwards\n",
			    dir->ob->name);
			nref->ymotion = 0;
			mapdir_unset(dir, DIR_UP);
			return (0);
		}
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL)) {
		    	rootmap_scroll(m, DIR_UP, dir->speed);
		}
		if (nref->ymotion <= (-TILEW + dir->speed)) {
			dir->moved |= DIR_UP;
			moved |= DIR_UP;
			if ((*mapy)-- < 1) {
				*mapy = 1;
			}
		} else {
			if (dir->flags & DIR_SOFTSCROLL) {
				nref->ymotion -= dir->speed;
			} else {
				nref->ymotion = -TILEH;
			}
		}
	}

	/* Downward motion */
	if (nref->ymotion > 0) {
		debug(DEBUG_MOVE, "%s moving down (ymotion=%d)\n",
		    dir->ob->name, nref->ymotion);
		if (nref->ymotion == 1 &&
		    !mapdir_canmove(dir, m, *mapx, (*mapy)+1)) {
			debug(DEBUG_BLOCKED, "%s cannot move down\n",
			    dir->ob->name);
			nref->ymotion = 0;
			mapdir_unset(dir, DIR_DOWN);
			return (0);
		}
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL)) {
		    	rootmap_scroll(m, DIR_DOWN, dir->speed);
		}
		if (nref->ymotion >= TILEW - dir->speed) {
			nref->ymotion = 1;
			dir->moved |= DIR_DOWN;
			moved |= DIR_DOWN;
			if (++(*mapy) > m->maph - 1) {
				*mapy = m->maph - 1;
			}
		} else {
			if (dir->flags & DIR_SOFTSCROLL) {
				nref->ymotion += dir->speed;
			} else {
				nref->ymotion = TILEH;
			}
		}
	}

	/* Left motion */
	if (nref->xmotion < 0) {
		debug(DEBUG_MOVE, "%s moving left (xmotion=%d)\n",
		    dir->ob->name, nref->xmotion);
		if (nref->xmotion == -1 &&
		    !mapdir_canmove(dir, m, (*mapx)-1, *mapy)) {
			debug(DEBUG_BLOCKED, "%s cannot move left)\n",
			    dir->ob->name);
			nref->xmotion = 0;
			mapdir_unset(dir, DIR_LEFT);
			return (0);
		}
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL)) {
		    	rootmap_scroll(m, DIR_LEFT, dir->speed);
		}
		if (nref->xmotion <= (-TILEW + dir->speed)) {
			nref->xmotion = -1;
			dir->moved |= DIR_LEFT;
			moved |= DIR_LEFT;
			if ((*mapx)-- < 1) {
				*mapx = 1;
			}
		} else {
			if (dir->flags & DIR_SOFTSCROLL) {
				nref->xmotion -= dir->speed;
			} else {
				nref->xmotion = -TILEW;
			}
		}
	}

	/* Right motion */
	if (nref->xmotion > 0) {
		debug(DEBUG_MOVE, "%s moving right (xmotion=%d)\n",
		    dir->ob->name, nref->xmotion);
		if (nref->xmotion == 1 &&
		    !mapdir_canmove(dir, m, (*mapx)+1, *mapy)) {
			debug(DEBUG_BLOCKED, "%s cannot move right\n",
			    dir->ob->name);
			nref->xmotion = 0;
			mapdir_unset(dir, DIR_RIGHT);
			return (0);
		}
		if ((dir->flags & DIR_SCROLLVIEW) && (view->rootmap != NULL)) {
		    	rootmap_scroll(m, DIR_RIGHT, dir->speed);
		}
		if (nref->xmotion >= (TILEW - dir->speed)) {
			nref->xmotion = 1;
			dir->moved |= DIR_RIGHT;
			moved |= DIR_RIGHT;
			if (++(*mapx) > m->mapw-1) {
				*mapx = m->mapw-1;
			}
		} else {
			if (dir->flags & DIR_SOFTSCROLL) {
				nref->xmotion += dir->speed;
			} else {
				nref->xmotion = TILEW;
			}
		}
	}
	return (moved);
}

/*
 * Called after a movement, to ensure continuation if necessary, or
 * stop moving.
 *
 * Map must be locked.
 */
void
mapdir_postmove(struct mapdir *dir, Uint32 *mapx, Uint32 *mapy, Uint32 moved)
{
	struct node *node;
	struct noderef *nref = NULL, *onref;
	struct map *m = dir->map;
	Uint32 offs;

	/* XXX pick the first reference to this object */
#ifdef DEBUG
	if (*mapy > m->maph || *mapx > m->mapw) {
		fatal("bad coordinates\n");
	}
#endif
	node = &m->map[*mapy][*mapx];
	nref = NULL;
	TAILQ_FOREACH(onref, &node->nrefs, nrefs) {	
		if (onref->pobj == dir->ob) {
			nref = onref;
			break;
		}
	}
	if (nref == NULL) {
		fatal("no reference to %s at %s:%d,%d\n", dir->ob->name,
		    OBJECT(m)->name, *mapx, *mapy);
	}
#if 0
	view->rootmap->sx = 0;
	view->rootmap->sy = 0;
#endif

	/* Clear any direction first (ie. key release). */
	if (dir->clear != 0) {
		if (dir->clear & DIR_UP) {
			dir->current &= ~(DIR_UP);
			dir->clear &= ~(DIR_UP);
			nref->ymotion = 0;

			if (prop_get(dir->ob, "physics-up-idle-anim",
			    PROP_UINT32, &offs) == 0) {
				nref->offs = offs;
			}
			/* XXX sprite ... */
		}
		if (dir->clear & DIR_DOWN) {
			dir->current &= ~(DIR_DOWN);
			dir->clear &= ~(DIR_DOWN);
			nref->ymotion = 0;
			
			if (prop_get(dir->ob, "physics-down-idle-anim",
			    PROP_UINT32, &offs) == 0) {
				nref->offs = offs;
			}
		}
		if (dir->clear & DIR_LEFT) {
			dir->current &= ~(DIR_LEFT);
			dir->clear &= ~(DIR_LEFT);
			nref->xmotion = 0;
			
			if (prop_get(dir->ob, "physics-left-idle-anim",
			    PROP_UINT32, &offs) == 0) {
				nref->offs = offs;
			}
		}
		if (dir->clear & DIR_RIGHT) {
			dir->current &= ~(DIR_RIGHT);
			dir->clear &= ~(DIR_RIGHT);
			nref->xmotion= 0;
			
			if (prop_get(dir->ob, "physics-right-idle-anim",
			    PROP_UINT32, &offs) == 0) {
				nref->offs = offs;
			}
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

