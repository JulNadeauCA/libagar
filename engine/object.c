/*	$Csoft: object.c,v 1.139 2003/06/29 11:33:41 vedge Exp $	*/

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
#include <engine/config.h>
#include <engine/map.h>
#include <engine/input.h>
#include <engine/view.h>
#include <engine/rootmap.h>
#include <engine/typesw.h>
#include <engine/mkpath.h>
#ifdef EDITION
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#endif
#include <engine/loader/den.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

const struct version object_ver = {
	"agar object",
	5, 0
};

const struct object_ops object_ops = {
	NULL,	/* init */
	NULL,	/* reinit */
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

int	object_debug = DEBUG_STATE|DEBUG_POSITION|DEBUG_SUBMAPS|DEBUG_CONTROL;
#define engine_debug object_debug
#endif

/* Allocate, initialize and attach a generic object. */
struct object *
object_new(void *parent, const char *name)
{
	struct object *ob;

	ob = Malloc(sizeof(struct object));
	object_init(ob, "object", name, NULL);

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
	char *c;

	strlcpy(ob->type, type, sizeof(ob->type));
	strlcpy(ob->name, name, sizeof(ob->name));

	/* Prevent the pathname separator from creating ambiguities. */
	for (c = ob->name; *c != '\0'; c++) {
		if (*c == '/')
			*c = '_';
	}

	ob->save_pfx = "/world";
	ob->ops = (opsp != NULL) ? opsp : &object_ops;
	ob->parent = NULL;
	ob->flags = 0;
	ob->gfx = NULL;
	ob->audio = NULL;
	ob->pos = NULL;
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->childs);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	pthread_mutex_init(&ob->lock, &recursive_mutexattr);
	pthread_mutex_init(&ob->props_lock, &recursive_mutexattr);
	pthread_mutex_init(&ob->events_lock, &recursive_mutexattr);
}

/*
 * Reinitialize the state of an object and ensure that no dependency remains,
 * except for permanent dependencies (which are only released on destroy).
 *
 * This is useful to reset the state of an object before a load operation
 * (freeing the map(3) nodes, for instances).
 */
void
object_reinit(void *p)
{
	struct object *ob = p;
	struct object_dep *dep, *ndep;

	if (ob->ops->reinit != NULL)
		ob->ops->reinit(ob);

	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);

		if (dep->count == OBJECT_DEP_MAX)
			continue;

		/* Ensure the reinit operation did its job. */
		if (dep->count > 0) {
			fatal("%s still depends on %s", ob->name,
			    dep->obj->name);
		}
		TAILQ_REMOVE(&ob->deps, dep, deps);
		free(dep);
	}
}

/* Recursive function to construct absolute object names. */
static int
object_name_search(const void *obj, char *path, size_t path_len)
{
	const struct object *ob = obj;
	size_t name_len, cur_len;
	int rv = 0;

	cur_len = strlen(path)+1;
	name_len = strlen(ob->name)+1;
	
	if (sizeof("/")+name_len+sizeof("/")+cur_len >= path_len) {
		error_set(_("The path exceeds >= %lu bytes"),
		    (unsigned long)path_len);
		return (-1);
	}
	
	/* Prepend / and the object name. */
	memmove(&path[name_len], path, cur_len);    /* Move the NUL as well */
	path[0] = '/';
	memcpy(&path[1], ob->name, name_len-1);	    /* Omit the NUL */

	if (ob->parent != world && ob->parent != NULL)
		rv = object_name_search(ob->parent, path, path_len);

	return (rv);
}

/* Copy the absolute pathname of an object to a fixed-size buffer. */
int
object_copy_name(const void *obj, char *path, size_t path_len)
{
	const struct object *ob = obj;
	int rv = 0;

	path[0] = '/';
	path[1] = '\0';
	strlcat(path, ob->name, path_len);

	lock_linkage();
	if (ob->parent != world && ob->parent != NULL) {
		rv = object_name_search(ob->parent, path, path_len);
	}
	unlock_linkage();
	return (rv);
}

