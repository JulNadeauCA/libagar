/*	$Csoft: object.c,v 1.130 2003/06/14 00:23:11 vedge Exp $	*/

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

#include <engine/compat/asprintf.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/config.h>
#include <engine/map.h>
#include <engine/input.h>
#include <engine/view.h>
#include <engine/rootmap.h>
#include <engine/typesw.h>
#include <engine/mkpath.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

const struct version object_ver = {
	"agar object",
	1, 0
};

const struct object_ops object_ops = {
	NULL,	/* init */
	NULL,	/* destroy */
	NULL,	/* load */
	NULL,	/* save */
	NULL	/* edit */
};

#ifdef DEBUG
#define DEBUG_STATE	0x001
#define DEBUG_POSITION	0x002
#define DEBUG_DEPS	0x004
#define DEBUG_SUBMAPS	0x008
#define DEBUG_CONTROL	0x010
#define DEBUG_LINKAGE	0x020
#define DEBUG_GC	0x040

int	object_debug = DEBUG_STATE|DEBUG_POSITION|DEBUG_SUBMAPS|DEBUG_CONTROL|
	               DEBUG_GC|DEBUG_DEPS;
#define engine_debug object_debug
#endif

/* Allocate, initialize and attach a generic object. */
struct object *
object_new(void *parent, const char *type, const char *name, const void *opsp)
{
	struct object *ob;

	ob = Malloc(sizeof(struct object));
	object_init(ob, type, name, opsp);

	if (parent != NULL) {
		object_attach(parent, ob);
	}
	return (ob);
}

/* Initialize a generic object structure. */
void
object_init(void *p, const char *type, const char *name, const void *opsp)
{
	struct object *ob = p;

	if (strlcpy(ob->type, type, sizeof(ob->type)) >= sizeof(ob->type))
		fatal("type too big");
	if (strlcpy(ob->name, name, sizeof(ob->name)) >= sizeof(ob->name))
		fatal("name too big");

	ob->ops = (opsp != NULL) ? opsp : &object_ops;
	ob->parent = NULL;
	ob->flags = 0;
	ob->art = NULL;
	ob->pos = NULL;
	TAILQ_INIT(&ob->childs);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	pthread_mutex_init(&ob->lock, &recursive_mutexattr);
	pthread_mutex_init(&ob->props_lock, &recursive_mutexattr);
	pthread_mutex_init(&ob->events_lock, &recursive_mutexattr);
}

/* Recursive function to construct absolute object names. */
static void
object_name_search(void *obj, char **s, size_t *sl)
{
	struct object *ob = obj;
	size_t namelen;
	char *sp;

	namelen = strlen(ob->name);
	*sl += namelen+1;
	sp = *s = Realloc(*s, *sl);

	memmove(sp+namelen+1, sp, (*sl)-namelen-1);
	sp[0] = '/';
	memcpy(sp+1, ob->name, namelen);

	if (ob->parent != NULL)
		object_name_search(ob->parent, s, sl);
}

/* Return the absolute name of an object. */
/* XXX cache the absolute names? */
char *
object_name(void *obj)
{
	struct object *ob = obj;
	size_t sl, namelen;
	char *s;

	namelen = strlen(ob->name);
	sl = 1+namelen+1;
	s = Malloc(sl);

	s[0] = '/';
	memcpy(s+1, ob->name, namelen);

	lock_linkage();
	if (ob->parent != NULL) {
		object_name_search(ob->parent, &s, &sl);
	}
	unlock_linkage();

	s[sl-1] = '\0';
	return (s);
}

/* Move an object from a parent to another. */
void
object_move(void *oldparentp, void *childp, void *newparentp)
{
	struct object *oldparent = oldparentp;
	struct object *child = childp;
	struct object *newparent = newparentp;

	lock_linkage();

	TAILQ_REMOVE(&oldparent->childs, child, cobjs);
	child->parent = NULL;
	event_post(child, "detached", "%p", oldparent);

	TAILQ_INSERT_TAIL(&newparent->childs, child, cobjs);
	child->parent = newparent;
	event_post(child, "attached", "%p", newparent);
	event_post(child, "moved", "%p, %p", oldparent, newparent);
	debug(DEBUG_LINKAGE, "%s: moved %s to %s\n", oldparent->name,
	    child->name, newparent->name);

	unlock_linkage();
}

