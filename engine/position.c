/*	$Csoft$	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#include <engine/view.h>

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02
#define DEBUG_CONTROL	0x04
#define DEBUG_MOVE	0x08
#define DEBUG_BLOCKS	0x10
int	position_debug = DEBUG_STATE|DEBUG_POSITION|DEBUG_CONTROL|DEBUG_MOVE|
			 DEBUG_BLOCKS;
#define engine_debug position_debug
#endif

void
position_init(struct position *pos)
{
	pos->map = NULL;
	pos->map_name = NULL;
	pos->x = 0;
	pos->y = 0;
	pos->z = 0;
	pos->projmap = NULL;
	pos->projmap_name = NULL;
	pos->input = NULL;
	mapdir_init(&pos->dir, NULL, 0, 0);
}

void
position_destroy(struct position *pos)
{
	position_unproject(pos);
	Free(pos->map_name);
	Free(pos->projmap_name);
	mapdir_destroy(&pos->dir);
}

/* Assign a position to an object. */
int
position_set(void *p, struct map *dstmap, int x, int y, int z,
    struct map *projmap)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	dprintf("%s: position -> %s:[%d,%d,%d]\n", ob->name,
	    OBJECT(dstmap)->name, x, y, z);

	if (ob->pos != NULL) {
		debug(DEBUG_POSITION, "%s: updating position\n", ob->name);
		position_unproject(ob->pos);
	} else {
		debug(DEBUG_POSITION, "%s: creating position\n", ob->name);
		ob->pos = Malloc(sizeof(struct position));
		position_init(ob->pos);
		ob->pos->projmap = projmap;
		mapdir_init(&ob->pos->dir, ob, DIR_SOFT_MOTION, 1);
	}

	pthread_mutex_lock(&dstmap->lock);
	if (ob->pos->x >= 0 && ob->pos->x < dstmap->mapw &&
	    ob->pos->y >= 0 && ob->pos->y < dstmap->maph &&
	    z >= 0 && z < dstmap->nlayers) {
		ob->pos->map = dstmap;
		ob->pos->x = x;
		ob->pos->y = y;
		ob->pos->z = z;
		position_project(ob->pos);
	} else {
		error_set(_("Illegal coordinates: %s[%d,%d,%d]"),
		    OBJECT(dstmap)->name, x, y, z);
		goto fail;
	}
	pthread_mutex_unlock(&dstmap->lock);
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	position_unset(ob);
	pthread_mutex_unlock(&dstmap->lock);
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Unset an object's position, if there is one. */
void
position_unset(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	debug(DEBUG_POSITION, "%s: unset %p\n", ob->name, ob->pos);
	if (ob->pos != NULL) {
		position_destroy(ob->pos);
		free(ob->pos);
		ob->pos = NULL;
	}
	pthread_mutex_unlock(&ob->lock);
}

/* Control an object's position with an input device. */
int
position_set_input(void *p, const char *inname)
{
	struct object *ob = p;
	struct input *in;

	if ((in = input_find(inname)) == NULL) {
		error_set(_("There is no input device named `%s'."), inname);
		return (-1);
	}

	pthread_mutex_lock(&ob->lock);

	if (ob->pos == NULL) {
		error_set(_("Object `%s' has no position."), ob->name);
		goto fail;
	}
	debug(DEBUG_CONTROL, "%s: <%s>\n", ob->name, OBJECT(in)->name);
	ob->pos->input = in;

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Define the projection of an object. */
int
position_set_projmap(void *p, const char *name)
{
	struct object *ob = p;
	struct object *projmap;

	pthread_mutex_lock(&ob->lock);
	
	dprintf("%s: %s -> %s\n", ob->name,
	    ob->pos != NULL ? OBJECT(ob->pos->projmap)->name : "none", name);

	TAILQ_FOREACH(projmap, &ob->children, cobjs) {
		if (strcmp(projmap->name, name) == 0)
			break;
	}
	if (projmap == NULL) {
		error_set(_("There is no projmap named `%s'."), name);
		goto fail;
	}
	if (ob->pos != NULL) {
		position_unproject(ob->pos);
	}
	ob->pos->projmap = (struct map *)projmap;

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Project the object onto a level map. */
void
position_project(struct position *pos)
{
	struct map *sm = pos->projmap;
	struct map *dm = pos->map;
	int sx, sy, dx, dy;

	dprintf("[%d,%d,%d]+%dx%d\n", pos->x, pos->y, pos->z, sm->mapw,
	    sm->maph);
	
	object_add_dep(dm, sm);
	object_page_in(sm, OBJECT_DATA);
	object_page_in(dm, OBJECT_DATA);

	for (sy = 0, dy = pos->y;
	     sy < sm->maph && dy < pos->y+dm->maph;
	     sy++, dy++) {
		for (sx = 0, dx = pos->x;
		     sx < sm->mapw && dx < pos->x+dm->mapw;
		     sx++, dx++)
			node_copy(sm, &sm->map[sy][sx], -1, dm,
			    &dm->map[dy][dx], pos->z);
	}
	
	object_del_dep(dm, sm);
	object_page_out(sm, OBJECT_DATA);
	object_page_out(dm, OBJECT_DATA);
}

/* Remove a projection from the level map. */
void
position_unproject(struct position *pos)
{
	int x, y;
	
	dprintf("[%d,%d,%d]+%dx%d\n", pos->x, pos->y, pos->z,
	    pos->projmap->mapw, pos->projmap->maph);
	
	object_page_in(pos->map, OBJECT_DATA);
	for (y = pos->y; y < pos->y+pos->projmap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->projmap->mapw; x++)
			node_clear(pos->map, &pos->map->map[y][x], pos->z);
	}
	object_page_out(pos->map, OBJECT_DATA);
}

/*
 * Set the direction of an object inside a map.
 * A position must exist.
 */
int
position_set_direction(void *p, int dir, int dirflags, int speed)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);

	if (ob->pos == NULL) {
		error_set(_("Object `%s' has no position."), ob->name);
		goto fail;
	}
	debug(DEBUG_POSITION, "%s: direction -> %d/%d\n", ob->name, dir, speed);
	mapdir_init(&ob->pos->dir, ob, dirflags, speed);

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