/*
 * Return the root of the object tree in which an object resides.
 * The linkage must be locked.
 */
void *
object_root(void *obj)
{
	struct object *ob = obj;

	while (ob != NULL) {
		if (ob->parent == NULL) {
			return (ob);
		}
		ob = ob->parent;
	}
	fatal("no root?");
}

/*
 * Search an object and its children for a dependency upon robj.
 * The linkage must be locked.
 */
static int
object_used_search(const void *p, const void *robj)
{
	const struct object *ob = p, *cob;
	struct object_dep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == robj)
			return (1);
	}
	TAILQ_FOREACH(cob, &ob->childs, cobjs) {
		if (object_used_search(cob, robj))
			return (1);
	}
	return (0);
}

/*
 * Return the number of references to an object by searching the dependency
 * tables of other objects sharing the same root.
 */
int
object_used(void *obj)
{
	struct object *root;
	int used;

	lock_linkage();
	root = object_root(obj);
	used = object_used_search(root, obj);
	unlock_linkage();

	if (used) {
		dprintf("%s: in use\n", OBJECT(obj)->name);
		error_set(_("The `%s' object is in use."), OBJECT(obj)->name);
	}
	return (used);
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
object_find_child(const struct object *parent, const char *name)
{
	char node_name[OBJECT_PATH_MAX];
	void *rv;
	char *s;
	struct object *child;

	strlcpy(node_name, name, sizeof(node_name));

	if ((s = strchr(node_name, '/')) != NULL) {
		*s = '\0';
	}
	TAILQ_FOREACH(child, &parent->childs, cobjs) {
		if (strcmp(child->name, node_name) != 0)
			continue;

		if ((s = strchr(name, '/')) != NULL) {
			rv = object_find_child(child, s+1);
			if (rv != NULL)
				return (rv);
		}
		return (child);
	}
	return (NULL);
}

/* Search for the named object (absolute path). */
void *
object_find(const char *name)
{
	void *rv;

	if (name[0] != '/')
		fatal("not an absolute path");

	lock_linkage();
	rv = object_find_child(world, name+1);
	unlock_linkage();

	if (rv == NULL) {
		error_set(_("The object `%s' does not exist."), name);
	}
	return (rv);
}

/* Detach and free child objects. */
int
object_free_childs(struct object *pob)
{
	struct object *cob, *ncob;

	pthread_mutex_lock(&pob->lock);
	TAILQ_FOREACH(cob, &pob->childs, cobjs) {
		if (object_used(cob))
			goto fail;
	}
	for (cob = TAILQ_FIRST(&pob->childs);
	     cob != TAILQ_END(&pob->childs);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		debug(DEBUG_GC, "%s: freeing %s\n", pob->name, cob->name);
		object_destroy(cob);
		free(cob);
	}
	TAILQ_INIT(&pob->childs);
	pthread_mutex_unlock(&pob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&pob->lock);
	return (-1);
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

/* Release the resources allocated by an object and its children. */
int
object_destroy(void *p)
{
	struct object *ob = p;
	struct object_dep *dep, *ndep;

	if (object_used(ob))
		return (-1);

	if (object_free_childs(ob) == -1)
		return (-1);

	if (ob->ops->reinit != NULL) {
		ob->ops->reinit(ob);
	}
	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);
		free(dep);
	}

	if (ob->ops->destroy != NULL)
		ob->ops->destroy(ob);

	if (ob->gfx != NULL) {
		gfx_unused(ob->gfx);
		ob->gfx = NULL;
	}
	if (ob->audio != NULL) {
		audio_unused(ob->audio);
		ob->audio = NULL;
	}

	object_free_props(ob);
	object_free_events(ob);

	pthread_mutex_destroy(&ob->lock);
	pthread_mutex_destroy(&ob->events_lock);
	pthread_mutex_destroy(&ob->props_lock);
	return (0);
}

