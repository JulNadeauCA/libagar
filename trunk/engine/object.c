/*	$Csoft: object.c,v 1.158 2004/02/20 04:20:33 vedge Exp $	*/

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
#include <engine/widget/textbox.h>
#endif

#include <engine/loader/den.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
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
#define DEBUG_DEPS	0x004
#define DEBUG_CONTROL	0x010
#define DEBUG_LINKAGE	0x020
#define DEBUG_GC	0x040
#define DEBUG_PAGING	0x080

int	object_debug = DEBUG_STATE|DEBUG_CONTROL;
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
	ob->gfx_name = NULL;
	ob->gfx_used = 0;
	ob->audio = NULL;
	ob->audio_name = NULL;
	ob->audio_used = 0;
	ob->data_used = 0;
	ob->pos = NULL;
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->children);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	pthread_mutex_init(&ob->lock, &recursive_mutexattr);
}

/*
 * Reinitialize the state of an object (eg. free map nodes), but preserve
 * the dependencies (which are assumed to then have a reference count of 0).
 */
void
object_free_data(void *p)
{
	struct object *ob = p;

	if (ob->flags & OBJECT_DATA_RESIDENT) {
		dprintf("%s: was resident\n", ob->name);
		if (ob->ops->reinit != NULL) {
			ob->flags |= OBJECT_PRESERVE_DEPS;
			ob->ops->reinit(ob);
			ob->flags &= ~(OBJECT_PRESERVE_DEPS);
		}
		ob->flags &= ~(OBJECT_DATA_RESIDENT);
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
		error_set(_("The path exceeds >= %lu bytes."),
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

/*
 * Copy the absolute pathname of an object to a fixed-size buffer.
 * The buffer size must be >2 bytes.
 */
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
	return (NULL);
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
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
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
		error_set(_("The `%s' object is in use."), OBJECT(obj)->name);
	}
	return (used);
}

/* Move an object to a different parent. */
void
object_move(void *childp, void *newparentp)
{
	struct object *child = childp;
	struct object *oparent = child->parent;
	struct object *nparent = newparentp;

	lock_linkage();

	TAILQ_REMOVE(&oparent->children, child, cobjs);
	child->parent = NULL;
	event_post(oparent, child, "detached", NULL);

	TAILQ_INSERT_TAIL(&nparent->children, child, cobjs);
	child->parent = nparent;
	event_post(nparent, child, "attached", NULL);
	event_post(oparent, child, "moved", "%p", nparent);

	debug(DEBUG_LINKAGE, "%s: %s -> %s\n", child->name, oparent->name,
	    nparent->name);

	unlock_linkage();
}

/* Attach a child object to some parent object. */
void
object_attach(void *parentp, void *childp)
{
	struct object *parent = parentp;
	struct object *child = childp;

	lock_linkage();
	TAILQ_INSERT_TAIL(&parent->children, child, cobjs);
	child->parent = parent;
	event_post(parent, child, "attached", NULL);
	debug(DEBUG_LINKAGE, "%s: parent = %s\n", child->name, parent->name);
	unlock_linkage();
}

/* Detach a child object from its parent. */
void
object_detach(void *childp)
{
	struct object *child = childp;
	struct object *parent = child->parent;

	lock_linkage();
	TAILQ_REMOVE(&parent->children, child, cobjs);
	child->parent = NULL;
	event_post(child, parent, "detached", NULL);
	debug(DEBUG_LINKAGE, "%s: detached from %s\n", child->name,
	    parent->name);
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
	TAILQ_FOREACH(child, &parent->children, cobjs) {
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

#ifdef DEBUG
	if (name[0] != '/')
		fatal("not an absolute path");
#endif
	lock_linkage();
	rv = object_find_child(world, name+1);
	unlock_linkage();

	if (rv == NULL) {
		error_set(_("The object `%s' does not exist."), name);
	}
	return (rv);
}

/* Clear the dependency table. */
void
object_free_deps(struct object *ob)
{
	struct object_dep *dep, *ndep;

	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);
		free(dep);
	}
	TAILQ_INIT(&ob->deps);
}

