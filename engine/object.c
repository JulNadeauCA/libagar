/*	$Csoft: object.c,v 1.200 2005/04/18 03:38:21 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/input.h>
#include <engine/view.h>
#include <engine/typesw.h>
#include <engine/mkpath.h>
#include <engine/objmgr.h>

#include <engine/map/map.h>
#include <engine/map/rootmap.h>

#ifdef EDITION
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#include <engine/widget/textbox.h>
#include <engine/widget/notebook.h>
#include <engine/widget/separator.h>
#endif

#include <sys/stat.h>

#include <ctype.h>
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
#define DEBUG_STATE	0x01
#define DEBUG_DEPS	0x04
#define DEBUG_DEPRESV	0x08
#define DEBUG_LINKAGE	0x10
#define DEBUG_GC	0x20

int	object_debug = DEBUG_STATE|DEBUG_DEPRESV;
#define engine_debug object_debug
#endif

int object_ignore_data_errors;		/* Don't fail on a data load failure. */

/* Allocate, initialize and attach a generic object. */
struct object *
object_new(void *parent, const char *name)
{
	struct object *ob;

	ob = Malloc(sizeof(struct object), M_OBJECT);
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
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->children);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	CIRCLEQ_INIT(&ob->timeouts);
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
 * Return the root of a given object's ancestry.
 * The linkage must be locked.
 */
void *
object_root(const void *p)
{
	const struct object *ob = p;

	while (ob != NULL) {
		if (ob->parent == NULL) {
			return ((void *)ob);
		}
		ob = ob->parent;
	}
	return (NULL);
}

/*
 * Traverse an object's ancestry looking for a matching parent object.
 * The linkage must be locked.
 */
void *
object_find_parent(void *obj, const char *name, const char *type)
{
	struct object *ob = obj;

	while (ob != NULL) {
		struct object *po = ob->parent;

		if (po == NULL) {
			return (NULL);
		}
		if ((type == NULL || strcmp(po->type, type) == 0) &&
		    (name == NULL || strcmp(po->name, name) == 0)) {
			return (po);
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
find_depended(const void *p, const void *robj)
{
	const struct object *ob = p, *cob;
	struct object_dep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == robj &&
		    robj != ob) {
			error_set(_("The `%s' object is used by `%s'."),
			    OBJECT(robj)->name, ob->name);
			return (1);
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (find_depended(cob, robj))
			return (1);
	}
	return (0);
}

/*
 * Return 1 if the given object or one of its children is being referenced.
 * The linkage must be locked.
 */
int
object_in_use(const void *p)
{
	const struct object *ob = p, *cob;
	struct object *root;

	root = object_root(ob);
	if (find_depended(root, ob))
		return (1);

	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (object_in_use(cob))
			return (1);
	}
	return (0);
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
	event_post(child, parent, "child-attached", NULL);
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
	pthread_mutex_lock(&child->lock);

	/* Cancel scheduled non-detachable timeouts. */
	object_cancel_timeouts(child, TIMEOUT_DETACHABLE);

	TAILQ_REMOVE(&parent->children, child, cobjs);
	child->parent = NULL;
	event_post(parent, child, "detached", NULL);
	event_post(child, parent, "child-detached", NULL);
	debug(DEBUG_LINKAGE, "%s: detached from %s\n", child->name,
	    parent->name);

	pthread_mutex_unlock(&child->lock);
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
		Free(dep, M_DEP);
	}
	TAILQ_INIT(&ob->deps);
}

/*
 * Remove any dependencies of the object and its children with a reference
 * count of zero (as used by the load process to resolve object references).
 */
void
object_free_zerodeps(struct object *ob)
{
	struct object *cob;
	struct object_dep *dep, *ndep;

	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);
		if (dep->count == 0) {
			TAILQ_REMOVE(&ob->deps, dep, deps);
			Free(dep, M_DEP);
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs)
		object_free_zerodeps(cob);
}

/*
 * Detach the child objects, and free them, assuming that none of them
 * are currently in use.
 */
