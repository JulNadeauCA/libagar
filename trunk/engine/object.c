/*	$Csoft: object.c,v 1.118 2003/04/14 08:56:20 vedge Exp $	*/

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

#include <engine/compat/snprintf.h>
#include <engine/compat/strlcat.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/config.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>
#include <engine/view.h>
#include <engine/rootmap.h>
#include <engine/world.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static const struct version object_ver = {
	"agar object",
	1, 0
};

static const struct object_ops null_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

#ifdef DEBUG
#define DEBUG_STATE	0x001
#define DEBUG_POSITION	0x002
#define DEBUG_DEPS	0x004
#define DEBUG_SUBMAPS	0x008
#define DEBUG_CONTROL	0x010
#define DEBUG_ATTACH	0x020
#define DEBUG_GC	0x040

int	object_debug = DEBUG_STATE|DEBUG_POSITION|DEBUG_SUBMAPS|DEBUG_CONTROL|
	               DEBUG_ATTACH|DEBUG_GC;
#define engine_debug object_debug
#endif

struct object *
object_new(void *parent, char *type, char *name, char *media, int flags,
    const void *opsp)
{
	struct object *ob;

	ob = Malloc(sizeof(struct object));
	object_init(ob, type, name, media, flags, opsp);

	if (parent != NULL) {
		object_attach(parent, ob);
	}
	return (ob);
}

void
object_init(struct object *ob, char *type, char *name, char *media, int flags,
    const void *opsp)
{
	if (strlen(type) >= OBJECT_TYPE_MAX ||
	    strlen(name) >= OBJECT_NAME_MAX) {
		fatal("name/type too big");
	}
	ob->type = Strdup(type);
	ob->name = Strdup(name);
	
	ob->ops = (opsp != NULL) ? opsp : &null_ops;
	ob->flags = flags;
	ob->pos = NULL;
	ob->state = OBJECT_EMBRYONIC;
	SLIST_INIT(&ob->childs);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	pthread_mutex_init(&ob->lock, NULL);
	pthread_mutex_init(&ob->props_lock, &recursive_mutexattr);
	pthread_mutex_init(&ob->events_lock, &recursive_mutexattr);

	ob->art = (ob->flags & OBJECT_ART) ? art_fetch(media, ob) : NULL;
}

/* Attach a child object to a parent object. */
void
object_attach(void *parentp, void *childp)
{
	struct object *parent = parentp;
	struct object *child = childp;

	pthread_mutex_lock(&parent->lock);
	SLIST_INSERT_HEAD(&parent->childs, child, wobjs);
	debug(DEBUG_ATTACH, "%s: attached %s\n", parent->name, child->name);
	pthread_mutex_unlock(&parent->lock);
}

/* Detach a child object from its parent. */
void
object_detach(void *parentp, void *childp)
{
	struct object *parent = parentp;
	struct object *child = childp;

	pthread_mutex_lock(&parent->lock);
	SLIST_REMOVE(&parent->childs, child, object, wobjs);
	debug(DEBUG_ATTACH, "%s: detached %s\n", parent->name, child->name);
	pthread_mutex_unlock(&parent->lock);
}

/* Detach and free child objects. */
void
object_free_childs(struct object *ob)
{
	struct object *cob, *ncob;

	pthread_mutex_lock(&ob->lock);
	for (cob = SLIST_FIRST(&ob->childs);
	     cob != SLIST_END(&ob->childs);
	     cob = ncob) {
		ncob = SLIST_NEXT(ob, wobjs);
		if (cob->flags & OBJECT_STATIC)
			continue;
		debug(DEBUG_GC, "%s: freeing %s\n", ob->name, cob->name);
		object_destroy(cob);
		free(cob);
	}
	SLIST_INIT(&ob->childs);
	pthread_mutex_unlock(&ob->lock);
}

/* Clear an object's property table. */
void
object_free_props(struct object *ob)
{
	struct prop *prop, *nextprop;

	pthread_mutex_lock(&ob->props_lock);
	for (prop = TAILQ_FIRST(&ob->props);
	     prop != TAILQ_END(&ob->props);
	     prop = nextprop) {
		nextprop = TAILQ_NEXT(prop, props);
		prop_destroy(prop);
		free(prop);
	}
	TAILQ_INIT(&ob->props);
	pthread_mutex_unlock(&ob->props_lock);
}
	
