/*	$Csoft: object.c,v 1.106 2003/02/12 01:18:05 vedge Exp $	*/

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

#include "compat/asprintf.h"
#include "engine.h"

#include "version.h"

#include <sys/stat.h>

#include <fcntl.h>

#include <libfobj/fobj.h>

#include "config.h"
#include "map.h"
#include "physics.h"
#include "input.h"
#include "view.h"
#include "rootmap.h"
#include "world.h"

static const struct object_ops null_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02
#define DEBUG_DEPS	0x04

int	object_debug = DEBUG_STATE|DEBUG_POSITION;
#define engine_debug object_debug
#endif

char *
object_name(const char *base, int num)
{
	char *name;

	name = emalloc(strlen(base)+16);
	sprintf(name, "%s%d", base, num);
	return (name);
}

struct object *
object_new(char *type, char *name, char *media, int flags, const void *opsp)
{
	struct object *ob;

	ob = emalloc(sizeof(struct object));
	object_init(ob, type, name, media, flags, opsp);

	pthread_mutex_lock(&world->lock);
	world_attach(ob);
	pthread_mutex_unlock(&world->lock);

	return (ob);
}

void
object_init(struct object *ob, char *type, char *name, char *media, int flags,
    const void *opsp)
{
	ob->type = Strdup(type);
	ob->name = Strdup(name);
	ob->ops = (opsp != NULL) ? opsp : &null_ops;
	ob->flags = flags;
	ob->pos = NULL;
	ob->state = OBJECT_EMBRYONIC;
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	pthread_mutex_init(&ob->pos_lock, NULL);
	
	pthread_mutexattr_init(&ob->props_lockattr);
	pthread_mutexattr_settype(&ob->props_lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&ob->props_lock, &ob->props_lockattr);

	pthread_mutexattr_init(&ob->events_lockattr);
	pthread_mutexattr_settype(&ob->events_lockattr,PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&ob->events_lock, &ob->events_lockattr);

	ob->art = (ob->flags & OBJECT_ART) ? art_fetch(media, ob) : NULL;
#if 0
	ob->audio = (ob->flags & OBJECT_AUDIO) ? audio_fetch(media, ob) : NULL;
#endif
}

void
object_destroy(void *p)
{
	struct object *ob = p;
	struct event *eev, *nexteev;
	struct prop *prop, *nextprop;
	
	if (OBJECT_OPS(ob)->destroy != NULL) {
		/* Destroy object specific data. */
		OBJECT_OPS(ob)->destroy(ob);
	}

	if (((ob->flags & OBJECT_ART_CACHE) == 0) &&
	    (ob->art != NULL)) {
		art_unused(ob->art);
	}
#if 0
	if (((ob->flags & OBJECT_AUDIO_CACHE) == 0) &&
	    (ob->audio != NULL)) {
		audio_unused(ob->audio);
	}
#endif

	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = nexteev) {
		nexteev = TAILQ_NEXT(eev, events);
		free(eev);
	}

	for (prop = TAILQ_FIRST(&ob->props);
	     prop != TAILQ_END(&ob->props);
	     prop = nextprop) {
		nextprop = TAILQ_NEXT(prop, props);
		prop_destroy(prop);
	}
	
	pthread_mutex_destroy(&ob->pos_lock);
	pthread_mutex_destroy(&ob->events_lock);
	pthread_mutex_destroy(&ob->props_lock);
	pthread_mutexattr_destroy(&ob->events_lockattr);

	Free(ob->name);
	Free(ob->type);
	free(ob);
}

int
object_load_from(void *p, char *path)
{
	struct object *ob = p;
	int fd;

	debug(DEBUG_STATE, "loading %s from %s\n", ob->name, path);

	fd = open(path, O_RDONLY, 00600);
	if (fd == -1) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	
	/* Load generic properties. */
	if (prop_load(ob, fd) != 0) {
		close(fd);
		return (-1);
	}

	/* Load object specific data. */
	if (OBJECT_OPS(ob)->load != NULL) {
		if (OBJECT_OPS(ob)->load(ob, fd) != 0) {
			close(fd);
			return (-1);
		}
	}

	close(fd);
	return (0);
}

int
object_load(void *p)
{
	struct object *ob = p;
	char *path;
	int fd, rv = 0;
	
	debug(DEBUG_STATE, "loading %s\n", ob->name);

	path = object_path(ob->name, ob->type);
	if (path == NULL) {
		return (-1);
	}

	fd = open(path, O_RDONLY, 00600);
	if (fd == -1) {
		error_set("%s: %s", path, strerror(errno));
		free(path);
		return (-1);
	}

	/* Load generic properties. */
	if (prop_load(ob, fd) != 0) {
		free(path);
		close(fd);
		return (-1);
	}

	/* Load object specific data. */
	if (OBJECT_OPS(ob)->load != NULL) {
		if (OBJECT_OPS(ob)->load(ob, fd) != 0) {
			free(path);
			close(fd);
			return (-1);
		}
	}

	free(path);
	close(fd);
	return (rv);
}

