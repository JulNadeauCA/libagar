/*	$Csoft: object.c,v 1.80 2002/09/06 01:25:04 vedge Exp $	*/

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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "engine.h"
#include "config.h"
#include "map.h"
#include "physics.h"
#include "input.h"

extern int mapediting;

static const struct object_ops null_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

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
	world_attach(world, ob);
	pthread_mutex_unlock(&world->lock);

	return (ob);
}

void
object_init(struct object *ob, char *type, char *name, char *media, int flags,
    const void *opsp)
{
	pthread_mutexattr_t attr;

	ob->type = strdup(type);
	ob->name = strdup(name);
	ob->desc = NULL;
	sprintf(ob->saveext, "ag");
	ob->ops = (opsp != NULL) ? opsp : &null_ops;
	ob->flags = flags;
	ob->pos = NULL;
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	pthread_mutex_init(&ob->pos_lock, NULL);
	pthread_mutex_init(&ob->props_lock, NULL);

	pthread_mutexattr_init(&ob->events_lockattr);
	pthread_mutexattr_settype(&ob->events_lockattr,
	    PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&ob->events_lock, &ob->events_lockattr);

	ob->art = (ob->flags & OBJECT_ART) ?
	    media_get_art(media, ob) : NULL;
	ob->audio = (ob->flags & OBJECT_AUDIO) ?
	    media_get_audio(media, ob) : NULL;
}

/* Object must not be attached. */
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

	if ((ob->flags & OBJECT_KEEP_MEDIA) == 0) {
		if (ob->art != NULL)
			OBJECT_UNUSED(ob, art);
		if (ob->audio != NULL)
			OBJECT_UNUSED(ob, audio);
	}

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
		free(prop);
	}
	
	pthread_mutex_destroy(&ob->pos_lock);
	pthread_mutex_destroy(&ob->events_lock);
	pthread_mutex_destroy(&ob->props_lock);
	pthread_mutexattr_destroy(&ob->events_lockattr);

	if (ob->name != NULL)
		free(ob->name);
	if (ob->type != NULL)
		free(ob->type);
	if (ob->desc != NULL)
		free(ob->desc);

	free(ob);
}

/* Load an object from an alternate file. */
int
object_loadfrom(void *p, char *path)
{
	struct object *ob = p;
	int fd;

	if (OBJECT_OPS(ob)->load == NULL) {
		return (-1);
	}

	/* XXX mmap? */
	fd = open(path, O_RDONLY, 00600);
	if (fd < 0) {
		dprintf("%s: %s\n", path, strerror(errno));
		return (-1);
	}
	
	/* Load generic properties. */
	if (prop_load(ob, fd) != 0) {
		close(fd);
		return (-1);
	}

	/* Load object specific data. */
	if (OBJECT_OPS(ob)->load(ob, fd) != 0) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (0);
}

/* Load an object from its default location. */
int
object_load(void *p)
{
	struct object *ob = p;
	char *path;
	int fd, rv = 0;

	path = object_path(ob->name, ob->saveext);
	if (path == NULL) {
		dprintf("%s.%s: %s\n", ob->name, ob->saveext, error_get());
		return (-1);
	}

	/* XXX mmap? */
	fd = open(path, O_RDONLY, 00600);
	if (fd < 0) {
		dprintf("%s: %s\n", path, strerror(errno));
		return (-1);
	}

	/* Load generic properties. */
	if (prop_load(ob, fd) != 0) {
		close(fd);
		return (-1);
	}

	if (OBJECT_OPS(ob)->load != NULL) {
		/* Load object specific data. */
		rv = OBJECT_OPS(ob)->load(ob, fd);
	}

	close(fd);
	return (rv);
}

/* Save an object state. */
int
object_save(void *p)
{
	struct object *ob = p;
	char path[FILENAME_MAX];
	int fd;

	/* Save maps inside the maps subdirectory. XXX */
	if (strcmp(ob->saveext, "m") == 0) {
		sprintf(path, "%s/maps/%s.m",
		    prop_string(config, "path.user_data_dir"),
		    ob->name);
	} else {
		sprintf(path, "%s/%s.%s",
		    prop_string(config, "path.user_data_dir"),
		    ob->name, ob->saveext);
	}

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 00600);
	if (fd == -1) {
		fatal("%s: %s\n", path, strerror(errno));
		return (-1);
	}

	/* Save generic properties. */
	if (prop_save(ob, fd) != 0) {
		close(fd);
		return (-1);
	}

	if (OBJECT_OPS(ob)->save != NULL) {
		/* Save object specific data. */
		if (OBJECT_OPS(ob)->save(ob, fd) != 0) {
			close(fd);
			return (-1);
		}
	}
	close(fd);
	return (0);
}