/* Clear an object's event handler list. */
void
object_free_events(struct object *ob)
{
	struct event *eev, *nexteev;

	pthread_mutex_lock(&ob->events_lock);
	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = nexteev) {
		nexteev = TAILQ_NEXT(eev, events);
		free(eev);
	}
	TAILQ_INIT(&ob->events);
	pthread_mutex_unlock(&ob->events_lock);
}

void
object_destroy(void *p)
{
	struct object *ob = p;
	
	if (ob->ops->destroy != NULL)
		ob->ops->destroy(ob);

	if ((ob->flags & OBJECT_ART_CACHE) == 0 && ob->art != NULL)
		art_unused(ob->art);

	object_free_events(ob);
	object_free_props(ob);
	object_free_childs(ob);

	pthread_mutex_destroy(&ob->lock);
	pthread_mutex_destroy(&ob->events_lock);
	pthread_mutex_destroy(&ob->props_lock);

	free(ob->name);
	free(ob->type);
}

static int
object_load_data(struct object *ob, struct netbuf *buf)
{
	Uint32 i, nchilds;

	if (version_read(buf, &object_ver, NULL) == -1)
		return (-1);

	pthread_mutex_lock(&ob->lock);

	if (prop_load(ob, buf) == -1)
		goto fail;
	if (read_uint32(buf) > 0 &&
	    object_load_position(ob, buf) == -1)
		goto fail;

	nchilds = read_uint32(buf);
	for (i = 0; i < nchilds; i++) {
		char childname[OBJECT_NAME_MAX];
		struct object *child;

		if (copy_string(childname, buf, sizeof(childname)) >=
		    sizeof(childname)) {
			error_set("child object name too big");
			goto fail;
		}
		SLIST_FOREACH(child, &ob->childs, wobjs) {
			if (strcmp(child->name, childname) == 0)
				break;
		}
		if (child != NULL) {
			dprintf("%s: loading child: %s\n", ob->name,
			    child->name);
			if (object_load_data(child, buf) == -1)
				goto fail;
		} else {
			dprintf("%s: no child matches `%s'\n", ob->name,
			    childname);
		}
	}

	if (ob->ops->load != NULL &&
	    ob->ops->load(ob, buf) == -1)
		goto fail;

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

int
object_load(void *p, char *opath)
{
	char path[FILENAME_MAX];
	struct object *ob = p;
	struct netbuf buf;
	int fd, rv = 0;

	debug(DEBUG_STATE, "loading %s\n", ob->name);

	if (opath == NULL) {
		if (object_path(ob->name, ob->type, path, sizeof(path)) == -1)
			return (-1);
	} else {
		if (strlcpy(path, opath, sizeof(path)) >= sizeof(path)) {
			error_set("path too big");
			return (-1);
		}
	}	
	
	if ((fd = open(path, O_RDONLY)) == -1) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}

	netbuf_init(&buf, fd, NETBUF_BIG_ENDIAN);	
	rv = object_load_data(ob, &buf);
	netbuf_destroy(&buf);
	close(fd);
	return (rv);
}