/* Copy the full pathname to an object's data file to a fixed-size buffer. */
int
object_copy_filename(const void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN];
	char obj_name[OBJECT_PATH_MAX];
	const struct object *ob = p;
	struct stat sta;
	char *dir, *last;

	prop_copy_string(config, "load-path", load_path, sizeof(load_path));
	object_copy_name(ob, obj_name, sizeof(obj_name));

	for (dir = strtok_r(load_path, ":", &last);
	     dir != NULL;
	     dir = strtok_r(NULL, ":", &last)) {
	     	strlcpy(path, dir, path_len);
		if (ob->save_pfx != NULL) {
			strlcat(path, ob->save_pfx, path_len);
		}
		strlcat(path, obj_name, path_len);
		strlcat(path, "/", path_len);
		strlcat(path, ob->name, path_len);
		strlcat(path, ".", path_len);
		strlcat(path, ob->type, path_len);

		if (stat(path, &sta) == 0)
			return (0);
	}
	error_set(_("The %s/%s.%s file is not in load-path."), obj_name,
	    ob->name, ob->type);
	return (-1);
}

/* Load the state of an object and its descendants. */
int
object_load(void *p)
{
	char path[MAXPATHLEN];
	struct object *ob = p;
	struct netbuf *buf;
	char *gfx, *audio;
	Uint32 count, i;
	int ti;

	if (ob->flags & OBJECT_NON_PERSISTENT) {
		error_set(_("The `%s' object is non-persistent."), ob->name);
		return (-1);
	}

	if (object_copy_filename(ob, path, sizeof(path)) == -1)
		return (-1);

	debug(DEBUG_STATE, "%s: %s\n", ob->name, path);

	if ((buf = netbuf_open(path, "rb", NETBUF_BIG_ENDIAN)) == NULL) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	if (version_read(buf, &object_ver, NULL) == -1) {
		netbuf_close(buf);
		return (-1);
	}

	lock_linkage();

	/* Reinitialize the state of the object; clear the dependencies. */
	object_reinit(ob);

	/* Read the object flags. */
	ob->flags = (int)read_uint32(buf);
	if (ob->flags & OBJECT_NON_PERSISTENT) {
		error_set(_("The `%s' save has inconsistent flags."), ob->name);
		return (-1);
	}

	/* Decode and resolve the saved dependencies. */
	/* XXX too early */
	count = read_uint32(buf);
	for (i = 0; i < count; i++) {
		char dep_name[OBJECT_PATH_MAX];
		struct object_dep *dep;
		struct object *dep_obj;

		copy_string(dep_name, buf, sizeof(dep_name));
		if ((dep_obj = object_find(dep_name)) == NULL)
			goto fail;

		dep = Malloc(sizeof(struct object_dep));
		dep->obj = dep_obj;
		dep->count = 0;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
		dprintf("%s: depends on %s (%p)\n", ob->name, dep_name,
		    dep->obj);
	}

	/* Decode the generic properties. */
	if ((ob->flags & OBJECT_RELOAD_PROPS) == 0) {
		object_free_props(ob);
	}
	if (prop_load(ob, buf) == -1)
		goto fail;

	/* Decode and restore the position. */
	pthread_mutex_lock(&ob->lock);
	if (read_uint32(buf) > 0 &&
	    object_load_position(ob, buf) == -1) {
		pthread_mutex_unlock(&ob->lock);
		goto fail;
	}
	pthread_mutex_unlock(&ob->lock);

	/* Decode and fetch the associated gfx/audio packages. */
	if ((gfx = read_string(buf)) != NULL) {
		dprintf("%s: gfx: %s\n", ob->name, gfx);
		if (gfx_fetch(ob, gfx) == -1) {
			goto fail;
		}
		free(gfx);
	}
	if ((audio = read_string(buf)) != NULL) {
		dprintf("%s: audio: %s\n", ob->name, audio);
		if (audio_fetch(ob, audio) == -1) {
			goto fail;
		}
		free(audio);
	}

	/*
	 * Load the child objects.
	 *
	 * If an saved object matches an attached object's name and type,
	 * invoke reinit/load on it. Otherwise, allocate and attach a new
	 * object from scratch using the type switch.
	 *
	 * Try to destroy the attached objects which do not have saved
	 * states, and are not currently in use.
	 *
	 * XXX ensure that there is no duplicate names.
	 */
	count = read_uint32(buf);
	for (i = 0; i < count; i++) {
		char cname[OBJECT_NAME_MAX];
		char ctype[OBJECT_TYPE_MAX];
		struct object *eob, *child;

		copy_string(cname, buf, sizeof(cname));
		copy_string(ctype, buf, sizeof(ctype));

		OBJECT_FOREACH_CHILD(eob, ob, object) {
			if (strcmp(eob->name, cname) == 0)
				break;
		}
		if (eob != NULL) {
			/* XXX free the existing object or ignore */
			if (strcmp(eob->type, ctype) != 0) {
				fatal("existing object of different type");
			}
			/* XXX ignore */
			if (eob->flags & OBJECT_NON_PERSISTENT) {
				fatal("existing non-persistent object");
			}
			dprintf("%s: reinit %s (existing)\n", ob->name,
			    eob->name);
			object_reinit(eob);

			if (object_load(eob) == -1)
				goto fail;
		} else {
			dprintf("%s: alloc %s (new %s)\n", ob->name,
			    cname, ctype);
		 	for (ti = 0; ti < ntypesw; ti++) {
				if (strcmp(typesw[ti].type, ctype) == 0)
					break;
			}
			if (ti == ntypesw) {
				error_set(_("Unknown object type: `%s'"),
				    ctype);
				goto fail;
			}
			dprintf("%s: child `%s' (%s, %lu bytes)\n", ob->name,
			    cname, typesw[ti].desc,
			    (unsigned long)typesw[ti].size);

			child = Malloc(typesw[ti].size);
			if (typesw[ti].ops->init != NULL) {
				typesw[ti].ops->init(child, cname);
			} else {
				object_init(child, ctype, cname,
				    typesw[ti].ops);
			}

			object_attach(ob, child);

			if (object_load(child) == -1)
				goto fail;
		}
	}

	if (ob->ops->load != NULL &&
	    ob->ops->load(ob, buf) == -1) {
		goto fail;
	}

	unlock_linkage();
	netbuf_close(buf);
	return (0);
fail:
	object_reinit(ob);
	unlock_linkage();
	netbuf_close(buf);
	return (-1);
}