/* Attach a child object to a parent. */
void
object_attach(void *parentp, void *childp)
{
	struct object *parent = parentp;
	struct object *child = childp;

	lock_linkage();
	TAILQ_INSERT_TAIL(&parent->childs, child, cobjs);
	child->parent = parent;
	event_post(child, "attached", "%p", parent);
	debug(DEBUG_LINKAGE, "%s: attached %s\n", parent->name, child->name);
	unlock_linkage();
}

/* Detach a child object from its parent. */
void
object_detach(void *parentp, void *childp)
{
	struct object *parent = parentp;
	struct object *child = childp;

	lock_linkage();
	TAILQ_REMOVE(&parent->childs, child, cobjs);
	child->parent = NULL;
	event_post(child, "detached", "%p", parent);
	debug(DEBUG_LINKAGE, "%s: detached %s\n", parent->name, child->name);
	unlock_linkage();
}

/* Traverse the object tree using a pathname. */
static void *
object_find_child(struct object *parent, const char *name)
{
	char cname[OBJECT_NAME_MAX], *s;
	struct object *child;

	dprintf("%s\n", name);
	TAILQ_FOREACH(child, &parent->childs, cobjs) {
		if (strcmp(child->name, name) != 0)
			continue;

		if ((s = strchr(cname, '/')) != NULL) {
			strlcpy(cname, s, sizeof(cname));
			if ((s = strchr(cname, '/')) != NULL) {
				*s = '\0';
			}
			return (object_find_child(child, cname));
		}
	}
	return (NULL);
}

/* Search for the named object. The name is relative to the parent. */
void *
object_find(void *parentp, const char *name)
{
	char cname[OBJECT_NAME_MAX], *s;
	struct object *parent = parentp;
	void *rv;

	if (parent == NULL)
		parent = world;

	strlcpy(cname, name, sizeof(cname));
	if ((s = strchr(cname, '/')) != NULL)
		*s = '\0';

	lock_linkage();
	rv = object_find_child(parent, cname);
	unlock_linkage();
	return (rv);
}