static int
object_save_data(struct object *ob, struct netbuf *buf)
{
	struct object *child;
	off_t nchilds_offs;
	Uint32 nchilds = 0;

	version_write(buf, &object_ver);

	pthread_mutex_lock(&ob->lock);

	if (prop_save(ob, buf) == -1)
		goto fail;
	if (ob->pos != NULL) {
		write_uint32(buf, 1);
		object_save_position(ob, buf);
	} else {
		write_uint32(buf, 0);
	}

	nchilds_offs = buf->offs;
	write_uint32(buf, 0);				/* Skip count */
	SLIST_FOREACH(child, &ob->childs, wobjs) {
		write_string(buf, child->name);
		if (object_save_data(child, buf) == -1)
			goto fail;
		nchilds++;
	}
	pwrite_uint32(buf, nchilds, nchilds_offs);

	if (ob->ops->save != NULL &&			/* Save custom data */
	    ob->ops->save(ob, buf) == -1)
		goto fail;

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

int
object_save(void *p, char *opath)
{
	char path[FILENAME_MAX];
	struct object *ob = p;
	struct stat sta;
	struct netbuf buf;
	int fd, rv;

	if (opath != NULL) {
		if (strlcpy(path, opath, sizeof(path)) >= sizeof(path))
			goto toobig;
	} else {
		if (prop_copy_string(config, "path.user_data_dir",
		    path, sizeof(path)) >= sizeof(path))
			goto toobig;

		if (stat(path, &sta) == -1 &&
		    mkdir(path, 0700) == -1) {
			error_set("creating %s: %s", path, strerror(errno));
			return (-1);
		}
		if (strlcat(path, "/", sizeof(path)) >= sizeof(path) ||
		    strlcat(path, ob->type, sizeof(path)) >= sizeof(path))
			goto toobig;

		if (stat(path, &sta) == -1 &&
		    mkdir(path, 0700) == -1) {
			error_set("creating %s: %s", path, strerror(errno));
			return (-1);
		}
		if (strlcat(path, "/", sizeof(path)) >= sizeof(path) ||
		    strlcat(path, ob->name, sizeof(path)) >= sizeof(path) ||
		    strlcat(path, ".", sizeof(path)) >= sizeof(path) ||
		    strlcat(path, ob->type, sizeof(path)) >= sizeof(path))
			goto toobig;
	}
	
	debug(DEBUG_STATE, "saving %s to %s\n", ob->name, path);

	if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0600)) == -1) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	netbuf_init(&buf, fd, NETBUF_BIG_ENDIAN);
	rv = object_save_data(ob, &buf);
	netbuf_flush(&buf);
	netbuf_destroy(&buf);
	close(fd);
	return (rv);
toobig:
	error_set("path is too big");
	return (-1);
}

/* Search the data file directories. */
int
object_path(char *obname, const char *suffix, char *dst, size_t dst_size)
{
	char datapath[FILENAME_MAX];
	struct stat sta;
	char *p, *last;

	prop_copy_string(config, "path.data_path", datapath, sizeof(datapath));

	for (p = strtok_r(datapath, ":", &last);
	     p != NULL;
	     p = strtok_r(NULL, ":", &last)) {
		if (snprintf(dst, dst_size, "%s/%s/%s.%s", p, suffix, obname,
		    suffix) >= dst_size) {
			error_set("path too big");
			return (-1);
		}
		if (stat(dst, &sta) == 0)
			return (0);
	}
	error_set("cannot find %s.%s in %s", obname, suffix, datapath);
	return (-1);
}

/* Control an object's position with an input device. */
int
object_control(void *p, char *inname)
{
	struct object *ob = p;
	struct input *in;

	if ((in = input_find(inname)) == NULL)
		return (-1);

	pthread_mutex_lock(&ob->lock);
	if (ob->pos == NULL) {
		error_set("%s is nowhere\n", ob->name);
		goto fail;
	}
	debug(DEBUG_CONTROL, "%s: control with <%s>\n", ob->name,
	    OBJECT(in)->name);
	ob->pos->input = in;
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

void
object_center(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	if (ob->pos != NULL && view->gfx_engine == GFX_ENGINE_TILEBASED) {
		dprintf("%s: centering\n", ob->name);
		ob->pos->dir.flags |= DIR_SCROLLVIEW;
		rootmap_center(ob->pos->map, ob->pos->x, ob->pos->y);
	}
	pthread_mutex_unlock(&ob->lock);
}

/* Copy the current submap to the level map. */
static void
object_blit_submap(struct object_position *pos)
{
	int x, y;

	dprintf("blit: [%d,%d,%d]+%dx%d\n", pos->x, pos->y, pos->layer,
	    pos->submap->mapw, pos->submap->maph);

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			struct node *srcnode = &pos->submap->map[y][x];
			struct node *dstnode = &pos->map->map[y][x];
			struct noderef *nref, *nnref;

			TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
				nnref = node_copy_ref(nref, dstnode);
				nnref->layer = pos->layer;
			}
		}
	}
}

/* Clear the copy of the current submap from the level map. */
static void
object_unblit_submap(struct object_position *pos)
{
	int x, y;
	
	dprintf("unblit: [%d,%d,%d]+%dx%d\n", pos->x, pos->y, pos->layer,
	    pos->submap->mapw, pos->submap->maph);

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			node_clear_layer(&pos->map->map[y][x], pos->layer);
		}
	}
}