/* Recursively save the state of an object and its descendents. */
int
object_save(void *p)
{
	char save_path[MAXPATHLEN];
	char save_dir[MAXPATHLEN];
	char save_file[MAXPATHLEN];
	char obj_name[OBJECT_PATH_MAX];
	struct object *ob = p;
	struct stat sta;
	struct netbuf *buf;
	struct object *child;
	off_t count_offs;
	Uint32 count;
	struct object_dep *dep;
	
	if (ob->flags & OBJECT_NON_PERSISTENT) {
		error_set(_("The `%s' object is non-persistent."), ob->name);
		return (-1);
	}

	object_copy_name(ob, obj_name, sizeof(obj_name));
	
	/* Create the save directory. */
	prop_copy_string(config, "save-path", save_path, sizeof(save_path));
	strlcpy(save_dir, save_path, sizeof(save_dir));
	if (ob->save_pfx != NULL) {
		strlcat(save_dir, ob->save_pfx, sizeof(save_dir));
	}
	strlcat(save_dir, obj_name, sizeof(save_dir));
	if (stat(save_dir, &sta) == -1 &&
	    mkpath(save_dir, 0700, 0700) == -1) {
		error_set("mkpath %s: %s", save_dir, strerror(errno));
		return (-1);
	}

	/* Open the data file. */
	strlcpy(save_file, save_dir, sizeof(save_file));
	strlcat(save_file, "/", sizeof(save_file));
	strlcat(save_file, ob->name, sizeof(save_file));
	strlcat(save_file, ".", sizeof(save_file));
	strlcat(save_file, ob->type, sizeof(save_file));
	debug(DEBUG_STATE, "saving `%s' to `%s'\n", ob->name, save_file);
	if ((buf = netbuf_open(save_file, "wb", NETBUF_BIG_ENDIAN)) == NULL)
		return (-1);

	version_write(buf, &object_ver);

	lock_linkage();
	
	/* Encode the saveable object flags. */
	write_uint32(buf, (Uint32)(ob->flags & OBJECT_SAVED_FLAGS));

	/* Encode the object dependencies. */
	count_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	for (dep = TAILQ_FIRST(&ob->deps), count = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), count++) {
		char dep_name[OBJECT_PATH_MAX];
		
		object_copy_name(dep->obj, dep_name, sizeof(dep_name));
		dprintf("saving dep: `%s'\n", dep_name);
		write_string(buf, dep_name);
	}
	pwrite_uint32(buf, count, count_offs);

	/* Encode the generic properties. */
	if (prop_save(ob, buf) == -1)
		goto fail;

	/* Encode the position. */
	pthread_mutex_lock(&ob->lock);
	if (ob->pos != NULL) {
		write_uint32(buf, 1);
		object_save_position(ob, buf);
	} else {
		write_uint32(buf, 0);
	}
	pthread_mutex_unlock(&ob->lock);

	/* Encode the media references. */
	write_string(buf, ob->gfx != NULL ? ob->gfx->name : NULL);
	write_string(buf, ob->audio != NULL ? ob->audio->name : NULL);

	/* Save the child objects. */
	count_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	count = 0;
	TAILQ_FOREACH(child, &ob->childs, cobjs) {
		if (child->flags & OBJECT_NON_PERSISTENT) {
			dprintf("skipping non persistent: %s\n", child->name);
			continue;
		}
		write_string(buf, child->name);
		write_string(buf, child->type);
		if (object_save(child) == -1) {
			goto fail;
		}
		count++;
	}
	pwrite_uint32(buf, count, count_offs);

	/* Save the object derivate data. */
	if (ob->ops->save != NULL &&
	    ob->ops->save(ob, buf) == -1) {
		goto fail;
	}

	unlock_linkage();
	netbuf_flush(buf);
	netbuf_close(buf);
	return (0);