/* Detach and free child objects. */
void
object_free_childs(struct object *pob)
{
	struct object *cob, *ncob;

	pthread_mutex_lock(&pob->lock);
	for (cob = TAILQ_FIRST(&pob->childs);
	     cob != TAILQ_END(&pob->childs);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		debug(DEBUG_GC, "%s: freeing %s\n", pob->name, cob->name);
		object_destroy(cob);
		if ((cob->flags & OBJECT_STATIC) == 0)
			free(cob);
	}
	TAILQ_INIT(&pob->childs);
	pthread_mutex_unlock(&pob->lock);
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

/* Import graphical data into an object. */
int
object_load_art(void *p, const char *key, int wired)
{
	struct object *ob = p;

	if ((ob->art = art_fetch(ob, key)) == NULL)
		return (-1);

	if (wired) {
		art_wire(ob->art);
	}
	return (0);
}

/* Release the resources allocated by an object and its child objects. */
void
object_destroy(void *p)
{
	struct object *ob = p;

	object_free_childs(ob);
	object_free_events(ob);
	object_free_props(ob);
	
	if (ob->ops->destroy != NULL)
		ob->ops->destroy(ob);
	
	if (ob->art != NULL)
		art_unused(ob->art);

	pthread_mutex_destroy(&ob->lock);
	pthread_mutex_destroy(&ob->events_lock);
	pthread_mutex_destroy(&ob->props_lock);
}

/* Return the full pathname to an object's data file on disk. */
static char *
object_file(struct object *ob)
{
	struct stat sta;
	char *path, *dir, *last;
	char *objname;

	path = prop_get_string(config, "load-path");
	objname = object_name(ob);
	for (dir = strtok_r(path, ":", &last);
	     dir != NULL;
	     dir = strtok_r(NULL, ":", &last)) {
		char *file;

		Asprintf(&file, "%s%s/%s.%s", dir, objname, ob->name, ob->type);
		if (stat(file, &sta) == 0) {
			return (file);
		}
		free(file);
	}
	error_set("%s.%s not in <load-path>", ob->name, ob->type);
	free(path);
	free(objname);
	return (NULL);
}

/* Load the state of an object and its descendants. */
int
object_load(void *p)
{
	struct object *ob = p;
	char *path;
	struct netbuf buf;
	int fd;
	Uint32 i, nchilds;

	if ((path = object_file(ob)) == NULL) {
		dprintf("%s\n", error_get());
		return (-1);
	}

	debug(DEBUG_STATE, "loading `%s' from `%s'\n", ob->name, path);
	if ((fd = open(path, O_RDONLY)) == -1) {
		error_set("%s: %s", path, strerror(errno));
		free(path);
		return (-1);
	}
	free(path);

	netbuf_init(&buf, fd, NETBUF_BIG_ENDIAN);	
	if (version_read(&buf, &object_ver, NULL) == -1) {
		netbuf_destroy(&buf);
		close(fd);
		return (-1);
	}

	lock_linkage();
	pthread_mutex_lock(&ob->lock);
	if (prop_load(ob, &buf) == -1)
		goto fail;
	if (read_uint32(&buf) > 0 &&
	    object_load_position(ob, &buf) == -1)
		goto fail;
	if (ob->flags & OBJECT_RELOAD_CHILDS) {
		fatal("not yet");
	} else {
		object_free_childs(ob);

		/* Load the descendants. */
		nchilds = read_uint32(&buf);
		for (i = 0; i < nchilds; i++) {
			char cname[OBJECT_NAME_MAX];
			char ctype[OBJECT_TYPE_MAX];
			struct object *child;
			Uint32 cflags;
			const struct object_type *type;
			int ti;

			if (copy_string(cname, &buf, sizeof(cname)) >=
			    sizeof(cname)) {
				error_set("child name too big");
				goto fail;
			}
			if (copy_string(ctype, &buf, sizeof(ctype)) >=
			    sizeof(ctype)) {
				error_set("child type too big");
				goto fail;
			}
			cflags = read_uint32(&buf);

			/*
			 * Find out how much memory to allocate and which
			 * function to call for initialization.
			 */
		 	for (ti = 0, type = NULL; ti < ntypesw; ti++) {
				if (strcmp(typesw[ti].type, ctype) == 0) {
					type = &typesw[ti];
					break;
				}
			}
			if (type == NULL) {
				error_set("unknown type `%s'", ctype);
				goto fail;
			}

			dprintf("%s: %s (%lu bytes)\n", type->type,
			    type->desc, (unsigned long)type->size);
			child = Malloc(type->size);
			if (type->ops->init != NULL) {
				type->ops->init(child, cname);
			} else {
				object_init(child, ctype, cname, type->ops);
			}
			child->flags |= cflags;

			object_attach(ob, child);
			if (object_load(child) == -1)
				goto fail;
		}
	}
	if (ob->ops->load != NULL &&
	    ob->ops->load(ob, &buf) == -1) {
		goto fail;
	}
	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	netbuf_destroy(&buf);
	close(fd);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	netbuf_destroy(&buf);
	close(fd);
	return (-1);
}

/* Recursively save the state of an object and its descendents. */
/* XXX remove previous dir contents */
int
object_save(void *p)
{
	char *s, *savepath, *savedir, *file;
	struct object *ob = p;
	struct stat sta;
	struct netbuf buf;
	struct object *child;
	off_t nchilds_offs;
	Uint32 nchilds = 0;
	int fd;

	savepath = prop_get_string(config, "save-path");
	s = object_name(ob);
	Asprintf(&savedir, "%s%s", savepath, s);
	free(savepath);
	free(s);

	if (stat(savedir, &sta) == -1 &&
	    mkpath(savedir, 0700, 0700) == -1) {
		error_set("mkpath %s: %s", savedir, strerror(errno));
		free(savedir);
		return (-1);
	}
	Asprintf(&file, "%s/%s.%s", savedir, ob->name, ob->type);
	free(savedir);

	debug(DEBUG_STATE, "saving `%s' to `%s'\n", ob->name, file);
	if ((fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0600)) == -1) {
		error_set("%s: %s", file, strerror(errno));
		free(file);
		return (-1);
	}
	free(file);

	netbuf_init(&buf, fd, NETBUF_BIG_ENDIAN);
	version_write(&buf, &object_ver);

	lock_linkage();
	pthread_mutex_lock(&ob->lock);
	if (prop_save(ob, &buf) == -1) {
		goto fail;
	}
	if (ob->pos != NULL) {
		write_uint32(&buf, 1);
		object_save_position(ob, &buf);
	} else {
		write_uint32(&buf, 0);
	}

	nchilds_offs = buf.offs;
	write_uint32(&buf, 0);				/* Skip count */
	TAILQ_FOREACH(child, &ob->childs, cobjs) {	/* Save descendents */
		write_string(&buf, child->name);
		write_string(&buf, child->type);
		write_uint32(&buf, child->flags);

		if (object_save(child) == -1)
			goto fail;

		nchilds++;
	}
	pwrite_uint32(&buf, nchilds, nchilds_offs);	/* Write count */
	if (ob->ops->save != NULL &&			/* Save custom data */
	    ob->ops->save(ob, &buf) == -1) {
		goto fail;
	}

	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	netbuf_flush(&buf);
	netbuf_destroy(&buf);
	close(fd);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	netbuf_destroy(&buf);
	close(fd);
	return (-1);
}