void
object_load_submap(void *p, char *name)
{
	struct object *ob = p;
	struct map *sm;

	sm = Malloc(sizeof(struct map));
	map_init(sm, name, ob->name);
	if (object_load(sm, NULL) == 0) {
		object_attach(ob, sm);
	} else {
		fatal("loading %s: %s", name, error_get());
	}
}

/* Change the current submap of an object. */
int
object_set_submap(void *p, char *name)
{
	struct object *ob = p;
	struct object *submap;

	pthread_mutex_lock(&ob->lock);
	
	debug(DEBUG_SUBMAPS, "%s: %s -> %s\n", ob->name,
	    ob->pos != NULL ? OBJECT(ob->pos->submap)->name : "none", name);

	SLIST_FOREACH(submap, &ob->childs, wobjs) {
		if (strcmp(submap->name, name) == 0)
			break;
	}
	if (submap == NULL) {
		error_set("no such submap");
		goto fail;
	}
	if (ob->pos != NULL) {
		object_unblit_submap(ob->pos);
		ob->pos->submap = (struct map *)submap;
	}

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Set the position of an object inside a map. */
void
object_set_position(void *p, struct map *dstmap, int x, int y, int layer)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	
	debug(DEBUG_POSITION, "%s: position -> %s:[%d,%d,%d]\n",
	    ob->name, OBJECT(dstmap)->name, x, y, layer);

	if (ob->pos == NULL) {
		ob->pos = Malloc(sizeof(struct object_position));
		ob->pos->map = NULL;
		ob->pos->x = 0;
		ob->pos->y = 0;
		ob->pos->layer = 0;
		ob->pos->center = 0;
		ob->pos->submap = NULL;
		ob->pos->input = NULL;
		mapdir_init(&ob->pos->dir, ob, dstmap, DIR_SOFTSCROLL, 1);
	} else {
		object_unblit_submap(ob->pos);
	}

	pthread_mutex_lock(&dstmap->lock);
	if (ob->pos->x >= 0 && ob->pos->x < dstmap->mapw &&
	    ob->pos->y >= 0 && ob->pos->y < dstmap->maph &&
	    layer >= 0 && layer < dstmap->nlayers) {
		ob->pos->map = dstmap;
		ob->pos->x = x;
		ob->pos->y = y;
		ob->pos->layer = layer;
		object_blit_submap(ob->pos);
	} else {
		debug(DEBUG_POSITION, "%s: bad coords %s:[%d,%d,%d]\n",
		    ob->name, OBJECT(dstmap)->name, x, y, layer);
		free(ob->pos);
		ob->pos = NULL;
	}
	pthread_mutex_unlock(&dstmap->lock);

	pthread_mutex_unlock(&ob->lock);
}

void
object_unset_position(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	debug(DEBUG_POSITION, "%s: unset %p\n", ob->name, ob->pos);
	if (ob->pos != NULL) {
		object_unblit_submap(ob->pos);
		free(ob->pos);
		ob->pos = NULL;
	}
	pthread_mutex_unlock(&ob->lock);
}

void
object_save_position(void *p, struct netbuf *buf)
{
	struct object *ob = p;
	struct object_position *pos = ob->pos;
		
	write_string(buf, OBJECT(pos->map)->name);
	write_uint32(buf, (Uint32)pos->x);
	write_uint32(buf, (Uint32)pos->y);
	write_uint8(buf, (Uint8)pos->layer);
	write_uint8(buf, (Uint8)pos->center);
	write_string(buf, (pos->submap != NULL) ?
	    OBJECT(pos->submap)->name : "");
	write_string(buf, (pos->input != NULL) ?
	    OBJECT(pos->input)->name : "");
}