fail:
	unlock_linkage();
	netbuf_close(buf);
	return (-1);
}

/* Control an object's position with an input device. */
int
object_control(void *p, const char *inname)
{
	struct object *ob = p;
	struct input *in;

	if ((in = input_find(inname)) == NULL) {
		error_set(_("There is no input device named `%s'."), inname);
		return (-1);
	}

	pthread_mutex_lock(&ob->lock);
	if (ob->pos == NULL) {
		error_set(_("There is no position to control."));
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

/* Project the current submap onto the level map. */
static void
object_project_submap(struct object_position *pos)
{
	struct map *sm = pos->submap;
	struct map *dm = pos->map;
	int sx, sy, dx, dy;

	dprintf("[%d,%d,%d]+%dx%d\n", pos->x, pos->y, pos->layer, sm->mapw,
	    sm->maph);

	for (sy = 0, dy = pos->y;
	     sy < sm->maph && dy < pos->y+dm->maph;
	     sy++, dy++) {
		for (sx = 0, dx = pos->x;
		     sx < sm->mapw && dx < pos->x+dm->mapw;
		     sx++, dx++) {
			node_copy(sm, &sm->map[sy][sx], -1,
			    dm, &dm->map[dy][dx], pos->layer);
		}
	}
}

/* Clear the copy of the current submap from the level map. */
static void
object_unproject_submap(struct object_position *pos)
{
	int x, y;
	
	dprintf("[%d,%d,%d]+%dx%d\n", pos->x, pos->y, pos->layer,
	    pos->submap->mapw, pos->submap->maph);

	for (y = pos->y; y < pos->y+pos->submap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->submap->mapw; x++) {
			node_clear(pos->map, &pos->map->map[y][x], pos->layer);
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
		error_set(_("There is no submap named `%s'."), name);
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
object_save_position(const void *p, struct netbuf *buf)
{
	const struct object *ob = p;
	const struct object_position *pos = ob->pos;

	write_uint32(buf, (Uint32)pos->x);
	write_uint32(buf, (Uint32)pos->y);
	write_uint8(buf, (Uint8)pos->layer);
	write_uint8(buf, (Uint8)pos->center);
	write_string(buf, (pos->submap != NULL) ?
	    OBJECT(pos->submap)->name : NULL);
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
	int x, y, layer, center;
	struct object *ob = p;
	struct map *m = ob->parent;
	char submap_id[OBJECT_PATH_MAX];
	char input_id[OBJECT_PATH_MAX];

	pthread_mutex_lock(&ob->lock);
	pthread_mutex_lock(&m->lock);

	x = (int)read_uint32(buf);
	y = (int)read_uint32(buf);
	layer = (int)read_uint8(buf);
	center = (int)read_uint8(buf);

	copy_string(submap_id, buf, sizeof(submap_id));
	copy_string(input_id, buf, sizeof(input_id));

	debug(DEBUG_STATE, "%s: at [%d,%d,%d]%s\n", ob->name, x, y, layer,
	    center ? ", centered" : "");

	if (x < 0 || x >= m->mapw ||
	    y < 0 || y >= m->maph ||
	    layer < 0 || layer >= m->nlayers) {
		error_set(_("Bad coordinates: %d,%d,%d"), x, y, layer);
		goto fail;
	}
	if (submap_id != NULL &&
	    object_set_submap(ob, submap_id) == -1) {
		goto fail;
	}
	object_set_position(ob, m, x, y, layer);

	if (input_id != NULL) {
		if (object_control(ob, input_id) == -1)
			debug(DEBUG_STATE, "%s: %s\n", ob->name, error_get());
	} else {
		debug(DEBUG_STATE, "%s: no controller\n", ob->name);
	}

	pthread_mutex_unlock(&m->lock);
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&m->lock);
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Override an object's type; thread unsafe. */
void
object_set_type(void *p, const char *type)
{
	struct object *ob = p;

	strlcpy(ob->type, type, sizeof(ob->type));
}

/* Override an object's name; thread unsafe. */
void
object_set_name(void *p, const char *name)
{
	struct object *ob = p;
	char *c;

	strlcpy(ob->name, name, sizeof(ob->name));

	for (c = &ob->name[0]; *c != '\0'; c++) {
		if (*c == '/')			/* Pathname separator */
			*c = '_';
	}
}

/* Override an object's ops; thread unsafe. */
void
object_set_ops(void *p, const void *ops)
{
	OBJECT(p)->ops = ops;
}

/* Add a new dependency or increment the reference count on one. */
struct object_dep *
object_add_dep(void *p, void *depobj)
{
	struct object *ob = p;
	struct object_dep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == depobj)
			break;
	}
	if (dep != NULL) {
		debug(DEBUG_DEPS, "%s: [%s/%u]\n", ob->name,
		    OBJECT(depobj)->name, dep->count);
		if (++dep->count > OBJECT_DEP_MAX) {
			fprintf(stderr, "%s: wiring %s dep (too many refs)\n",
			    ob->name, OBJECT(depobj)->name);
			dep->count = OBJECT_DEP_MAX;
		}
	} else {
		debug(DEBUG_DEPS, "%s: +[%s]\n", ob->name,
		    OBJECT(depobj)->name);
		dep = Malloc(sizeof(struct object_dep));
		dep->obj = depobj;
		dep->count = 1;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
	}
	return (dep);
}

/* Return the dependency at the given index (ie. for load routines). */
struct object *
object_find_dep(const void *p, Uint32 ind)
{
	const struct object *ob = p;
	struct object_dep *dep;
	Uint32 i;

	for (dep = TAILQ_FIRST(&ob->deps), i = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), i++) {
		if (i == ind)
			break;
	}
	return (dep != NULL ? dep->obj : NULL);
}

/* Return the index of a dependency (ie. for save routines). */
Uint32
object_dep_index(const void *p, const struct object *depobj)
{
	const struct object *ob = p;
	struct object_dep *dep;
	Uint32 i;

	for (dep = TAILQ_FIRST(&ob->deps), i = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), i++) {
		if (dep->obj == depobj)
			return (i);
	}
	fatal("%s: no such dep", depobj->name);
	return (0);
}