void
object_free_children(struct object *pob)
{
	struct object *cob, *ncob;

	pthread_mutex_lock(&pob->lock);
	for (cob = TAILQ_FIRST(&pob->children);
	     cob != TAILQ_END(&pob->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		debug(DEBUG_GC, "%s: freeing %s\n", pob->name, cob->name);
		object_destroy(cob);
		if ((cob->flags & OBJECT_STATIC) == 0)
			Free(cob, M_OBJECT);
	}
	TAILQ_INIT(&pob->children);
	pthread_mutex_unlock(&pob->lock);
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
		Free(prop, M_PROP);
	}
	TAILQ_INIT(&ob->props);
	pthread_mutex_unlock(&ob->lock);
}

/*
 * Destroy the event handlers registered by an object, cancelling
 * any scheduled execution.
 */
void
object_free_events(struct object *ob)
{
	struct event *eev, *neev;

	pthread_mutex_lock(&ob->lock);
	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = neev) {
		neev = TAILQ_NEXT(eev, events);
	
		if (eev->flags & EVENT_SCHEDULED) {
			event_cancel(ob, eev->name);
		}
		Free(eev, M_EVENT);
	}
	TAILQ_INIT(&ob->events);
	pthread_mutex_unlock(&ob->lock);
}

/* Cancel any scheduled timeout(3) event associated with the object. */
void
object_cancel_timeouts(struct object *ob, int flags)
{
	extern pthread_mutex_t timeout_lock;
	extern struct objectq timeout_objq;
	struct event *ev;
	struct timeout *to, *nto;

	pthread_mutex_lock(&timeout_lock);
	pthread_mutex_lock(&ob->lock);

	TAILQ_FOREACH(ev, &ob->events, events) {
		if ((ev->flags & EVENT_SCHEDULED) &&
		    (ev->timeout.flags & flags) == 0) {
			dprintf("%s: cancelling scheduled `%s' ev\n", ob->name,
			    ev->name);
			timeout_del(ob, &ev->timeout);
			ev->flags &= ~(EVENT_SCHEDULED);
		}
	}
	CIRCLEQ_FOREACH(to, &ob->timeouts, timeouts) {
		dprintf("%s: cancelling scheduled timeout (%d tick, ival %d)\n",
		    ob->name, to->ticks, to->ival);
		TAILQ_REMOVE(&timeout_objq, ob, tobjs);
	}
	CIRCLEQ_INIT(&ob->timeouts);

	pthread_mutex_unlock(&ob->lock);
	pthread_mutex_unlock(&timeout_lock);
}

/*
 * Release the resources allocated by an object and its children, assuming
 * that none of them is currently in use.
 */
void
object_destroy(void *p)
{
	struct object *ob = p;
	struct object_dep *dep, *ndep;

	debug(DEBUG_GC, "destroy %s (parent=%p)\n", ob->name, ob->parent);

	/* Cancel every scheduled timeout event. */
	object_cancel_timeouts(ob, 0);

	/* Destroy the descendants recursively. */
	object_free_children(ob);
	
	if (ob->ops->reinit != NULL) {
		ob->ops->reinit(ob);
	}
	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);
		Free(dep, M_DEP);
	}

	if (ob->ops->destroy != NULL)
		ob->ops->destroy(ob);

	Free(ob->gfx_name, 0);
	Free(ob->audio_name, 0);
	if (ob->gfx != NULL) {
		gfx_unused(ob->gfx);
	}
	if (ob->audio != NULL) {
		audio_unused(ob->audio);
	}
	object_free_props(ob);
	object_free_events(ob);
	pthread_mutex_destroy(&ob->lock);
}