/* Detach and free child objects. */
int
object_free_children(struct object *pob)
{
	struct object *cob, *ncob;

	pthread_mutex_lock(&pob->lock);
	TAILQ_FOREACH(cob, &pob->children, cobjs) {
		if (object_used(cob))
			goto fail;
	}
	for (cob = TAILQ_FIRST(&pob->children);
	     cob != TAILQ_END(&pob->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		debug(DEBUG_GC, "%s: freeing %s\n", pob->name, cob->name);
		object_destroy(cob);
		if ((cob->flags & OBJECT_STATIC) == 0)
			free(cob);
	}
	TAILQ_INIT(&pob->children);
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

	pthread_mutex_lock(&ob->lock);
	for (prop = TAILQ_FIRST(&ob->props);
	     prop != TAILQ_END(&ob->props);
	     prop = nextprop) {
		nextprop = TAILQ_NEXT(prop, props);
		prop_destroy(prop);
		free(prop);
	}
	TAILQ_INIT(&ob->props);
	pthread_mutex_unlock(&ob->lock);
}

/* Clear an object's event handler list. */
void
object_free_events(struct object *ob)
{
	struct event *eev, *nexteev;

	pthread_mutex_lock(&ob->lock);
	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = nexteev) {
		nexteev = TAILQ_NEXT(eev, events);
		free(eev);
	}
	TAILQ_INIT(&ob->events);
	pthread_mutex_unlock(&ob->lock);
}

/* Release the resources allocated by an object and its children. */
/* XXX XXX first verify that none of the children are in use! */
int
object_destroy(void *p)
{
	struct object *ob = p;
	struct object_dep *dep, *ndep;

	if (object_used(ob))
		return (-1);

	if (object_free_children(ob) == -1)
		return (-1);
	
	if (ob->pos != NULL)
		position_unset(ob);

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
	}
	if (ob->audio != NULL) {
		audio_unused(ob->audio);
	}
	Free(ob->gfx_name);
	Free(ob->audio_name);

	object_free_props(ob);
	object_free_events(ob);

	pthread_mutex_destroy(&ob->lock);
	return (0);
}

/* Copy the full pathname to an object's data file to a fixed-size buffer. */
/* XXX modifies buffer even on failure */
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
	error_set(_("The %s%s/%s.%s file is not in load-path."),
	    ob->save_pfx != NULL ? ob->save_pfx : "",
	    obj_name, ob->name, ob->type);
	return (-1);
}

/* Copy the full pathname of an object's data dir to a fixed-size buffer. */
int
object_copy_dirname(const void *p, char *path, size_t path_len)
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
		char tmp_path[MAXPATHLEN];

	     	strlcpy(tmp_path, dir, sizeof(tmp_path));
		if (ob->save_pfx != NULL) {
			strlcat(tmp_path, ob->save_pfx, sizeof(tmp_path));
		}
		strlcat(tmp_path, obj_name, sizeof(tmp_path));
		if (stat(tmp_path, &sta) == 0) {
			strlcpy(path, tmp_path, path_len);
			return (0);
		}
	}
	error_set(_("The %s directory is not in load-path."), obj_name);
	return (-1);
}