/*
 * Decrement the reference count on a dependency and remove it if the
 * reference count reaches 0.
 */
void
object_del_dep(void *p, const void *depobj)
{
	struct object *ob = p;
	struct object_dep *dep;
	
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == depobj)
			break;
	}
	if (dep == NULL)
		fatal("%s: no such dep", OBJECT(depobj)->name);

	if (dep->count == OBJECT_DEP_MAX)			/* Wired */
		return;

	if ((dep->count-1) == 0) {
		debug(DEBUG_DEPS, "%s: -[%s]\n", ob->name,
		    OBJECT(depobj)->name);
		TAILQ_REMOVE(&ob->deps, dep, deps);
		free(dep);
	} else if ((dep->count-1) < 0) {
		fatal("neg ref count");
	} else {
		debug(DEBUG_DEPS, "%s: [%s/%u]\n", ob->name,
		    OBJECT(depobj)->name, dep->count);
		dep->count--;
	}
}

#ifdef EDITION
/* Update the dependencies display. */
static void
poll_deps(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct object_dep *dep;

	tlist_clear_items(tl);
	lock_linkage();
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		char label[TLIST_LABEL_MAX];

		if (dep->count == OBJECT_DEP_MAX) {
			snprintf(label, sizeof(label), "%s (perm)",
			    dep->obj->name);
		} else {
			snprintf(label, sizeof(label), "%s (%u)",
			    dep->obj->name, dep->count);
		}
		tlist_insert_item(tl, OBJECT_ICON(dep->obj), label, dep);
	}
	unlock_linkage();
	tlist_restore_selections(tl);
}