/* Copy the full pathname to an object's data file to a fixed-size buffer. */
/* XXX modifies buffer even on failure */
int
object_copy_filename(const void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[OBJECT_PATH_MAX];
	const struct object *ob = p;
	struct stat sta;
	char *dir;

	prop_copy_string(config, "load-path", load_path, sizeof(load_path));
	object_copy_name(ob, obj_name, sizeof(obj_name));

	for (dir = strsep(&loadpathp, ":");
	     dir != NULL;
	     dir = strsep(&loadpathp, ":")) {
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
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[OBJECT_PATH_MAX];
	const struct object *ob = p;
	struct stat sta;
	char *dir;

	prop_copy_string(config, "load-path", load_path, sizeof(load_path));
	object_copy_name(ob, obj_name, sizeof(obj_name));

	for (dir = strsep(&loadpathp, ":");
	     dir != NULL;
	     dir = strsep(&loadpathp, ":")) {
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
		if (ob->gfx == NULL) {
			if (ob->gfx_name != NULL &&
			   (ob->gfx = gfx_fetch_shd(ob->gfx_name)) == NULL) {
				goto fail;
			}
			ob->gfx_used = 1;
		} else {
			if (++ob->gfx_used > OBJECT_DEP_MAX)
				ob->gfx_used = OBJECT_DEP_MAX;
		}
		break;
	case OBJECT_AUDIO:
		if (ob->audio == NULL) {
			if (ob->audio_name != NULL &&
			   (ob->audio = audio_fetch(ob->audio_name)) == NULL) {
				goto fail;
			}
			ob->audio_used = 1;
		} else {
			if (++ob->audio_used > OBJECT_DEP_MAX)
				ob->audio_used = OBJECT_DEP_MAX;
		}
		break;
	case OBJECT_DATA:
		if (ob->flags & OBJECT_NON_PERSISTENT) {
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

/* Page out or decrement reference counts on media/data. */
int
object_page_out(void *p, enum object_page_item item)
{
	struct object *ob = p;
	
	pthread_mutex_lock(&ob->lock);
	switch (item) {
	case OBJECT_GFX:
#ifdef DEBUG
		if (ob->gfx_used == 0)
			fatal("neg gfx ref count");
#endif
		if (ob->gfx != NULL &&
		    ob->gfx_used != OBJECT_DEP_MAX &&
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
		if (ob->audio != NULL &&
		    ob->audio_used != OBJECT_DEP_MAX &&
		    --ob->audio_used == 0) {
			audio_unused(ob->audio);
			ob->audio = NULL;
		}
		break;
	case OBJECT_DATA:
		if (ob->flags & OBJECT_NON_PERSISTENT)
			goto done;
#ifdef DEBUG
		if (ob->data_used == 0)
			fatal("neg data ref count");
#endif
		if (ob->data_used != OBJECT_DEP_MAX &&
		    --ob->data_used == 0) {
		    	extern int mapedition;

			if (mapedition && (ob->flags & OBJECT_EDIT_RESIDENT)) {
				ob->data_used = 1;
			} else {
				if (object_save(ob) == -1) {
					goto fail;
				}
				object_free_data(ob);
			}
		}
		break;
	}
done:
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
	
	/* Cancel scheduled non-loadable timeouts. */
	object_cancel_timeouts(ob, TIMEOUT_LOADABLE);
	
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
		debug_n(DEBUG_DEPRESV, "%s: depends on %s...", ob->name,
		    dep->path);
		if (dep->obj != NULL) {
			debug_n(DEBUG_DEPRESV, "already resolved\n");
			continue;
		}
		if ((dep->obj = object_find(dep->path)) == NULL) {
			debug_n(DEBUG_DEPRESV, "unexisting\n");
			error_set(_("%s: Cannot resolve dependency `%s'"),
			    ob->name, dep->path);
			return (-1);
		}
		debug_n(DEBUG_DEPRESV, "%p (%s)\n", dep->obj, dep->obj->name);
		Free(dep->path, 0);
		dep->path = NULL;
	}

	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (cob->flags & OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (object_resolve_deps(cob) == -1)
			return (-1);
	}
	return (0);
}

/*
 * Reload the data of an object and its children which are currently resident
 * (or have the special OBJECT_EDIT_RESIDENT flag set). The object and linkage
 * must be locked.
 */
int
object_reload_data(void *p)
{
	struct object *ob = p, *cob;
	extern int mapedition;

	if ((ob->flags & OBJECT_WAS_RESIDENT) ||
	    (mapedition && (ob->flags & OBJECT_EDIT_RESIDENT))) {
		ob->flags &= ~(OBJECT_WAS_RESIDENT);
		if (object_load_data(ob) == -1) {
			if (object_ignore_data_errors) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
			} else {
				return (-1);
			}
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (object_reload_data(cob) == -1) {
			if (object_ignore_data_errors) {
				text_msg(MSG_ERROR, "%s: %s", cob->name,
				    error_get());
			} else {
				return (-1);
			}
		}
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
	char *mname;

	if (object_copy_filename(ob, path, sizeof(path)) == -1)
		return (-1);
	if ((buf = netbuf_open(path, "rb", NETBUF_BIG_ENDIAN)) == NULL) {
		error_set("%s: %s", path, strerror(errno));
		return (-1);
	}
	if (version_read(buf, &object_ver, NULL) == -1)
		goto fail;
	
	debug(DEBUG_STATE, "loading %s (generic)\n", ob->name);
	
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

		dep = Malloc(sizeof(struct object_dep), M_DEP);
		dep->path = read_string(buf);
		dep->obj = NULL;
		dep->count = 0;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
		debug(DEBUG_DEPS, "%s: depends on `%s'\n", ob->name, dep->path);
	}

	/* Decode the generic properties. */
	if (prop_load(ob, buf) == -1)
		goto fail;

	read_uint32(buf);				/* Pad (was position) */

	/* Decode shared gfx reference and resolve if graphics are resident. */
	if ((mname = read_string_len(buf, OBJECT_PATH_MAX)) != NULL) {
		Free(ob->gfx_name, 0);
		ob->gfx_name = mname;
		if (ob->gfx != NULL) {
			gfx_unused(ob->gfx);
			if ((ob->gfx = gfx_fetch_shd(ob->gfx_name)) == NULL) {
				Free(ob->gfx_name, 0);
				ob->gfx_name = NULL;
				goto fail;
			}
		}
	} else {
		if (ob->gfx_name != NULL) {
			Free(ob->gfx_name, 0);
			if (ob->gfx != NULL) {
				gfx_unused(ob->gfx);
				ob->gfx = NULL;
			}
		}
	}

	/* Decode shared audio reference and resolve if audio is resident. */
	if ((mname = read_string_len(buf, OBJECT_PATH_MAX)) != NULL) {
		Free(ob->audio_name, 0);
		ob->audio_name = mname;
		if (ob->audio != NULL) {
			audio_unused(ob->audio);
			if ((ob->audio = audio_fetch(ob->audio_name)) == NULL) {
				Free(ob->audio_name, 0);
				ob->audio_name = NULL;
				goto fail;
			}
		}
	} else {
		if (ob->audio_name != NULL) {
			Free(ob->audio_name, 0);
			if (ob->audio != NULL) {
				audio_unused(ob->audio);
				ob->audio = NULL;
			}
		}
	}

	/*
	 * Load the generic part of the child objects.
	 *
	 * If a saved object matches an existing object's name and type,
	 * invoke reinit on it (and reload its data if it is resident).
	 * Otherwise, allocate and attach a new object from scratch using
	 * the type switch.
	 *
	 * XXX ensure that there are no duplicate names.
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
			if (object_load_generic(eob) == -1) {
				goto fail;
			}
		} else {
		 	for (ti = 0; ti < ntypesw; ti++) {
				if (strcmp(typesw[ti].type, ctype) == 0)
					break;
			}
			if (ti == ntypesw) {
				error_set(_("Unknown object type: `%s'"),
				    ctype);
				goto fail;
			}

			child = Malloc(typesw[ti].size, M_OBJECT);
			if (typesw[ti].ops->init != NULL) {
				typesw[ti].ops->init(child, cname);
			} else {
				object_init(child, ctype, cname,
				    typesw[ti].ops);
			}
			object_attach(ob, child);
			if (object_load_generic(child) == -1) {
				goto fail;
			}
		}
#if 0
		/*
		 * Destroy any attached object without a match in the
		 * save (that is not currently in use).
		 */
		OBJECT_FOREACH_CHILD(eob, ob, object) {
			if (eob->flags & OBJECT_IN_SAVE) {
				continue;
			}
			if (!object_in_use(eob)) {
				dprintf("%s: not in save; destroying\n",
				    eob->name);
				object_detach(eob);
				object_unlink_datafiles(eob);
				object_destroy(eob);
				if ((eob->flags & OBJECT_STATIC) == 0)
					Free(eob, M_OBJECT);
			} else {
				/* XXX */
				dprintf("%s: not in save; detaching\n",
				    OBJECT(eob)->name);
				text_msg(MSG_ERROR,
				    _("Detaching `%s' (not in save)."),
				    eob->name);
				object_detach(eob);
			}
		}
#endif
	}

	netbuf_close(buf);
	if (ob->flags & OBJECT_REOPEN_ONLOAD) {
		objmgr_reopen(ob);
	}
	return (0);
fail:
	object_free_data(ob);
	object_free_deps(ob);
	netbuf_close(buf);
	if (ob->flags & OBJECT_REOPEN_ONLOAD) {
		objmgr_reopen(ob);
	}
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
	debug(DEBUG_STATE, "loading %s (data)\n", ob->name);

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
	debug(DEBUG_STATE, "saving %s\n", ob->name);

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
	    mkpath(save_dir) == -1) {
		error_set("mkpath %s: %s", save_dir, strerror(errno));
		goto fail_lock;
	}

	/* Page in the data unless it is already resident. */
	if (!was_resident) {
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
	
	write_uint32(buf, 0);				/* Pad (was position) */

	/* Encode the media references. */
	write_string(buf, ob->gfx_name);
	write_string(buf, ob->audio_name);
	
	/* Save the child objects. */
	count_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	count = 0;
	TAILQ_FOREACH(child, &ob->children, cobjs) {
		if (child->flags & OBJECT_NON_PERSISTENT) {
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

/* Associate shared graphics and prevent any pageout. */
void
object_wire_gfx(void *p, const char *key)
{
	struct object *ob = p;

	Free(ob->gfx_name, 0);
	ob->gfx_name = Strdup(key);
	if ((ob->gfx = gfx_fetch_shd(key)) == NULL) {
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
		dep = Malloc(sizeof(struct object_dep), M_DEP);
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
			Free(dep, M_DEP);
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
	u_int n = 0;

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
void
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

/* Duplicate an object and its children. */
/* XXX EXPERIMENTAL */
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
	dob = Malloc(t->size, M_OBJECT);

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
	Free(dob, M_OBJECT);
	return (NULL);
}

SDL_Surface *
object_icon(void *p)
{
	struct object *obj = p;
	int i;
		 
	for (i = 0; i < ntypesw; i++) {
		if (strcmp(typesw[i].type, obj->type) == 0)
			return (typesw[i].icon >= 0 ? ICON(typesw[i].icon) :
			    NULL);
	}
	return (NULL);
}

#ifdef EDITION

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
		tlist_insert_item(tl, object_icon(dep->obj), label, dep);
	}
	unlock_linkage();
	tlist_restore_selections(tl);
}

static void
poll_gfx(int argc, union evarg *argv)
{
	extern char *gfx_snap_names[];
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct gfx *gfx = ob->gfx;
	struct tlist_item *it;
	Uint32 i;
	
	if (gfx == NULL)
		return;
	
	tlist_clear_items(tl);
	for (i = 0; i < gfx->nsprites; i++) {
		struct sprite *spr = &gfx->sprites[i];
		SDL_Surface *su = spr->su;
		struct gfx_cached_sprite *csp;

		if (su != NULL) {
			it = tlist_insert(tl, su, "%u. %ux%ux%u (%s)",
			    i, su->w, su->h, su->format->BitsPerPixel,
			    gfx_snap_names[spr->snap_mode]);
		} else {
			it = tlist_insert(tl, su, "%u. (null)", i);
		}
		it->p1 = spr;
		it->depth = 0;

		if (!SLIST_EMPTY(&spr->csprites)) {
			it->flags |= TLIST_HAS_CHILDREN;
		}
		SLIST_FOREACH(csp, &spr->csprites, sprites) {
			char label[TLIST_LABEL_MAX];
			struct tlist_item *it;

			snprintf(label, sizeof(label), "%u ticks\n",
			    csp->last_drawn);
			transform_print(&csp->transforms, label,
			    sizeof(label));

			it = tlist_insert_item(tl, csp->su, label, csp);
			it->depth = 1;
		}
	}
	tlist_restore_selections(tl);
}

static void
poll_props(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *ob = argv[1].p;
	struct prop *prop;
	
	tlist_clear_items(tl);
	TAILQ_FOREACH(prop, &ob->props, props) {
		char val[TLIST_LABEL_MAX];

		prop_print_value(val, sizeof(val), prop);
		tlist_insert(tl, NULL, "%s = %s", prop->key, val);
	}
	tlist_restore_selections(tl);
}

static void
rename_object(int argc, union evarg *argv)
{
	struct widget_binding *stringb;
	struct textbox *tb = argv[0].p;
	struct object *ob = argv[1].p;

	object_page_in(ob, OBJECT_DATA);
	object_unlink_datafiles(ob);
	strlcpy(ob->name, tb->string, sizeof(ob->name));
	object_page_out(ob, OBJECT_DATA);

	event_post(NULL, ob, "renamed", NULL);
}

struct window *
object_edit(void *p)
{
	struct object *ob = p;
	struct window *win;
	struct textbox *tbox;
	struct notebook *nb;
	struct notebook_tab *ntab;
	struct tlist *tl;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Object %s"), ob->name);
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 0);

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	ntab = notebook_add_tab(nb, _("Infos"), BOX_VERT);
	notebook_select_tab(nb, ntab);
	{
		tbox = textbox_new(ntab, _("Name: "));
		textbox_printf(tbox, ob->name);
		event_new(tbox, "textbox-return", rename_object, "%p", ob);
		
		separator_new(ntab, SEPARATOR_HORIZ);
	
		label_new(ntab, LABEL_STATIC, _("Type: %s"), ob->type);
		label_new(ntab, LABEL_POLLED, _("Flags : 0x%x"), &ob->flags);
		label_new(ntab, LABEL_POLLED_MT, _("Parent: %[obj]"),
		    &linkage_lock, &ob->parent);

		separator_new(ntab, SEPARATOR_HORIZ);

		label_new(ntab, LABEL_POLLED, _("Data references: %[u32]"),
		    &ob->data_used);
		label_new(ntab, LABEL_POLLED, _("Graphic references: %[u32]"),
		    &ob->gfx_used);
		label_new(ntab, LABEL_POLLED, _("Audio references: %[u32]"),
		    &ob->audio_used);
	}

	ntab = notebook_add_tab(nb, _("Deps"), BOX_VERT);
	{
		tl = tlist_new(ntab, TLIST_POLL);
		tlist_prescale(tl, "XXXXXXXXXXXX", 6);
		event_new(tl, "tlist-poll", poll_deps, "%p", ob);
	}
	
	ntab = notebook_add_tab(nb, _("Gfx"), BOX_VERT);
	{
		tl = tlist_new(ntab, TLIST_POLL);
		tlist_prescale(tl, "XXXXXXXXXXXX", 6);
		tlist_set_item_height(tl, TILESZ);
		event_new(tl, "tlist-poll", poll_gfx, "%p", ob);
	}
	
	ntab = notebook_add_tab(nb, _("Props"), BOX_VERT);
	{
		tl = tlist_new(ntab, TLIST_POLL);
		tlist_prescale(tl, "XXXXXXXXXXXX", 6);
		event_new(tl, "tlist-poll", poll_props, "%p", ob);
	}
	return (win);
}

#endif /* EDITION */