/* Page in or increment reference counts on media/data. */
int
object_page_in(void *p, enum object_page_item item)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	switch (item) {
	case OBJECT_GFX:
		debug(DEBUG_PAGING, "gfx of %s: %s; used = %u++\n", ob->name,
		    ob->gfx_name, ob->gfx_used);
		if (ob->gfx_name == NULL) {
			error_set(_("The `%s' object contains no graphics."),
			    ob->name);
			goto fail;
		}
		if (gfx_fetch(ob) == -1) {
			Free(ob->gfx_name);
			goto fail;
		}
		if (++ob->gfx_used > OBJECT_DEP_MAX) {
			ob->gfx_used = OBJECT_DEP_MAX;
		}
		break;
	case OBJECT_AUDIO:
		debug(DEBUG_PAGING, "audio of %s: %s; used = %u++\n", ob->name,
		    ob->audio_name, ob->audio_used);
		if (ob->gfx_name == NULL) {
			error_set(_("The `%s' object contains no audio."),
			    ob->name);
			goto fail;
		}
		if (audio_fetch(ob) == -1) {
			goto fail;
		}
		if (++ob->audio_used > OBJECT_DEP_MAX) {
			ob->audio_used = OBJECT_DEP_MAX;
		}
		break;
	case OBJECT_DATA:
		debug(DEBUG_PAGING, "data of %s; used = %u++\n", ob->name,
		    ob->data_used);
		if (ob->flags & OBJECT_NON_PERSISTENT) {
			debug(DEBUG_PAGING, "%s: non-persistent; ignoring\n",
			    ob->name);
			goto out;
		}
		if (ob->data_used == 0) {
			if (object_load_data(ob) == -1) {
				/*
				 * Assume that this failure means the data has
				 * never been saved before.
				 * XXX
				 */
				printf("%s: %s\n", ob->name, error_get());
				ob->flags |= OBJECT_DATA_RESIDENT;
			}
		}
out:
		if (++ob->data_used > OBJECT_DEP_MAX) {
			ob->data_used = OBJECT_DEP_MAX;
		}
		break;
	}
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/*
 * Decrement the reference count on the data of an object. If the count
 * reaches 0, save and release the data (preserving the dependency table).
 */
int
object_page_out(void *p, enum object_page_item item)
{
	struct object *ob = p;
	
	pthread_mutex_lock(&ob->lock);
	switch (item) {
	case OBJECT_GFX:
		debug(DEBUG_PAGING, "%s: -gfx (used=%u)\n", ob->name,
		    ob->gfx_used);
#ifdef DEBUG
		if (ob->gfx_used == 0)
			fatal("neg gfx ref count");
#endif
		if (ob->gfx_used != OBJECT_DEP_MAX &&
		    --ob->gfx_used == 0) {
			gfx_unused(ob->gfx);
			ob->gfx = NULL;
		}
		break; 
	case OBJECT_AUDIO:
#ifdef DEBUG
		if (ob->audio_used == 0)
			fatal("neg audio ref count");
#endif
		debug(DEBUG_PAGING, "%s: -audio (used=%u)\n", ob->name,
		    ob->audio_used);
		if (ob->audio_used != OBJECT_DEP_MAX &&
		    --ob->audio_used == 0) {
			audio_unused(ob->audio);
			ob->audio = NULL;
		}
		break;
	case OBJECT_DATA:
		debug(DEBUG_PAGING, "%s: -data (used=%u)\n", ob->name,
		    ob->data_used);
		if (ob->flags & OBJECT_NON_PERSISTENT) {
			debug(DEBUG_PAGING, "%s: non-persistent; skipping\n",
			    ob->name);
			goto out;
		}
#ifdef DEBUG
		if (ob->data_used == 0)
			fatal("neg data ref count");
#endif
		if (ob->data_used != OBJECT_DEP_MAX &&
		    --ob->data_used == 0) {
			if (object_save(ob) == -1) {
				goto fail;
			}
			object_free_data(ob);
		}
		break;
	}
out:
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

int
object_load(void *p)
{
	struct object *ob = p;

	lock_linkage();
	pthread_mutex_lock(&ob->lock);
	
	if (ob->flags & OBJECT_NON_PERSISTENT) {
		error_set(_("The `%s' object is non-persistent."), ob->name);
		goto fail;
	}

 	/* Load the generic part of the object and its children. */
	if (object_load_generic(ob) == -1)
		goto fail;

	/*
	 * Resolve the dependency tables now that the generic object tree
	 * is in a consistent state.
	 */
	if (object_resolve_deps(ob) == -1)
		goto fail;

	/*
	 * Reload the data of the object and its children (if resident),
	 * now that the dependency tables are resolved.
	 */
	if (object_reload_data(ob) == -1)
		goto fail;

	/* Resolve the positions now that the object tree is consistent. */
	if (ob->pos != NULL &&
	    object_resolve_position(ob) == -1)
		goto fail;

	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	return (-1);
}

/*
 * Resolve the encoded dependencies of an object and its children.
 * The object linkage must be locked.
 */
int
object_resolve_deps(void *p)
{
	struct object *ob = p, *cob;
	struct object_dep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj != NULL) {
			printf("%s: already resolved dep `%s'\n", ob->name,
			    dep->obj->name);
			continue;
		}
		dprintf("%s: resolving `%s'\n", ob->name, dep->path);
		if ((dep->obj = object_find(dep->path)) == NULL) {
			error_set(_("%s: Cannot resolve dependency `%s'"),
			    ob->name, dep->path);
			return (-1);
		}
		free(dep->path);
		dep->path = NULL;
	}

	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (object_resolve_deps(cob) == -1)
			return (-1);
	}
	return (0);
}