int
object_save(void *p)
{
	struct object *ob = p;
	char *path, *datadir, *typedir;
	struct stat sta;
	int fd;

	datadir = prop_get_string(config, "path.user_data_dir");
	if (stat(datadir, &sta) != 0 && mkdir(datadir, 0700) != 0) {
		fatal("creating %s: %s\n", datadir, strerror(errno));
	}
	Asprintf(&typedir, "%s/%s", datadir, ob->type);
	free(datadir);

	if (stat(typedir, &sta) != 0 && mkdir(typedir, 0700) != 0) {
		fatal("creating %s: %s\n", typedir, strerror(errno));
	}
	Asprintf(&path, "%s/%s.%s", typedir, ob->name, ob->type);
	free(typedir);
	
	debug(DEBUG_STATE, "saving %s to %s\n", ob->name, path);

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if (fd == -1) {
		fatal("%s: %s\n", path, strerror(errno));
	}

	/* Save generic properties. */
	if (prop_save(ob, fd) != 0) {
		close(fd);
		fatal("%s\n", error_get());
	}

	/* Save object specific data. */
	if (OBJECT_OPS(ob)->save != NULL && OBJECT_OPS(ob)->save(ob, fd) != 0) {
		close(fd);
		fatal("%s\n", error_get());
	}

	free(path);
	close(fd);
	return (0);
}

/* Search the data file directories for the given file. */
char *
object_path(char *obname, const char *suffix)
{
	struct stat sta;
	char *p, *last;
	char *datapath, *datapathp, *path;

	path = emalloc((size_t)FILENAME_MAX);
	datapathp = datapath = prop_get_string(config, "path.data_path");

	for (p = strtok_r(datapath, ":", &last);
	     p != NULL;
	     p = strtok_r(NULL, ":", &last)) {
		sprintf(path, "%s/%s/%s.%s", p, suffix, obname, suffix);
		if (stat(path, &sta) == 0) {
			free(datapathp);
			return (path);
		}
	}
	free(datapathp);
	free(path);

	error_set("Cannot find %s.%s", obname, suffix);
	return (NULL);
}

/* Control an object's position with an input device. */
void
object_control(void *p, struct input *in, int center)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->pos_lock);

	if (ob->pos == NULL) {
		pthread_mutex_unlock(&ob->pos_lock);
		fatal("%s is nowhere\n", ob->name);
	}

	/* Set the input device. */
	ob->pos->input = in;

	if (center && view->gfx_engine == GFX_ENGINE_TILEBASED) {
		/* Center on the object and enable soft-scrolling. */
		ob->pos->dir.flags |= DIR_SCROLLVIEW;
		rootmap_center(ob->pos->map, ob->pos->x, ob->pos->y);
	}
	pthread_mutex_unlock(&ob->pos_lock);
}

int
object_vanish(void *p)
{
	struct object *ob = p;
	struct map *m;
	struct node *node;

	pthread_mutex_lock(&ob->pos_lock);
	if (ob->pos == NULL) {
		pthread_mutex_unlock(&ob->pos_lock);
		error_set("%s is nowhere\n", ob);
		return (-1);
	}

	m = ob->pos->map;
	node = &m->map[ob->pos->y][ob->pos->x];

	debug(DEBUG_POSITION, "%s vanishing from %s:%d,%d\n", ob->name,
	    OBJECT(m)->name, ob->pos->x, ob->pos->y);

	node_remove_ref(&m->map[ob->pos->y][ob->pos->x], ob->pos->nref);

	free(ob->pos);
	ob->pos = NULL;

	pthread_mutex_unlock(&ob->pos_lock);
	return (0);
}

/*
 * Update an object's map position.
 *
 * This is usually called after a new noderef is created to update the
 * object's back reference to it.
 *
 * If there is a current noderef, it is removed. No noderef is added,
 * the code should use the node_add_* functions to create new noderefs
 * since there are various types of noderefs with different arguments.
 */
void
object_set_position(void *p, struct noderef *nref, struct map *m,
    int x, int y)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->pos_lock);

	if (ob->pos == NULL) {				/* Allocate position */
		debug(DEBUG_POSITION,
		    "%s: new position on %s:%d,%d\n", ob->name,
		    OBJECT(m)->name, x, y);
		ob->pos = emalloc(sizeof(struct mappos));
		ob->pos->input = NULL;
		mapdir_init(&ob->pos->dir, ob, m, DIR_SOFTSCROLL, 1);
	} else {
		debug(DEBUG_POSITION,
		    "%s: position change from %s:%d,%d to %s:%d,%d\n", ob->name,
		    OBJECT(ob->pos->map)->name, ob->pos->x, ob->pos->y,
		    OBJECT(m)->name, x, y);

		/* Remove the old noderef. */
		debug(DEBUG_POSITION, "%s: removing old noderef\n", ob->name);
		node_remove_ref(&ob->pos->map->map[ob->pos->y][ob->pos->x],
		    nref);
	}
	ob->pos->map = m;				/* Update position */
	ob->pos->x = x;
	ob->pos->y = y;
	ob->pos->nref = nref;

	pthread_mutex_unlock(&ob->pos_lock);
}