/*
 * Search for an object matching the given string.
 * The world must be locked.
 * XXX hash
 */
struct object *
object_strfind(char *s)
{
	struct object *ob;

	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		if (strcmp(ob->name, s) == 0) {
			return (ob);
		}
	}
	return (NULL);
}

/* Search the data file directories for the given file. */
char *
object_path(char *obname, const char *suffix)
{
	struct stat sta;
	char *p, *last;
	char *datapath, *datapathp, *path;

	path = emalloc((size_t)FILENAME_MAX);
	datapathp = datapath = strdup(prop_string(config, "path.data_path"));

	for (p = strtok_r(datapath, ":;", &last);
	     p != NULL;
	     p = strtok_r(NULL, ":;", &last)) {
	     	if (strcmp(suffix, "m") == 0) {
			sprintf(path, "%s/maps/%s.m", p, obname);
		} else {
			sprintf(path, "%s/%s.%s", p, obname, suffix);
		}
		if (stat(path, &sta) == 0) {
			free(datapathp);
			return (path);
		}
	}
	free(datapathp);
	free(path);

	error_set("cannot find %s.%s", obname, suffix);
	return (NULL);
}

/* XXX too intricate */

/*
 * Add a noderef at m:x,y, and a back reference to it.
 * Map must be locked, ob->pos must not.
 */
struct mappos *
object_addpos(void *p, Uint32 offs, Uint32 flags, struct input *in,
    struct map *m, Uint32 x, Uint32 y)
{
	struct object *ob = p;
	struct node *node;
	struct mappos *pos;

	node = &m->map[y][x];
	pos = emalloc(sizeof(struct mappos));
	pos->map = m;
	pos->x = x;
	pos->y = y;
	pos->speed = 1;

	/* Add a noderef at m:x,y. */
	pos->nref = node_addref(node, ob, offs, flags);
	if (pos->nref == NULL) {
		free(pos);
		return (NULL);
	}

	/* Display smooth transitions from one node to another. */
	node->flags |= NODE_ANIM;
	if (y > 1) {
		m->map[y - 1][x].overlap++;
	}
	mapdir_init(&pos->dir, ob, m, DIR_SCROLLVIEW|DIR_SOFTSCROLL, 3);

	/* Set the input device. */
	pos->input = in;
	if (in != NULL) {
		in->pos = pos;	/* XXX lock */
	}

	/* Link this back reference to its object. */
	pthread_mutex_lock(&ob->pos_lock);
	ob->pos = pos;
	pthread_mutex_unlock(&ob->pos_lock);
	
	return (pos);
}

/*
 * Remove the only back reference of this object.
 * Map must be locked.
 */
void
object_delpos(void *obp)	/* XXX will change */
{
	struct object *ob = obp;
	struct mappos *pos;
	
	pthread_mutex_lock(&ob->pos_lock);
	if (ob->pos != NULL) {
		pos = ob->pos;
		if (pos->map != NULL) {
			struct node *node;
	
			node = &pos->map->map[pos->y][pos->x];
			node_delref(node, pos->nref);
			node->flags &= ~(NODE_ANIM);
			if (pos->y > 1) {
				pos->map->map[pos->y - 1][pos->x].overlap--;
			}
		}
		free(pos);
		ob->pos = NULL;
	} else {
		dprintf("%s: no position\n", ob->name);
	}
	pthread_mutex_unlock(&ob->pos_lock);
}

/*
 * Move a noderef from its current node to m:x,y and update
 * the back reference. Return the new position.
 *
 * Map must be locked, ob->pos must not.
 */
struct mappos *
object_movepos(void *obp, struct map *m, Uint32 x, Uint32 y)
{
	struct mappos oldpos, *pos, *newpos;
	struct noderef *oldnref;
	struct object *ob = obp;

	/* Obtain the object's position. */
	pthread_mutex_lock(&ob->pos_lock);
	pos = ob->pos;
	if (pos == NULL) {
		dprintf("%s: no position\n", ob->name);
		pthread_mutex_unlock(&ob->pos_lock);
		return (NULL);
	}
	oldpos = *pos;
	pthread_mutex_unlock(&ob->pos_lock);
	
	/* Save and remove the old reference. */
	oldnref = oldpos.nref;
	object_delpos(oldnref->pobj);

	/* Insert at the new position. */
	newpos = object_addpos(oldnref->pobj, oldnref->offs, oldnref->flags,
	    oldpos.input, m, x, y);
	newpos->dir = oldpos.dir;

	m->redraw++;

	return (newpos);
}