int
object_load_position(void *p, struct netbuf *buf)
{
	char map_id[OBJECT_NAME_MAX];
	char submap_id[OBJECT_NAME_MAX];
	char input_id[OBJECT_NAME_MAX];
	int x, y, layer, center;
	struct map *dst_map;
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);

	if (copy_string(map_id, buf, sizeof(map_id)) >= sizeof(map_id)) {
		error_set("map_id too big");
		goto fail;
	}
	x = (int)read_uint32(buf);
	y = (int)read_uint32(buf);
	layer = (int)read_uint8(buf);
	center = (int)read_uint8(buf);

	if (copy_string(submap_id, buf, sizeof(submap_id)) >=
	    sizeof(submap_id)) {
		error_set("submap_id too big");
		goto fail;
	}
	if (copy_string(input_id, buf, sizeof(input_id)) >= sizeof(input_id)) {
		error_set("input_id too big");
		goto fail;
	}
	if ((dst_map = world_find(map_id)) == NULL)
		goto fail;
	debug(DEBUG_STATE, "%s: at %s:[%d,%d,%d]%s\n", ob->name,
	    OBJECT(dst_map)->name, x, y, layer, center ? ", centered" : "");

	pthread_mutex_lock(&dst_map->lock);
	if (x < 0 || x >= dst_map->mapw ||
	    y < 0 || y >= dst_map->maph ||
	    layer < 0 || layer >= dst_map->nlayers) {
		error_set("bad coords: %d,%d,%d", x, y, layer);
		pthread_mutex_unlock(&dst_map->lock);
		goto fail;
	}
	if (object_set_submap(ob, submap_id) == -1) {
		pthread_mutex_unlock(&dst_map->lock);
		goto fail;
	}
	object_set_position(ob, dst_map, x, y, layer);
	pthread_mutex_unlock(&dst_map->lock);

	if (input_id[0] != '\0') {
		if (object_control(ob, input_id) == -1)
			debug(DEBUG_STATE, "%s: %s\n", ob->name, error_get());
	} else {
		debug(DEBUG_STATE, "%s: no controller\n", ob->name);
	}
	
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

void
object_table_init(struct object_table *obt)
{
	obt->objs = NULL;
	obt->used = NULL;
	obt->nobjs = 0;
}

void
object_table_destroy(struct object_table *obt)
{
	Free(obt->objs);
	Free(obt->used);
}

void
object_table_insert(struct object_table *obt, struct object *obj)
{
	if (obt->objs != NULL) {
		obt->objs = Realloc(obt->objs, (obt->nobjs + 1) *
		    sizeof(struct object *));
		obt->used = Realloc(obt->used, (obt->nobjs + 1) *
		    sizeof(int));
	} else {
		obt->objs = Malloc(sizeof(struct object *));
		obt->used = Malloc(sizeof(int));
	}
	obt->objs[obt->nobjs] = obj;
	obt->used[obt->nobjs] = 0;
	obt->nobjs++;
}

void
object_table_save(struct object_table *obt, struct netbuf *buf)
{
	off_t nobjs_offs;
	struct object *pob;
	Uint32 i;

	nobjs_offs = buf->offs;
	write_uint32(buf, 0);				/* Skip */
	for (i = 0; i < obt->nobjs; i++) {
		pob = obt->objs[i];
		write_string(buf, pob->name);
		write_string(buf, pob->type);
	}
	pwrite_uint32(buf, obt->nobjs, nobjs_offs);
}

int
object_table_load(struct object_table *obt, struct netbuf *buf, char *objname)
{
	char name[OBJECT_NAME_MAX];
	char type[OBJECT_TYPE_MAX];
	struct object *pob;
	Uint32 i, nobjs;

	nobjs = read_uint32(buf);

	pthread_mutex_lock(&world->lock);
	for (i = 0; i < nobjs; i++) {
		if (copy_string(name, buf, sizeof(name)) >= sizeof(name)) {
			error_set("object name too big");
			goto fail;
		}
		if (copy_string(type, buf, sizeof(type)) >= sizeof(type)) {
			error_set("object type too big");
			goto fail;
		}
		debug_n(DEBUG_DEPS, "\t%s: depends on %s...", objname, name);
		if ((pob = world_find(name)) != NULL) {
			debug_n(DEBUG_DEPS, "%p (%s)\n", pob, type);
			if (strcmp(pob->type, type) != 0) {
				error_set("%s: expected %s to be of type %s",
				    objname, name, type);
				goto fail;
			}
		} else {
			debug_n(DEBUG_DEPS, "missing\n");
		}
		object_table_insert(obt, pob);
	}
	pthread_mutex_unlock(&world->lock);
	return (0);
fail:
	pthread_mutex_unlock(&world->lock);
	return (-1);
}