/* Search subdirectories for .den files containing a given hint. */
static void
object_scan_dens(const struct object *ob, const char *hint, struct combo *com,
    const char *spath)
{
	DIR *di;
	struct dirent *dent;

	if ((di = opendir(".")) == NULL) {
		text_msg(MSG_ERROR, "opendir: %s\n", strerror(errno));
		return;
	}
	while ((dent = readdir(di)) != NULL) {
		char path[MAXPATHLEN];
		struct stat sta;
		char *ext;

		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		if (stat(dent->d_name, &sta) == -1) {
			dprintf("%s: %s\n", dent->d_name, strerror(errno));
			continue;
		}

		strlcpy(path, spath, sizeof(path));
		strlcat(path, "/", sizeof(path));
		strlcat(path, dent->d_name, sizeof(path));

		if ((sta.st_mode & S_IFREG) &&
		    (ext = strrchr(dent->d_name, '.')) != NULL &&
		    strcasecmp(ext, ".den") == 0) {
			struct den *den;

			if ((den = den_open(dent->d_name, DEN_READ)) != NULL) {
				if (strcmp(den->hint, hint) == 0 &&
				    (ext = strrchr(path, '.')) != NULL) {
					*ext = '\0';
					tlist_insert_item(com->list, NULL, path,
					    NULL);
				}
				den_close(den);
			}
		} else if (sta.st_mode & S_IFDIR) {
			if (chdir(dent->d_name) == 0) {
				object_scan_dens(ob, hint, com, path);
				if (chdir("..") == -1) {
					text_msg(MSG_ERROR, "chdir ..: %s\n",
					    strerror(errno));
					closedir(di);
					return;
				}
			}
		}
	}
	closedir(di);
}