int
position_save(struct position *pos, struct netbuf *buf)
{
	char map_id[OBJECT_NAME_MAX];

	object_copy_name(pos->map, map_id, sizeof(map_id));
	write_string(buf, map_id);
	write_string(buf, OBJECT(pos->projmap)->name);
	write_string(buf, (pos->input != NULL) ?
	    OBJECT(pos->input)->name : "");
	write_uint32(buf, (Uint32)pos->x);
	write_uint32(buf, (Uint32)pos->y);
	write_uint8(buf, (Uint8)pos->z);

	/* XXX save direction */
}

int
position_load(struct object *ob, struct position *pos, struct netbuf *buf)
{
	char map_name[OBJECT_PATH_MAX];
	char projmap_name[OBJECT_PATH_MAX];
	char input_name[OBJECT_PATH_MAX];
	struct map *map, *projmap;
	int x, y, z;

	if (ob->pos != NULL)
		position_unset(ob);

	copy_string(map_name, buf, sizeof(map_name));
	copy_string(projmap_name, buf, sizeof(projmap_name));
	copy_string(input_name, buf, sizeof(input_name));

	x = (int)read_uint32(buf);
	y = (int)read_uint32(buf);
	z = (int)read_uint8(buf);
		
	debug(DEBUG_STATE, "%s: at %s:[%d,%d,%d], as %s\n", ob->name,
	    map_name, x, y, z, projmap_name);

	if ((map = object_find(map_name)) == NULL) {
		error_set(_("No such level map: `%s'"), map_name);
		return (-1);
	}
	if ((projmap = object_find(projmap_name)) == NULL) {
		error_set(_("No such projection map: `%s'"), projmap_name);
		return (-1);
	}

	pthread_mutex_lock(&map->lock);
	if (position_set(ob, map, x, y, z, projmap) == NULL) {
		goto fail;
	}
	if (input_name != NULL &&
	    position_set_input(ob, input_name) == -1) {
		goto fail;
	}
	pthread_mutex_unlock(&map->lock);
	return (0);
fail:
	pthread_mutex_unlock(&map->lock);
	return (-1);
}

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
mapdir_destroy(struct mapdir *dir)
{
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
	struct position *pos = dir->ob->pos;
	int x, y;

	for (y = pos->y; y < pos->y+pos->projmap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->projmap->mapw; x++) {
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
	struct position *pos = dir->ob->pos;
	int x, y;

	for (y = pos->y; y < pos->y+pos->projmap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->projmap->mapw; x++) {
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
		position_set_projmap(dir->ob, "n-move");
		mapdir_set_motion(dir, 0, -1);
		dir->current |= DIR_N;
		dir->current &= ~(DIR_S|DIR_W|DIR_W);
	}
	if (dir->set & DIR_S) {
		position_set_projmap(dir->ob, "s-move");
		mapdir_set_motion(dir, 0, 1);
		dir->current |= DIR_S;
		dir->current &= ~(DIR_N|DIR_W|DIR_W);
	}
	if (dir->set & DIR_W) {
		position_set_projmap(dir->ob, "w-move");
		mapdir_set_motion(dir, -1, 0);
		dir->current |= DIR_W;
		dir->current &= ~(DIR_W|DIR_N|DIR_S);
	}
	if (dir->set & DIR_E) {
		position_set_projmap(dir->ob, "e-move");
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
	struct map *projmap = ob->pos->projmap;
	struct node *node = &dstmap->map[y][x];
	struct noderef *r;

	if (x < 0 || y < 0 ||
	    x+projmap->mapw >= dstmap->mapw ||
	    y+projmap->maph >= dstmap->maph) {
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
	struct position *pos = ob->pos;
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
	struct position *pos;
	int x, y;

	pthread_mutex_lock(&dir->ob->lock);
	pos = dir->ob->pos;

	pthread_mutex_lock(&pos->map->lock);
	pthread_mutex_lock(&pos->projmap->lock);

	dprintf("%s at %s:[%d,%d]+%dx%d\n", dir->ob->name,
	    OBJECT(pos->projmap)->name, pos->x, pos->y,
	    pos->projmap->mapw, pos->projmap->maph);

	if (pos->x < 0 || pos->y < 0 ||
	    pos->x+pos->projmap->mapw >= pos->map->mapw ||
	    pos->y+pos->projmap->maph >= pos->map->maph ||
	    pos->z >= pos->map->nlayers) {
		error_set("bad coords");
		goto fail;
	}

	for (y = pos->y; y < pos->y+pos->projmap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->projmap->mapw; x++) {
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
	pthread_mutex_unlock(&pos->projmap->lock);
	pthread_mutex_unlock(&pos->map->lock);
	pthread_mutex_unlock(&dir->ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&pos->projmap->lock);
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
		position_set_projmap(dir->ob, "n-idle");
	}
	if (dir->clear & DIR_S) {
		dir->current &= ~DIR_S;
		position_set_projmap(dir->ob, "s-idle");
	}
	if (dir->clear & DIR_W) {
		dir->current &= ~DIR_W;
		position_set_projmap(dir->ob, "w-idle");
	}
	if (dir->clear & DIR_W) {
		dir->current &= ~DIR_W;
		position_set_projmap(dir->ob, "e-idle");
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