/*
 * Resolve the encoded position of an object and its children.
 * The object linkage must be locked.
 */
int
object_resolve_position(void *p)
{
	struct object *ob = p;

	if (ob->pos->map_name == NULL) {
		error_set(_("No position to resolve."));
		return (-1);
	}

	return (0);
}

/*
 * Reload the data of an object and its children which are currently resident.
 * The object and linkage must be locked.
 */
int
object_reload_data(void *p)
{
	struct object *ob = p, *cob;

	if (ob->flags & OBJECT_WAS_RESIDENT) {
		dprintf("`%s' is resident; reloading data\n", ob->name);
		ob->flags &= ~(OBJECT_WAS_RESIDENT);
		if (object_load_data(p) == -1)
			return (-1);
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (object_reload_data(cob) == -1)
			return (-1);
	}
	return (0);
}

/*
 * Load the generic part of an object and its children.
 * The object and linkage must be locked.
 */
int
object_load_generic(void *p)
{
	char path[MAXPATHLEN];
	struct object *ob = p;
	struct netbuf *buf;
	Uint32 count, i;
	int ti, flags, flags_save;

	if (object_copy_filename(ob, path, sizeof(path)) == -1)
		return (-1);
	if ((buf = netbuf_open(path, "rb", NETBUF_BIG_ENDIAN)) == NULL) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	if (version_read(buf, &object_ver, NULL) == -1)
		goto fail;
	
	/*
	 * Must free the resident data in order to clear the dependencies.
	 * Sets the OBJECT_WAS_RESIDENT flag to be used at data load stage.
	 */
	if (ob->flags & OBJECT_DATA_RESIDENT) {
		ob->flags |= OBJECT_WAS_RESIDENT;
		object_free_data(ob);
	}
	object_free_deps(ob);

	/* Skip the data offset. */
	read_uint32(buf);

	/* Read and verify the generic object flags. */
	flags_save = ob->flags;
	flags = (int)read_uint32(buf);
	if (flags & (OBJECT_NON_PERSISTENT|OBJECT_DATA_RESIDENT|
	    OBJECT_WAS_RESIDENT)) {
		error_set(_("The `%s' save has inconsistent flags."), ob->name);
		goto fail;
	}
	ob->flags = flags | (flags_save & OBJECT_WAS_RESIDENT);

	/* Decode the saved dependencies (to be resolved later). */
	count = read_uint32(buf);
	for (i = 0; i < count; i++) {
		struct object_dep *dep;

		dep = Malloc(sizeof(struct object_dep));
		dep->path = read_string(buf);
		dep->obj = NULL;
		dep->count = 0;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
		debug(DEBUG_DEPS, "%s: depends on `%s'\n", ob->name, dep->path);
	}

	/* Decode the generic properties. */
	if (prop_load(ob, buf) == -1)
		goto fail;

	/* Decode and restore the position, if there is one. */
	if (read_uint32(buf) > 0)
		position_load(ob, ob->pos, buf);

	/*
	 * Decode the gfx/audio references. Try to resolve them immediately
	 * only if there is currently resident media.
	 */
	Free(ob->gfx_name);
	Free(ob->audio_name);
	if ((ob->gfx_name = read_string_len(buf, OBJECT_PATH_MAX)) != NULL) {
		if (ob->gfx != NULL) {
			if (gfx_fetch(ob) == -1)
				goto fail;
		}
	} else {
		if (ob->gfx != NULL) {
			gfx_unused(ob->gfx);
			ob->gfx = NULL;
		}
	}
	dprintf("%s: gfx=%s\n", ob->name, ob->gfx_name);
	ob->audio_name = read_string_len(buf, OBJECT_PATH_MAX);
	/* XXX ... */

	/*
	 * Load the generic part of the child objects.
	 *
	 * If a saved object matches an existing object's name and type,
	 * invoke reinit on it (and reload its data if it is resident).
	 * Otherwise, allocate and attach a new object from scratch using
	 * the type switch.
	 *
	 * Try to destroy the attached objects which do not have saved
	 * states, and are not currently in use.
	 *
	 * XXX ensure that there is no duplicate names.
	 * XXX destroy unmatched objects.
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
			if (object_load_generic(eob) == -1)
				goto fail;
		} else {
			dprintf("%s: alloc %s (%s)\n", ob->name, cname, ctype);
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
			    cname, typesw[ti].type,
			    (unsigned long)typesw[ti].size);

			child = Malloc(typesw[ti].size);
			if (typesw[ti].ops->init != NULL) {
				typesw[ti].ops->init(child, cname);
			} else {
				object_init(child, ctype, cname,
				    typesw[ti].ops);
			}
			object_attach(ob, child);
			if (object_load_generic(child) == -1)
				goto fail;
		}
	}

	netbuf_close(buf);
	return (0);
fail:
	object_free_data(ob);
	object_free_deps(ob);
	netbuf_close(buf);
	return (-1);
}

/*
 * Load object data. Called as part of a page in operation, for reading
 * data when saving a non-resident object, and from object_load() for
 * reloading data of resident objects.
 *
 * The object must be locked.
 *
 * XXX no provision for saved data being out of sync with the generic object.
 * XXX encode some sort of key?
 */
int
object_load_data(void *p)
{
	char path[MAXPATHLEN];
	struct object *ob = p;
	struct netbuf *buf;
	off_t data_offs;

	if (ob->flags & OBJECT_DATA_RESIDENT) {
		error_set(_("The data of `%s' is already resident."), ob->name);
		return (-1);
	}

	if (object_copy_filename(ob, path, sizeof(path)) == -1)
		return (-1);
	if ((buf = netbuf_open(path, "rb", NETBUF_BIG_ENDIAN)) == NULL) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}

	if (version_read(buf, &object_ver, NULL) == -1)
		goto fail;

	data_offs = (off_t)read_uint32(buf);
	netbuf_seek(buf, data_offs, SEEK_SET);
	if (ob->ops->load != NULL &&
	    ob->ops->load(ob, buf) == -1)
		goto fail;

	ob->flags |= OBJECT_DATA_RESIDENT;
	netbuf_close(buf);
	return (0);
fail:
	netbuf_close(buf);
	return (-1);
}