/* Control an object's position with an input device. */
int
object_control(void *p, const char *inname)
{
	struct object *ob = p;
	struct input *in;

	if ((in = input_find(inname)) == NULL) {
		error_set("no such input device: `%s'", inname);
		return (-1);
	}

	pthread_mutex_lock(&ob->lock);
	if (ob->pos == NULL) {
		error_set("no position");
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

void
object_center(void *p)
{
	struct object *ob = p;

	if (view->gfx_engine != GFX_ENGINE_TILEBASED) {
		dprintf("ignore\n");
		return;
	}

	pthread_mutex_lock(&ob->lock);
	if (ob->pos != NULL) {
		dprintf("%s at %d,%d\n", ob->name, ob->pos->x, ob->pos->y);
		ob->pos->dir.flags |= DIR_SCROLLVIEW;
		rootmap_center(ob->pos->map, ob->pos->x, ob->pos->y);
	}
	pthread_mutex_unlock(&ob->lock);
}

/* Copy the current submap to the level map. */
static void
object_project_submap(struct object_position *pos)
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
object_unproject_submap(struct object_position *pos)
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

/* Load a submap of a given name. */
void
object_load_submap(void *p, const char *name)
{
	struct object *ob = p;
	struct map *sm;

	sm = Malloc(sizeof(struct map));
	map_init(sm, name);
	if (object_load(sm) == 0) {
		object_attach(ob, sm);
	} else {
		fatal("loading %s: %s", name, error_get());
	}
}

/* Change the current submap of an object. */
int
object_set_submap(void *p, const char *name)
{
	struct object *ob = p;
	struct object *submap;

	pthread_mutex_lock(&ob->lock);
	
	debug(DEBUG_SUBMAPS, "%s: %s -> %s\n", ob->name,
	    ob->pos != NULL ? OBJECT(ob->pos->submap)->name : "none", name);

	TAILQ_FOREACH(submap, &ob->childs, cobjs) {
		if (strcmp(submap->name, name) == 0)
			break;
	}
	if (submap == NULL) {
		error_set("no such submap");
		goto fail;
	}
	if (ob->pos != NULL) {
		object_unproject_submap(ob->pos);
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
		object_unproject_submap(ob->pos);
		object_detach(ob->pos->map, ob);
	}

	pthread_mutex_lock(&dstmap->lock);
	if (ob->pos->x >= 0 && ob->pos->x < dstmap->mapw &&
	    ob->pos->y >= 0 && ob->pos->y < dstmap->maph &&
	    layer >= 0 && layer < dstmap->nlayers) {
		ob->pos->map = dstmap;
		ob->pos->x = x;
		ob->pos->y = y;
		ob->pos->layer = layer;
		object_attach(ob->pos->map, ob);
		object_project_submap(ob->pos);
	} else {
		debug(DEBUG_POSITION, "%s: bad coords %s:[%d,%d,%d]\n",
		    ob->name, OBJECT(dstmap)->name, x, y, layer);
		free(ob->pos);
		ob->pos = NULL;
	}
	pthread_mutex_unlock(&dstmap->lock);

	pthread_mutex_unlock(&ob->lock);
}

/* Unset an object's position. */
void
object_unset_position(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	debug(DEBUG_POSITION, "%s: unset %p\n", ob->name, ob->pos);
	if (ob->pos != NULL) {
		object_detach(ob->pos->map, ob);
		object_unproject_submap(ob->pos);
		free(ob->pos);
		ob->pos = NULL;
	}
	pthread_mutex_unlock(&ob->lock);
}

/*
 * Save the position of an object relative to its parent map, as well as
 * the current submap and associated input device, if any.
 */
void
object_save_position(void *p, struct netbuf *buf)
{
	struct object *ob = p;
	struct object_position *pos = ob->pos;

	write_uint32(buf, (Uint32)pos->x);
	write_uint32(buf, (Uint32)pos->y);
	write_uint8(buf, (Uint8)pos->layer);
	write_uint8(buf, (Uint8)pos->center);
	write_string(buf, (pos->submap != NULL) ?
	    OBJECT(pos->submap)->name : "");
	write_string(buf, (pos->input != NULL) ?
	    OBJECT(pos->input)->name : "");
}

/*
 * Load the position of an object inside its parent map, as well as the
 * submap to display. Try to reassign the last used input device.
 */
int
object_load_position(void *p, struct netbuf *buf)
{
	char submap_id[OBJECT_NAME_MAX];
	char input_id[OBJECT_NAME_MAX];
	int x, y, layer, center;
	struct object *ob = p;
	struct map *m = ob->parent;

	pthread_mutex_lock(&ob->lock);
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
	debug(DEBUG_STATE, "%s: at [%d,%d,%d]%s\n", ob->name,
	    x, y, layer, center ? ", centered" : "");

	pthread_mutex_lock(&m->lock);
	if (x < 0 || x >= m->mapw ||
	    y < 0 || y >= m->maph ||
	    layer < 0 || layer >= m->nlayers) {
		error_set("bad coords: %d,%d,%d", x, y, layer);
		pthread_mutex_unlock(&m->lock);
		goto fail;
	}
	if (object_set_submap(ob, submap_id) == -1) {
		pthread_mutex_unlock(&m->lock);
		goto fail;
	}
	object_set_position(ob, m, x, y, layer);
	pthread_mutex_unlock(&m->lock);

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

/* Initialize a dependency table. */
void
object_table_init(struct object_table *obt)
{
	obt->objs = NULL;
	obt->used = NULL;
	obt->nobjs = 0;
}

/* Release the resources allocated by a dependency table. */
void
object_table_destroy(struct object_table *obt)
{
	Free(obt->objs);
	Free(obt->used);
}

/* Insert an object into a dependency table. */
void
object_table_insert(struct object_table *obt, struct object *obj)
{
	int i;

	for (i = 0; i < obt->nobjs; i++) {
		if (obt->objs[i] == obj)
			return;				/* Existing entry */
	}

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

/* Encode a dependency table. */
void
object_table_save(struct object_table *obt, struct netbuf *buf)
{
	off_t nobjs_offs;
	struct object *pob;
	Uint32 i;

	nobjs_offs = buf->offs;
	write_uint32(buf, 0);				/* Skip */
	for (i = 0; i < obt->nobjs; i++) {
		char *name;

		pob = obt->objs[i];
		name = object_name(pob);

		write_string(buf, pob->name);
		write_string(buf, pob->type);

		free(name);
	}
	pwrite_uint32(buf, obt->nobjs, nobjs_offs);
}

/* Decode a dependency table. */
int
object_table_load(struct object_table *obt, struct netbuf *buf,
    const char *objname)
{
	char name[OBJECT_NAME_MAX];
	char type[OBJECT_TYPE_MAX];
	struct object *pob;
	Uint32 i, nobjs;

	nobjs = read_uint32(buf);

	lock_linkage();
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
		if ((pob = object_find(world, name)) != NULL) {
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
	unlock_linkage();
	return (0);
fail:
	unlock_linkage();
	return (-1);
}

void
object_set_ops(void *p, const void *ops)
{
	OBJECT(p)->ops = ops;
}

enum {
	LOAD_OP,
	SAVE_OP,
	EDIT_OP
};

static void
object_invoke_op(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	int op = argv[2].i;

	switch (op) {
	case LOAD_OP:
		if (object_load(ob) == -1) {
			text_msg("Error loading", "%s: %s", ob->name,
			    error_get());
		}
		break;
	case SAVE_OP:
		if (object_save(ob) == -1) {
			text_msg("Error saving", "%s: %s", ob->name,
			    error_get());
		}
		break;
	case EDIT_OP:
		if (ob->ops->edit != NULL) {
			ob->ops->edit(ob);
		} else {
			text_msg("Error", "%s has no edit op", ob->name);
		}
		break;
	}
}

static void
poll_props(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct prop *prop;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ob->props_lock);
	TAILQ_FOREACH(prop, &ob->props, props) {
		char val[TLIST_LABEL_MAX];
		char label[TLIST_LABEL_MAX];

		prop_print_value(val, sizeof(val), prop);
		snprintf(label, sizeof(label), "%s = %s", prop->key, val);
		tlist_insert_item(tl, NULL, label, prop);
	}
	pthread_mutex_unlock(&ob->props_lock);
	tlist_restore_selections(tl);
}

void
object_edit(void *p)
{
	struct object *ob = p;
	struct window *win;
	struct box *bo;
	struct button *bu;
	char *obname;

	obname = object_name(ob);
	if ((win = window_new("object-edit-%s", obname)) == NULL) {
		goto out;
	}
	window_set_caption(win, "%s object", ob->name);
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 0);
	window_set_closure(win, WINDOW_DETACH);

	label_new(win, "Name: \"%s\"", ob->name);
	label_new(win, "Type: \"%s\"", ob->type);
	label_new(win, "Flags : 0x%02x", ob->flags);

	label_polled_new(win, NULL, "Parent: %p", &ob->parent);
	label_polled_new(win, NULL, "Gfx: %p", &ob->art);
	label_polled_new(win, &ob->lock, "Pos: %p", &ob->pos);

	bo = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	box_set_padding(bo, 0);
	{
		bu = button_new(bo, "Load");
		event_new(bu, "button-pushed", object_invoke_op, "%p, %i", ob,
		    LOAD_OP);
		bu = button_new(bo, "Save");
		event_new(bu, "button-pushed", object_invoke_op, "%p, %i", ob,
		    SAVE_OP);
		bu = button_new(bo, "Edit");
		event_new(bu, "button-pushed", object_invoke_op, "%p, %i", ob,
		    EDIT_OP);
	}

	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	box_set_padding(bo, 0);
	{
		struct tlist *tl;

		tl = tlist_new(bo, TLIST_POLL);
		tlist_prescale(tl, "XXXXXXXXXXXX", 2);
		event_new(tl, "tlist-poll", poll_props, "%p", ob);
	}

	window_show(win);

	if (ob->ops->edit != NULL) {
		ob->ops->edit(ob);
	}
out:
	free(obname);
	return;
}