/*
 * Move the object from its current position to dst_map:dst_x,dst_y.
 * The current noderef is moved from the current node to the new position.
 */
void
object_move(void *p, struct map *dst_map, int dst_x, int dst_y)
{
	struct object *ob = p;
	struct map *src_map;
	struct node *src_node, *dst_node;
	int src_x, src_y;
	
	pthread_mutex_lock(&ob->pos_lock);
	if (ob->pos == NULL) {
		fatal("%s is nowhere\n", ob->name);
	}
	src_map = ob->pos->map;
	src_x = ob->pos->x;
	src_y = ob->pos->y;

	pthread_mutex_lock(&src_map->lock);
	src_node = &src_map->map[src_y][src_x];

	pthread_mutex_lock(&dst_map->lock);
	dst_node = &dst_map->map[dst_y][dst_x];

#ifdef DEBUG
	if (dst_x > dst_map->mapw || dst_y > dst_map->maph) {
		fatal("%d,%d exceeds map boundaries\n", dst_x, dst_y);
	}
#endif
	if (src_map == dst_map && src_x == dst_x && src_y == dst_y) {
		pthread_mutex_unlock(&dst_map->lock);
		pthread_mutex_unlock(&src_map->lock);
		pthread_mutex_unlock(&ob->pos_lock);
		debug(DEBUG_POSITION, "%s is already at %s:%d,%d\n", ob->name,
		    OBJECT(dst_map)->name, dst_x, dst_y);
		return;					/* Nothing to do */
	}
	
	debug(DEBUG_POSITION, "moving %s from %s:%d,%d to %s:%d,%d\n", ob->name,
	     OBJECT(src_map)->name, src_x, src_y,
	     OBJECT(dst_map)->name, dst_x, dst_y);

	/* Move the noderef to the new node. */
	node_move_ref(ob->pos->nref, src_node, dst_node);
	
	/* Update the object's back reference. */
	ob->pos->map = dst_map;
	ob->pos->x = dst_x;
	ob->pos->y = dst_y;

	pthread_mutex_unlock(&dst_map->lock);
	pthread_mutex_unlock(&src_map->lock);
	pthread_mutex_unlock(&ob->pos_lock);
}

struct object_table *
object_table_new(void)
{
	struct object_table *obt;

	obt = emalloc(sizeof(struct object_table));
	obt->objs = NULL;
	obt->used = NULL;
	obt->nobjs = 0;

	return (obt);
}

void
object_table_destroy(struct object_table *obt)
{
	Free(obt->objs);
	Free(obt->used);
	free(obt);
}

void
object_table_insert(struct object_table *obt, struct object *obj)
{
	if (obt->objs != NULL) {
		obt->objs = erealloc(obt->objs, (obt->nobjs + 1) *
		    sizeof(struct object *));
		obt->used = erealloc(obt->used, (obt->nobjs + 1) *
		    sizeof(int));
	} else {
		obt->objs = emalloc(sizeof(struct object *));
		obt->used = emalloc(sizeof(int));
	}
	obt->objs[obt->nobjs] = obj;
	obt->used[obt->nobjs] = 0;
	obt->nobjs++;
}

void
object_table_save(struct fobj_buf *buf, struct object_table *obt)
{
	size_t solen = 0;
	off_t nobjs_offs;
	struct object *pob;
	Uint32 i;

	nobjs_offs = buf->offs;
	buf_write_uint32(buf, 0);				/* Skip */
	for (i = 0; i < obt->nobjs; i++) {
		pob = obt->objs[i];
		buf_write_string(buf, pob->name);
		buf_write_string(buf, pob->type);
	}
	buf_pwrite_uint32(buf, obt->nobjs, nobjs_offs);
}

struct object_table *
object_table_load(int fd, char *objname)
{
	struct object_table *obt;
	Uint32 i, nobjs;

	obt = object_table_new();

	nobjs = read_uint32(fd);

	pthread_mutex_lock(&world->lock);
	for (i = 0; i < nobjs; i++) {
		struct object *pob;
		char *name, *type;

		name = read_string(fd, NULL);
		type = read_string(fd, NULL);
		pob = world_find(name);

		debug_n(DEBUG_DEPS, "\t%s: depends on %s...", objname, name);
		if (pob != NULL) {
			debug_n(DEBUG_DEPS, "%p (%s)\n", pob, type);
			if (strcmp(pob->type, type) != 0) {
				fatal("%s: expected `%s' to be of type `%s'\n",
				    objname, name, type);
			}
		} else {
			debug_n(DEBUG_DEPS, "missing\n");
		}
		object_table_insert(obt, pob);
		free(name);
		free(type);
	}
	pthread_mutex_unlock(&world->lock);

	return (obt);
}