/* Save the state of an object and its children. */
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
	off_t count_offs, data_offs;
	Uint32 count;
	struct object_dep *dep;
	int was_resident;
	
	lock_linkage();
	pthread_mutex_lock(&ob->lock);

	if (ob->flags & OBJECT_NON_PERSISTENT) {
		error_set(_("The `%s' object is non-persistent."), ob->name);
		goto fail_lock;
	}
	was_resident = ob->flags & OBJECT_DATA_RESIDENT;
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
		goto fail_lock;
	}

	/* Page in the data unless it is already resident. */
	if (!was_resident) {
		debug(DEBUG_PAGING, "paging `%s' data for save\n", ob->name);
		if (object_load_data(ob) == -1) {
			/*
			 * Assume that this failure means the data has never
			 * been saved before.
			 * XXX
			 */
			dprintf("%s: %s\n", ob->name, error_get());
			ob->flags |= OBJECT_DATA_RESIDENT;
		}
	}

	strlcpy(save_file, save_dir, sizeof(save_file));
	strlcat(save_file, "/", sizeof(save_file));
	strlcat(save_file, ob->name, sizeof(save_file));
	strlcat(save_file, ".", sizeof(save_file));
	strlcat(save_file, ob->type, sizeof(save_file));

	if ((buf = netbuf_open(save_file, "wb", NETBUF_BIG_ENDIAN)) == NULL)
		goto fail_reinit;

	version_write(buf, &object_ver);

	data_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	write_uint32(buf, (Uint32)(ob->flags & OBJECT_SAVED_FLAGS));

	/* Encode the object dependencies. */
	count_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	for (dep = TAILQ_FIRST(&ob->deps), count = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), count++) {
		char dep_name[OBJECT_PATH_MAX];
		
		object_copy_name(dep->obj, dep_name, sizeof(dep_name));
		write_string(buf, dep_name);
	}
	pwrite_uint32(buf, count, count_offs);

	/* Encode the generic properties. */
	if (prop_save(ob, buf) == -1)
		goto fail;
	
	/* Encode the position if there is one. */
	if (ob->pos != NULL) {
		write_uint32(buf, 1);
		if (position_save(ob->pos, buf) == -1)
			goto fail;
	} else {
		write_uint32(buf, 0);
	}

	/* Encode the media references. */
	write_string(buf, ob->gfx_name);
	write_string(buf, ob->audio_name);
	
	/* Save the child objects. */
	count_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	count = 0;
	TAILQ_FOREACH(child, &ob->children, cobjs) {
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
	pwrite_uint32(buf, netbuf_tell(buf), data_offs);

	/* Save the object derivate data. */
	if (ob->ops->save != NULL &&
	    ob->ops->save(ob, buf) == -1)
		goto fail;

	netbuf_flush(buf);
	netbuf_close(buf);
	if (!was_resident) {
		object_free_data(ob);
	}
	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
	return (0);
fail:
	netbuf_close(buf);
fail_reinit:
	if (!was_resident)
		object_free_data(ob);
fail_lock:
	pthread_mutex_unlock(&ob->lock);
	unlock_linkage();
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

/* Load graphics and leave resident (ie. for widgets). */
void
object_wire_gfx(void *p, const char *key)
{
	struct object *ob = p;

	Free(ob->gfx_name);
	ob->gfx_name = Strdup(key);
	if (gfx_fetch(ob) == -1) {
		fatal("%s: %s", key, error_get());
	}
	gfx_wire(ob->gfx);
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
object_dep_index(const void *p, const void *depobjp)
{
	const struct object *ob = p;
	const struct object *depobj = depobjp;
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
		if ((ob->flags & OBJECT_PRESERVE_DEPS) == 0) {
			debug(DEBUG_DEPS, "%s: -[%s]\n", ob->name,
			    OBJECT(depobj)->name);
			TAILQ_REMOVE(&ob->deps, dep, deps);
			free(dep);
		} else {
			dep->count = 0;
		}
	} else if (dep->count == 0) {
		fatal("neg ref count");
	} else {
		debug(DEBUG_DEPS, "%s: [%s/%u]\n", ob->name,
		    OBJECT(depobj)->name, dep->count);
		dep->count--;
	}
}

/* Move an object towards the head of its parent's children list. */
void
object_move_up(void *p)
{
	struct object *ob = p, *prev;
	struct object *parent = ob->parent;

	if (parent == NULL || ob == TAILQ_FIRST(&parent->children))
		return;

	prev = TAILQ_PREV(ob, objectq, cobjs);
	TAILQ_REMOVE(&parent->children, ob, cobjs);
	TAILQ_INSERT_BEFORE(prev, ob, cobjs);
}

/* Move an object towards the tail of its parent's children list. */
void
object_move_down(void *p)
{
	struct object *ob = p;
	struct object *parent = ob->parent;
	struct object *next = TAILQ_NEXT(ob, cobjs);

	if (parent == NULL || next == NULL)
		return;

	TAILQ_REMOVE(&parent->children, ob, cobjs);
	TAILQ_INSERT_AFTER(&parent->children, next, ob, cobjs);
}

/* Make sure that an object's name is unique. */
static void
object_rename_unique(struct object *obj)
{
	struct object *oob, *oparent = obj->parent;
	char basename[OBJECT_NAME_MAX];
	char newname[OBJECT_NAME_MAX];
	size_t len, i;
	char *c, *num;
	unsigned int n = 0;

rename:
	num = NULL;
	len = strlen(obj->name);
	for (i = len-1; i > 0; i--) {
		if (!isdigit(obj->name[i])) {
			num = &obj->name[i+1];
			break;
		}
	}
	*num = '\0';
	strlcpy(basename, obj->name, sizeof(basename));
	snprintf(newname, sizeof(newname), "%s%u", basename, n++);

	TAILQ_FOREACH(oob, &oparent->children, cobjs) {
		if (strcmp(oob->name, newname) == 0)
			break;
	}
	if (oob != NULL) {
		goto rename;
	}
	strlcpy(obj->name, newname, sizeof(obj->name));
}

/*
 * Remove the data files of an object and its children.
 * The linkage must be locked.
 */
static void
object_unlink_datafiles(void *p)
{
	char path[MAXPATHLEN];
	struct object *ob = p, *cob;

	if (object_copy_filename(ob, path, sizeof(path)) == 0)
		unlink(path);

	TAILQ_FOREACH(cob, &ob->children, cobjs)
		object_unlink_datafiles(cob);

	if (object_copy_dirname(ob, path, sizeof(path)) == 0)
		rmdir(path);
}

/*
 * Duplicate an object and its children.
 * The position is not duplicated.
 */
void *
object_duplicate(void *p)
{
	char oname[OBJECT_NAME_MAX];
	struct object *ob = p;
	struct object *dob;
	struct object_type *t;

	for (t = &typesw[0]; t < &typesw[ntypesw]; t++)
		if (strcmp(ob->type, t->type) == 0)
			break;
#ifdef DEBUG
	if (t == &typesw[ntypesw])
		fatal("unrecognized object type");
#endif
	dob = Malloc(t->size);

	pthread_mutex_lock(&ob->lock);

	/* Create the duplicate object. */
	if (t->ops->init != NULL) {
		t->ops->init(dob, ob->name);
	} else {
		object_init(dob, ob->type, ob->name, t->ops);
	}

	if (object_page_in(ob, OBJECT_DATA) == -1)
		goto fail;

	/* Change the name and attach to the same parent as the original. */
	object_attach(ob->parent, dob);
	object_rename_unique(dob);
	dob->flags = (ob->flags & OBJECT_DUPED_FLAGS);

	/* Save the state of the original object using the new name. */
	strlcpy(oname, ob->name, sizeof(oname));
	strlcpy(ob->name, dob->name, sizeof(ob->name));
	if (object_save(ob) == -1) {
		object_page_out(ob, OBJECT_DATA);
		goto fail;
	}

	if (object_page_out(ob, OBJECT_DATA) == -1)
		goto fail;

	if (object_load(dob) == -1)
		goto fail;

	strlcpy(ob->name, oname, sizeof(ob->name));
	pthread_mutex_unlock(&ob->lock);
	return (dob);
fail:
	strlcpy(ob->name, oname, sizeof(ob->name));
	pthread_mutex_unlock(&ob->lock);
	object_destroy(dob);
	free(dob);
	return (NULL);
}

#ifdef EDITION

/* Update the dependencies display. */
static void
object_poll_deps(int argc, union evarg *argv)
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
		text_msg(MSG_ERROR, ".: %s", strerror(errno));
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
					text_msg(MSG_ERROR, "..: %s",
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
	struct tlist_item *it = argv[2].p;
	
	Free(ob->gfx_name);

	if (it->text[0] == '\0') {
		if (ob->gfx != NULL) {
			gfx_unused(ob->gfx);
		}
		ob->gfx = NULL;
		ob->gfx_name = NULL;
	} else {
		ob->gfx_name = Strdup(it->text);
		/* XXX don't fetch immediately */
		if (gfx_fetch(ob) == -1) {
			text_msg(MSG_ERROR, "%s: %s", ob->gfx_name,
			    error_get());
			free(ob->gfx_name);
			ob->gfx_name = NULL;
		} else {
			ob->gfx_name = Strdup(it->text);
		}
	}
}

static void
select_audio(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;
	struct tlist_item *it = argv[2].p;
	
	Free(ob->audio_name);

	if (it->text[0] == '\0') {
		if (ob->audio != NULL) {
			audio_unused(ob->audio);
		}
		ob->audio = NULL;
		ob->audio_name = NULL;
	} else {
		ob->audio_name = Strdup(it->text);
		if (audio_fetch(ob) == -1) {
			text_msg(MSG_ERROR, "%s: %s", ob->audio_name,
			    error_get());
			free(ob->audio_name);
			ob->audio_name = NULL;
		} else {
			ob->audio_name = Strdup(it->text);
		}
	}
}

static void
object_name_prechg(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;

	object_page_in(ob, OBJECT_DATA);
	object_unlink_datafiles(ob);
}

static void
object_name_postchg(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;

	object_page_out(ob, OBJECT_DATA);
}

struct window *
object_edit(void *p)
{
	struct object *ob = p;
	struct window *win;
	struct box *bo;
	struct textbox *tbox;

	win = window_new(NULL);
	window_set_caption(win, _("%s object"), ob->name);
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 0);
	window_set_closure(win, WINDOW_DETACH);

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		char den_path[MAXPATHLEN];
		struct combo *gfx_com, *aud_com;
		char *dir, *last;
	
		tbox = textbox_new(bo, _("Name: "));
		widget_bind(tbox, "string", WIDGET_STRING, ob->name,
		    sizeof(ob->name));
		event_new(tbox, "textbox-prechg", object_name_prechg, "%p", ob);
		event_new(tbox, "textbox-postchg", object_name_postchg, "%p",
		    ob);

		gfx_com = combo_new(bo, 0, _("Graphics: "));
		aud_com = combo_new(bo, 0, _("Audio: "));

		textbox_prescale(gfx_com->tbox, "XXXXXXXXXXXXXX");

		event_new(gfx_com, "combo-selected", select_gfx, "%p", ob);
		event_new(aud_com, "combo-selected", select_audio, "%p", ob);

		if (ob->gfx_name != NULL)
			textbox_printf(gfx_com->tbox, "%s", ob->gfx_name);
		if (ob->audio_name != NULL)
			textbox_printf(aud_com->tbox, "%s", ob->audio_name);

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
	
	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		label_new(bo, _("Type: %s"), ob->type);
		label_polled_new(bo, &ob->lock, _("Flags : 0x%x"), &ob->flags);

		label_polled_new(bo, &linkage_lock, _("Parent: %[obj]"),
		    &ob->parent);
		label_polled_new(bo, &ob->lock, "Pos: %p", &ob->pos);
		label_polled_new(bo, &ob->lock,
		    "Refs: gfx=%[u32], audio=%[u32], data=%[u32]",
		    &ob->gfx_used, &ob->audio_used, &ob->data_used);
	}


	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	box_set_padding(bo, 2);
	{
		struct tlist *tl;

		label_new(bo, _("Dependencies:"));
		tl = tlist_new(bo, TLIST_POLL);
		tlist_prescale(tl, "XXXXXXXXXXXX", 2);
		event_new(tl, "tlist-poll", object_poll_deps, "%p", ob);
	}
	return (win);
}

#endif /* EDITION */