static void
select_gfx(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	char *text = argv[2].s;

	if (text[0] == '\0' && ob->gfx != NULL) {
		gfx_unused(ob->gfx);
		ob->gfx = NULL;
	} else {
		if (gfx_fetch(ob, text) == -1)
			text_msg(MSG_ERROR, "%s", error_get());
	}
}

static void
select_audio(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	char *text = argv[2].s;

	if (text[0] == '\0' && ob->audio != NULL) {
		audio_unused(ob->audio);
		ob->audio = NULL;
	} else {
		if (audio_fetch(ob, text) == -1)
			text_msg(MSG_ERROR, "%s", error_get());
	}
}

struct window *
object_edit(void *p)
{
	char obj_name[OBJECT_PATH_MAX];
	struct object *ob = p;
	struct window *win;
	struct box *bo;

	win = window_new(NULL);
	window_set_caption(win, _("%s object"), ob->name);
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 0);
	window_set_closure(win, WINDOW_DETACH);

	/* XXX path subject to change */
	object_copy_name(ob, obj_name, sizeof(obj_name));
	label_new(win, _("Name: %s (%s)"), ob->name, obj_name);
	label_new(win, _("Type: %s"), ob->type);
	label_new(win, _("Flags : 0x%02x"), ob->flags);

	label_polled_new(win, NULL, _("Parent: %[obj]"), &ob->parent);
	label_polled_new(win, &ob->lock, "Pos: %p", &ob->pos);

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		char den_path[MAXPATHLEN];
		struct combo *gfx_com, *aud_com;
		char *dir, *last;

		gfx_com = combo_new(bo, _("Graphics: "));
		aud_com = combo_new(bo, _("Audio: "));

		textbox_prescale(gfx_com->tbox, "XXXXXXXXXXXXXX");

		event_new(gfx_com, "combo-selected", select_gfx, "%p", ob);
		event_new(aud_com, "combo-selected", select_audio, "%p", ob);

		if (ob->gfx != NULL)
			textbox_printf(gfx_com->tbox, "%s", ob->gfx->name);
		if (ob->audio != NULL)
			textbox_printf(aud_com->tbox, "%s", ob->audio->name);

		prop_copy_string(config, "den-path", den_path,
		    sizeof(den_path));

		for (dir = strtok_r(den_path, ":", &last);
		     dir != NULL;
		     dir = strtok_r(NULL, ":", &last)) {
			char cwd[MAXPATHLEN];

			if (getcwd(cwd, sizeof(cwd)) == NULL) {
				text_msg(MSG_ERROR, "getcwd: %s",
				    strerror(errno));
				continue;
			}

			if (chdir(dir) == 0) {
				object_scan_dens(ob, "gfx", gfx_com, "");
				object_scan_dens(ob, "audio", aud_com, "");
				if (chdir(cwd) == -1) {
					text_msg(MSG_ERROR, "chdir %s: %s",
					    cwd, strerror(errno));
					continue;
				}
			}
		}
	}

	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	box_set_padding(bo, 2);
	{
		struct tlist *tl;

		label_new(bo, _("Dependencies:"));
		tl = tlist_new(bo, TLIST_POLL);
		tlist_prescale(tl, "XXXXXXXXXXXX", 4);
		event_new(tl, "tlist-poll", poll_deps, "%p", ob);
	}
	return (win);
}

#endif /* EDITION */
