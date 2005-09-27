/*	$Csoft: object.c,v 1.238 2005/09/27 00:25:17 vedge Exp $	*/

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

#include <compat/md5.h>
#include <compat/sha1.h>
#include <compat/rmd160.h>

#include <engine/config.h>
#include <engine/input.h>
#include <engine/view.h>
#include <engine/typesw.h>
#include <engine/mkpath.h>
#include <engine/objmgr.h>

#include <engine/map/map.h>

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

#ifdef NETWORK
#include <engine/rcs.h>
#endif

#include <sys/stat.h>

#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

const AG_Version ag_object_ver = {
	"agar object",
	7, 0
};

const AG_ObjectOps agObjectOps = {
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

int	agObjectDebugLvl = DEBUG_STATE|DEBUG_DEPRESV;
#define agDebugLvl agObjectDebugLvl
#endif

extern int agEditMode;

int agObjectIgnoreDataErrors = 0;  /* Don't fail on a data load failure. */
int agObjectIgnoreUnknownObjs = 0;   /* Don't fail on unknown object types. */

/* Allocate, initialize and attach a generic object. */
AG_Object *
AG_ObjectNew(void *parent, const char *name)
{
	AG_Object *ob;

	ob = Malloc(sizeof(AG_Object), M_OBJECT);
	AG_ObjectInit(ob, "object", name, NULL);

	if (parent != NULL) {
		AG_ObjectAttach(parent, ob);
	}
	return (ob);
}

/* Initialize a generic object structure. */
void
AG_ObjectInit(void *p, const char *type, const char *name, const void *opsp)
{
	AG_Object *ob = p;
	char *c;

	strlcpy(ob->type, type, sizeof(ob->type));
	strlcpy(ob->name, name, sizeof(ob->name));

	/* Prevent ambiguous characters in the name. */
	for (c = ob->name; *c != '\0'; c++) {
		if (*c == '/' || *c == '.' || *c == ':' || *c == ',')
			*c = '_';
	}

	ob->save_pfx = "/world";
	ob->ops = (opsp != NULL) ? opsp : &agObjectOps;
	ob->parent = NULL;
	ob->flags = 0;

	pthread_mutex_init(&ob->lock, &agRecursiveMutexAttr);
	ob->gfx = NULL;
	ob->audio = NULL;
	ob->data_used = 0;
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->children);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	CIRCLEQ_INIT(&ob->timeouts);
}

int
AG_ObjectSubclass(AG_Object *ob, const char *clname, size_t size)
{
	return (strncmp(ob->type, clname, size) == 0 &&
	        (ob->type[size] == '.' || ob->type[size] == '\0'));
}

void
AG_ObjectGetClassInfo(const char *type, AG_ObjectClassInfo *cli)
{
	char tpp[AG_OBJECT_TYPE_MAX], *tp = &tpp[0];
	char tn[AG_OBJECT_TYPE_MAX];
	char *s;
	u_int i;

	strlcpy(tpp, type, sizeof(tpp));
	tn[0] = '\0';

	cli->classes = Malloc(sizeof(char *), M_OBJECT);
	cli->types = Malloc(sizeof(AG_ObjectType *), M_OBJECT);
	cli->nclasses = 0;

	while ((s = strsep(&tp, ".")) != NULL) {
		if (tn[0] != '\0') {
			strlcat(tn, ".", sizeof(tn));
		}
		strlcat(tn, s, sizeof(tn));

		cli->classes = Realloc(cli->classes,
		    (cli->nclasses+1)*sizeof(char *));
		cli->types = Realloc(cli->types,
		    (cli->nclasses+1)*sizeof(AG_ObjectType *));
		cli->classes[cli->nclasses] = Strdup(s);
		cli->types[cli->nclasses] = AG_FindType(tn);
		cli->nclasses++;
	}
}

void
AG_ObjectFreeClassinfo(AG_ObjectClassInfo *cli)
{
	u_int i;

	for (i = 0; i < cli->nclasses; i++) {
		Free(cli->classes[i], 0);
	}
	Free(cli->classes, M_OBJECT);
	Free(cli->types, M_OBJECT);
}

void
AG_ObjectRemain(void *p, int flags)
{
	AG_Object *ob = p;

	if (flags & AG_OBJECT_REMAIN_DATA) {
		ob->flags |= (AG_OBJECT_REMAIN_DATA|AG_OBJECT_DATA_RESIDENT);
		ob->data_used = AG_OBJECT_DEP_MAX;
	} else {
		ob->flags &= ~AG_OBJECT_REMAIN_DATA;
	}
	if (flags & AG_OBJECT_REMAIN_GFX) {
		ob->flags |= AG_OBJECT_REMAIN_GFX;
		if (ob->gfx != NULL) {
			ob->gfx->used = AG_GFX_MAX_USED;
		}
	} else {
		ob->flags &= ~AG_OBJECT_REMAIN_GFX;
	}
}

/*
 * Reinitialize the state of an object (eg. free map nodes), but preserve
 * the dependencies (which are assumed to then have a reference count of 0).
 */
void
AG_ObjectFreeData(void *p)
{
	AG_Object *ob = p;

	if (ob->flags & AG_OBJECT_DATA_RESIDENT) {
		if (ob->ops->reinit != NULL) {
			ob->flags |= AG_OBJECT_PRESERVE_DEPS;
			ob->ops->reinit(ob);
			ob->flags &= ~(AG_OBJECT_PRESERVE_DEPS);
		}
		ob->flags &= ~(AG_OBJECT_DATA_RESIDENT);
	}
#if 0
	if (ob->gfx != NULL) {
		AG_GfxAllocSprites(ob->gfx, 0);
		AG_GfxAllocAnims(ob->gfx, 0);
	}
#endif
}

/* Recursive function to construct absolute object names. */
static int
AG_ObjectNameSearch(const void *obj, char *path, size_t path_len)
{
	const AG_Object *ob = obj;
	size_t name_len, cur_len;
	int rv = 0;

	cur_len = strlen(path)+1;
	name_len = strlen(ob->name)+1;
	
	if (sizeof("/")+name_len+sizeof("/")+cur_len >= path_len) {
		AG_SetError(_("The path exceeds >= %lu bytes."),
		    (unsigned long)path_len);
		return (-1);
	}
	
	/* Prepend / and the object name. */
	memmove(&path[name_len], path, cur_len);    /* Move the NUL as well */
	path[0] = '/';
	memcpy(&path[1], ob->name, name_len-1);	    /* Omit the NUL */

	if (ob->parent != agWorld && ob->parent != NULL)
		rv = AG_ObjectNameSearch(ob->parent, path, path_len);

	return (rv);
}

/*
 * Copy the absolute pathname of an object to a fixed-size buffer.
 * The buffer size must be >2 bytes.
 */
int
AG_ObjectCopyName(const void *obj, char *path, size_t path_len)
{
	const AG_Object *ob = obj;
	int rv = 0;

	path[0] = '/';
	path[1] = '\0';
	if (ob != agWorld)
		strlcat(path, ob->name, path_len);

	AG_LockLinkage();
	if (ob != agWorld && ob->parent != agWorld && ob->parent != NULL) {
		rv = AG_ObjectNameSearch(ob->parent, path, path_len);
	}
	AG_UnlockLinkage();
	return (rv);
}

/*
 * Return the root of a given object's ancestry.
 * The linkage must be locked.
 */
void *
AG_ObjectRoot(const void *p)
{
	const AG_Object *ob = p;

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
AG_ObjectFindParent(void *obj, const char *name, const char *type)
{
	AG_Object *ob = obj;

	while (ob != NULL) {
		AG_Object *po = ob->parent;

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
	const AG_Object *ob = p, *cob;
	AG_ObjectDep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == robj &&
		    robj != ob) {
			AG_SetError(_("The `%s' object is used by `%s'."),
			    AGOBJECT(robj)->name, ob->name);
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
AG_ObjectInUse(const void *p)
{
	const AG_Object *ob = p, *cob;
	AG_Object *root;

	root = AG_ObjectRoot(ob);
	if (find_depended(root, ob))
		return (1);

	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (AG_ObjectInUse(cob))
			return (1);
	}
	return (0);
}

/* Move an object to a different parent. */
void
AG_ObjectMove(void *childp, void *newparentp)
{
	AG_Object *child = childp;
	AG_Object *oparent = child->parent;
	AG_Object *nparent = newparentp;

	AG_LockLinkage();

	TAILQ_REMOVE(&oparent->children, child, cobjs);
	child->parent = NULL;
	AG_PostEvent(oparent, child, "detached", NULL);

	TAILQ_INSERT_TAIL(&nparent->children, child, cobjs);
	child->parent = nparent;
	AG_PostEvent(nparent, child, "attached", NULL);
	AG_PostEvent(oparent, child, "moved", "%p", nparent);

	debug(DEBUG_LINKAGE, "%s: %s -> %s\n", child->name, oparent->name,
	    nparent->name);

	AG_UnlockLinkage();
}

/* Attach a child object to some parent object. */
void
AG_ObjectAttach(void *parentp, void *childp)
{
	AG_Object *parent = parentp;
	AG_Object *child = childp;

	AG_LockLinkage();
	TAILQ_INSERT_TAIL(&parent->children, child, cobjs);
	child->parent = parent;
	AG_PostEvent(parent, child, "attached", NULL);
	AG_PostEvent(child, parent, "child-attached", NULL);
	debug(DEBUG_LINKAGE, "%s: parent = %s\n", child->name, parent->name);
	AG_UnlockLinkage();
}

/* Attach a child object to some parent object according to a path. */
int
AG_ObjectAttachPath(const char *path, void *child)
{
	char ppath[MAXPATHLEN];
	void *parent;
	char *p;

	if (strlcpy(ppath, path, sizeof(ppath)) >= sizeof(ppath)) {
		AG_SetError("path too big");
		goto fail;
	}
	if ((p = strrchr(ppath, '/')) != NULL) {
		*p = '\0';
	} else {
		AG_SetError("not an absolute path: `%s'", path);
		goto fail;
	}

	AG_LockLinkage();
	if (ppath[0] == '\0') {
		AG_ObjectAttach(agWorld, child);
	} else {
		if ((parent = AG_ObjectFind(ppath)) == NULL) {
			AG_SetError("%s: cannot attach to `%s': %s",
			    AGOBJECT(child)->name, ppath, AG_GetError());
			goto fail;
		}
		AG_ObjectAttach(parent, child);
	}
	AG_UnlockLinkage();
	return (0);
fail:
	AG_UnlockLinkage();
	return (-1);
}

/* Detach a child object from its parent. */
void
AG_ObjectDetach(void *childp)
{
	AG_Object *child = childp;
	AG_Object *parent = child->parent;

	AG_LockLinkage();
	pthread_mutex_lock(&child->lock);

	/* Cancel scheduled non-detachable timeouts. */
	AG_ObjectCancelTimeouts(child, AG_TIMEOUT_DETACHABLE);

	TAILQ_REMOVE(&parent->children, child, cobjs);
	child->parent = NULL;
	AG_PostEvent(parent, child, "detached", NULL);
	AG_PostEvent(child, parent, "child-detached", NULL);
	debug(DEBUG_LINKAGE, "%s: detached from %s\n", child->name,
	    parent->name);

	pthread_mutex_unlock(&child->lock);
	AG_UnlockLinkage();
}

/* Traverse the object tree using a pathname. */
static void *
AG_ObjectFindChild(const AG_Object *parent, const char *name)
{
	char node_name[AG_OBJECT_PATH_MAX];
	void *rv;
	char *s;
	AG_Object *child;

	strlcpy(node_name, name, sizeof(node_name));

	if ((s = strchr(node_name, '/')) != NULL) {
		*s = '\0';
	}
	TAILQ_FOREACH(child, &parent->children, cobjs) {
		if (strcmp(child->name, node_name) != 0)
			continue;

		if ((s = strchr(name, '/')) != NULL) {
			rv = AG_ObjectFindChild(child, &s[1]);
			if (rv != NULL) {
				return (rv);
			} else {
				return (NULL);
			}
		}
		return (child);
	}
	return (NULL);
}

/* Search for the named object (absolute path). */
void *
AG_ObjectFind(const char *name)
{
	void *rv;

#ifdef DEBUG
	if (name[0] != '/')
		fatal("not an absolute path: `%s'", name);
#endif
	if (name[0] == '/' && name[1] == '\0')
		return (agWorld);
	
	AG_LockLinkage();
	rv = AG_ObjectFindChild(agWorld, &name[1]);
	AG_UnlockLinkage();

	if (rv == NULL) {
		AG_SetError(_("The object `%s' does not exist."), name);
	}
	return (rv);
}

/* Search for the named object (absolute path). */
void *
AG_ObjectFindF(const char *fmt, ...)
{
	char path[AG_OBJECT_PATH_MAX];
	void *rv;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);
#ifdef DEBUG
	if (path[0] != '/')
		fatal("not an absolute path: `%s'", path);
#endif
	AG_LockLinkage();
	rv = AG_ObjectFindChild(agWorld, &path[1]);
	AG_UnlockLinkage();

	if (rv == NULL) {
		AG_SetError(_("The object `%s' does not exist."), path);
	}
	return (rv);
}

/* Clear the dependency table. */
void
AG_ObjectFreeDeps(AG_Object *ob)
{
	AG_ObjectDep *dep, *ndep;

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
AG_ObjectFreeZerodeps(AG_Object *ob)
{
	AG_Object *cob;
	AG_ObjectDep *dep, *ndep;

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
		AG_ObjectFreeZerodeps(cob);
}

/*
 * Detach the child objects, and free them, assuming that none of them
 * are currently in use.
 */
void
AG_ObjectFreeChildren(AG_Object *pob)
{
	AG_Object *cob, *ncob;

	pthread_mutex_lock(&pob->lock);
	for (cob = TAILQ_FIRST(&pob->children);
	     cob != TAILQ_END(&pob->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		debug(DEBUG_GC, "%s: freeing %s\n", pob->name, cob->name);
		AG_ObjectDestroy(cob);
		if ((cob->flags & AG_OBJECT_STATIC) == 0)
			Free(cob, M_OBJECT);
	}
	TAILQ_INIT(&pob->children);
	pthread_mutex_unlock(&pob->lock);
}

/* Clear an object's property table. */
void
AG_ObjectFreeProps(AG_Object *ob)
{
	AG_Prop *prop, *nextprop;

	pthread_mutex_lock(&ob->lock);
	for (prop = TAILQ_FIRST(&ob->props);
	     prop != TAILQ_END(&ob->props);
	     prop = nextprop) {
		nextprop = TAILQ_NEXT(prop, props);
		AG_PropDestroy(prop);
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
AG_ObjectFreeEvents(AG_Object *ob)
{
	AG_Event *eev, *neev;

	pthread_mutex_lock(&ob->lock);
	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = neev) {
		neev = TAILQ_NEXT(eev, events);
	
		if (eev->flags & AG_EVENT_SCHEDULED) {
			AG_CancelEvent(ob, eev->name);
		}
		Free(eev, M_EVENT);
	}
	TAILQ_INIT(&ob->events);
	pthread_mutex_unlock(&ob->lock);
}

/* Cancel any scheduled timeout(3) event associated with the object. */
void
AG_ObjectCancelTimeouts(void *p, int flags)
{
	AG_Object *ob = p, *tob;
	extern struct ag_objectq agTimeoutObjQ;
	AG_Event *ev;
	AG_Timeout *to;

	AG_LockTiming();
	pthread_mutex_lock(&ob->lock);

	TAILQ_FOREACH(ev, &ob->events, events) {
		if ((ev->flags & AG_EVENT_SCHEDULED) &&
		    (ev->timeout.flags & flags) == 0) {
			dprintf("%s: cancelling scheduled `%s'\n", ob->name,
			    ev->name);
			AG_DelTimeout(ob, &ev->timeout);
			ev->flags &= ~(AG_EVENT_SCHEDULED);
		}
	}
	TAILQ_FOREACH(tob, &agTimeoutObjQ, tobjs) {
		if (tob == ob)
			TAILQ_REMOVE(&agTimeoutObjQ, ob, tobjs);
	}
	CIRCLEQ_INIT(&ob->timeouts);

	pthread_mutex_unlock(&ob->lock);
	AG_UnlockTiming();
}

/*
 * Release the resources allocated by an object and its children, assuming
 * that none of them is currently in use.
 */
void
AG_ObjectDestroy(void *p)
{
	AG_Object *ob = p;
	AG_ObjectDep *dep, *ndep;

	debug(DEBUG_GC, "destroy %s (parent=%p)\n", ob->name, ob->parent);

	AG_ObjectCancelTimeouts(ob, 0);
	AG_ObjectFreeChildren(ob);
	
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

	if (ob->gfx != NULL) {
		AG_GfxDestroy(ob->gfx);
		ob->gfx = NULL;
	}
	if (ob->audio != NULL) {
		AG_AudioDestroy(ob->audio);
		ob->audio = NULL;
	}

	AG_ObjectFreeProps(ob);
	AG_ObjectFreeEvents(ob);
	pthread_mutex_destroy(&ob->lock);
}

/* Copy the full pathname to an object's data file to a fixed-size buffer. */
/* XXX modifies buffer even on failure */
int
AG_ObjectCopyFilename(const void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	const AG_Object *ob = p;
	struct stat sta;
	char *dir;

	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

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
	AG_SetError(_("The %s%s/%s.%s file is not in load-path."),
	    ob->save_pfx != NULL ? ob->save_pfx : "",
	    obj_name, ob->name, ob->type);
	return (-1);
}

/* Copy the full pathname of an object's data dir to a fixed-size buffer. */
int
AG_ObjectCopyDirname(const void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	const AG_Object *ob = p;
	struct stat sta;
	char *dir;

	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

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
	AG_SetError(_("The %s directory is not in load-path."), obj_name);
	return (-1);
}

/* Bring specific resources associated with an object in memory. */
int
AG_ObjectPageIn(void *p, enum ag_object_page_item item)
{
	AG_Object *ob = p;

	pthread_mutex_lock(&ob->lock);

	switch (item) {
	case AG_OBJECT_GFX:
		if (ob->gfx == NULL) {
			ob->gfx = AG_GfxNew(ob);
		}
		if (ob->gfx->used == 0 &&
		    AG_GfxLoad(ob) == -1) {
			goto fail;
		}
		if (++ob->gfx->used > AG_GFX_MAX_USED) {
			ob->gfx->used = AG_GFX_MAX_USED;
		}
		break;
	case AG_OBJECT_AUDIO:
	case AG_OBJECT_DATA:
		if (ob->flags & AG_OBJECT_NON_PERSISTENT) {
			goto out;
		}
		if (ob->data_used == 0) {
			if (AG_ObjectLoadData(ob) == -1) {
				/*
				 * Assume that this failure means the data has
				 * never been saved before.
				 * XXX
				 */
				printf("%s: %s\n", ob->name, AG_GetError());
				ob->flags |= AG_OBJECT_DATA_RESIDENT;
			}
		}
out:
		if (++ob->data_used > AG_OBJECT_DEP_MAX) {
			ob->data_used = AG_OBJECT_DEP_MAX;
		}
		break;
	}
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Remove specific object resources from memory / save to network format. */
int
AG_ObjectPageOut(void *p, enum ag_object_page_item item)
{
	AG_Object *ob = p;
	
	pthread_mutex_lock(&ob->lock);
	
	switch (item) {
	case AG_OBJECT_GFX:
		if (ob->gfx != NULL && ob->gfx->used != AG_GFX_MAX_USED) {
			if (--ob->gfx->used == 0) {
				AG_GfxAllocSprites(ob->gfx, 0);
				AG_GfxAllocAnims(ob->gfx, 0);
				/* TODO save the gfx part */
			}
		}
		break; 
	case AG_OBJECT_AUDIO:
		break; 
	case AG_OBJECT_DATA:
		if (ob->flags & AG_OBJECT_NON_PERSISTENT)
			goto done;
#ifdef DEBUG
		if (ob->data_used == 0)
			fatal("neg data ref count");
#endif
		if (ob->data_used != AG_OBJECT_DEP_MAX &&
		    --ob->data_used == 0) {
			extern int agObjMgrExiting;

			if (!agObjMgrExiting) {
				if (AG_ObjectSave(ob) == -1)
					goto fail;
			}
			AG_ObjectFreeData(ob);
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
AG_ObjectLoad(void *p)
{
	AG_Object *ob = p;

	AG_LockLinkage();
	pthread_mutex_lock(&ob->lock);
	
	/* Cancel scheduled non-loadable timeouts. */
	AG_ObjectCancelTimeouts(ob, AG_TIMEOUT_LOADABLE);
	
	if (ob->flags & AG_OBJECT_NON_PERSISTENT) {
		AG_SetError(_("The `%s' object is non-persistent."), ob->name);
		goto fail;
	}

 	/* Load the generic part of the object and its children. */
	if (AG_ObjectLoadGeneric(ob) == -1)
		goto fail;

	/*
	 * Resolve the dependency tables now that the generic object tree
	 * is in a consistent state.
	 */
	if (AG_ObjectResolveDeps(ob) == -1)
		goto fail;

	/*
	 * Reload the data of the object and its children (if resident),
	 * now that the dependency tables are resolved.
	 */
	if (AG_ObjectReloadData(ob) == -1)
		goto fail;

	pthread_mutex_unlock(&ob->lock);
	AG_UnlockLinkage();
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	AG_UnlockLinkage();
	return (-1);
}

/*
 * Resolve the encoded dependencies of an object and its children.
 * The object linkage must be locked.
 */
int
AG_ObjectResolveDeps(void *p)
{
	AG_Object *ob = p, *cob;
	AG_ObjectDep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		debug_n(DEBUG_DEPRESV, "%s: depends on %s...", ob->name,
		    dep->path);
		if (dep->obj != NULL) {
			debug_n(DEBUG_DEPRESV, "already resolved\n");
			continue;
		}
		if ((dep->obj = AG_ObjectFind(dep->path)) == NULL) {
			debug_n(DEBUG_DEPRESV, "unexisting\n");
			AG_SetError(_("%s: Cannot resolve dependency `%s'"),
			    ob->name, dep->path);
			return (-1);
		}
		debug_n(DEBUG_DEPRESV, "%p (%s)\n", dep->obj, dep->obj->name);
		Free(dep->path, 0);
		dep->path = NULL;
	}

	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (cob->flags & AG_OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (AG_ObjectResolveDeps(cob) == -1)
			return (-1);
	}
	return (0);
}

/*
 * Reload the data of an object and its children which are currently resident
 * The object and linkage must be locked.
 */
int
AG_ObjectReloadData(void *p)
{
	AG_Object *ob = p, *cob;

	if (ob->flags & (AG_OBJECT_WAS_RESIDENT|AG_OBJECT_REMAIN_DATA)) {
		ob->flags &= ~(AG_OBJECT_WAS_RESIDENT);
		if (AG_ObjectLoadData(ob) == -1) {
			if (agObjectIgnoreDataErrors) {
				AG_TextMsg(AG_MSG_ERROR, _("%s: %s (ignored)"),
				    ob->name, AG_GetError());
			} else {
				return (-1);
			}
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (AG_ObjectReloadData(cob) == -1) {
			if (agObjectIgnoreDataErrors) {
				AG_TextMsg(AG_MSG_ERROR, _("%s: %s (ignored)"),
				    cob->name, AG_GetError());
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
AG_ObjectLoadGeneric(void *p)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p;
	AG_Netbuf *buf;
	Uint32 count, i;
	int ti, flags, flags_save;
	char *mname;

	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1)
		return (-1);
	if ((buf = AG_NetbufOpen(path, "rb", AG_NETBUF_BIG_ENDIAN)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		return (-1);
	}
	if (AG_ReadVersion(buf, &ag_object_ver, NULL) == -1)
		goto fail;
	
	debug(DEBUG_STATE, "loading %s (generic)\n", ob->name);
	
	/*
	 * Must free the resident data in order to clear the dependencies.
	 * Sets the AG_OBJECT_WAS_RESIDENT flag to be used at data load stage.
	 */
	if (ob->flags & AG_OBJECT_DATA_RESIDENT) {
		ob->flags |= AG_OBJECT_WAS_RESIDENT;
		AG_ObjectFreeData(ob);
	}
	AG_ObjectFreeDeps(ob);

	/* Skip the data and gfx offsets. */
	AG_ReadUint32(buf);
	AG_ReadUint32(buf);

	/* Read and verify the generic object flags. */
	flags_save = ob->flags;
	flags = (int)AG_ReadUint32(buf);
	if (flags & (AG_OBJECT_NON_PERSISTENT|AG_OBJECT_DATA_RESIDENT|
	    AG_OBJECT_WAS_RESIDENT)) {
		AG_SetError("%s: inconsistent flags (0x%08x)", ob->name,
		    flags);
		goto fail;
	}
	ob->flags = flags | (flags_save & AG_OBJECT_WAS_RESIDENT);

	/* Decode the saved dependencies (to be resolved later). */
	count = AG_ReadUint32(buf);
	for (i = 0; i < count; i++) {
		AG_ObjectDep *dep;

		dep = Malloc(sizeof(AG_ObjectDep), M_DEP);
		dep->path = AG_ReadString(buf);
		dep->obj = NULL;
		dep->count = 0;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
		debug(DEBUG_DEPS, "%s: depends on `%s'\n", ob->name, dep->path);
	}

	/* Decode the generic properties. */
	if (AG_PropLoad(ob, buf) == -1)
		goto fail;

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
	count = AG_ReadUint32(buf);
	for (i = 0; i < count; i++) {
		char cname[AG_OBJECT_NAME_MAX];
		char ctype[AG_OBJECT_TYPE_MAX];
		AG_Object *eob, *child;

		AG_CopyString(cname, buf, sizeof(cname));
		AG_CopyString(ctype, buf, sizeof(ctype));

		AGOBJECT_FOREACH_CHILD(eob, ob, ag_object) {
			if (strcmp(eob->name, cname) == 0) 
				break;
		}
		if (eob != NULL) {
			/* XXX free the existing object or ignore */
			if (strcmp(eob->type, ctype) != 0) {
				fatal("existing object of different type");
			}
			/* XXX ignore */
			if (eob->flags & AG_OBJECT_NON_PERSISTENT) {
				fatal("existing non-persistent object");
			}
			if (AG_ObjectLoadGeneric(eob) == -1) {
				goto fail;
			}
		} else {
		 	for (ti = 0; ti < agnTypes; ti++) {
				if (strcmp(agTypes[ti].type, ctype) == 0)
					break;
			}
			if (ti == agnTypes) {
				AG_SetError(_("%s: unknown object type: `%s'"),
				    ob->name, ctype);
				if (agObjectIgnoreUnknownObjs) {
					AG_TextMsg(AG_MSG_ERROR,
					    _("%s (ignored)"), AG_GetError());
					continue;
				} else {
					goto fail;
				}
				goto fail;
			}

			child = Malloc(agTypes[ti].size, M_OBJECT);
			if (agTypes[ti].ops->init != NULL) {
				agTypes[ti].ops->init(child, cname);
			} else {
				AG_ObjectInit(child, ctype, cname,
				    agTypes[ti].ops);
			}
			AG_ObjectAttach(ob, child);
			if (AG_ObjectLoadGeneric(child) == -1) {
				goto fail;
			}
		}
#if 0
		/*
		 * Destroy any attached object without a match in the
		 * save (that is not currently in use).
		 */
		AGOBJECT_FOREACH_CHILD(eob, ob, ag_object) {
			if (eob->flags & AG_OBJECT_IN_SAVE) {
				continue;
			}
			if (!AG_ObjectInUse(eob)) {
				dprintf("%s: not in save; destroying\n",
				    eob->name);
				AG_ObjectDetach(eob);
				AG_ObjectUnlinkDatafiles(eob);
				AG_ObjectDestroy(eob);
				if ((eob->flags & AG_OBJECT_STATIC) == 0)
					Free(eob, M_OBJECT);
			} else {
				/* XXX */
				dprintf("%s: not in save; detaching\n",
				    AGOBJECT(eob)->name);
				AG_TextMsg(AG_MSG_ERROR,
				    _("Detaching `%s' (not in save)."),
				    eob->name);
				AG_ObjectDetach(eob);
			}
		}
#endif
	}

	AG_NetbufClose(buf);
	if (ob->flags & AG_OBJECT_REOPEN_ONLOAD) {
		AG_ObjMgrReopen(ob);
	}
	return (0);
fail:
	AG_ObjectFreeData(ob);
	AG_ObjectFreeDeps(ob);
	AG_NetbufClose(buf);
	if (ob->flags & AG_OBJECT_REOPEN_ONLOAD) {
		AG_ObjMgrReopen(ob);
	}
	return (-1);
}

/*
 * Load object data. Called as part of a page in operation, for reading
 * data when saving a non-resident object, and from AG_ObjectLoad() for
 * reloading data of resident objects.
 *
 * The object must be locked.
 *
 * XXX no provision for saved data being out of sync with the generic object.
 * XXX encode some sort of key?
 */
int
AG_ObjectLoadData(void *p)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p;
	AG_Netbuf *buf;
	off_t data_offs;

	if (ob->flags & AG_OBJECT_DATA_RESIDENT) {
		AG_SetError(_("The data of `%s' is already resident."), ob->name);
		return (-1);
	}
	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1)
		return (-1);
	if ((buf = AG_NetbufOpen(path, "rb", AG_NETBUF_BIG_ENDIAN)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		return (-1);
	}
	debug(DEBUG_STATE, "loading %s (data)\n", ob->name);

	if (AG_ReadVersion(buf, &ag_object_ver, NULL) == -1)
		goto fail;
	
	data_offs = (off_t)AG_ReadUint32(buf);
	AG_ReadUint32(buf);				/* Skip gfx offs */
	AG_NetbufSeek(buf, data_offs, SEEK_SET);

	if (ob->ops->load != NULL &&
	    ob->ops->load(ob, buf) == -1)
		goto fail;

	ob->flags |= AG_OBJECT_DATA_RESIDENT;
	AG_NetbufClose(buf);
	return (0);
fail:
	AG_NetbufClose(buf);
	return (-1);
}

static void
backup_object(void *p, const char *orig)
{
	char path[MAXPATHLEN];
	struct stat sb;

	if (stat(orig, &sb) == 0) {
		strlcpy(path, orig, sizeof(path));
		strlcat(path, ".bak", sizeof(path));
		rename(orig, path);
	}
}

/* Save the state of an object and its children. */
int
AG_ObjectSaveAll(void *p)
{
	AG_Object *obj = p, *cobj;

	AG_LockLinkage();
	if (AG_ObjectSave(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		if (cobj->flags & AG_OBJECT_NON_PERSISTENT) {
			continue;
		}
		if (AG_ObjectSaveAll(cobj) == -1)
			goto fail;
	}
	AG_UnlockLinkage();
	return (0);
fail:
	AG_UnlockLinkage();
	return (-1);
}

/* Save the state of an object. */
int
AG_ObjectSave(void *p)
{
	char save_path[MAXPATHLEN];
	char save_dir[MAXPATHLEN];
	char save_file[MAXPATHLEN];
	char obj_name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	struct stat sta;
	AG_Netbuf *buf;
	AG_Object *child;
	off_t count_offs, data_offs, gfx_offs;
	Uint32 count;
	AG_ObjectDep *dep;
	int was_resident;

	AG_LockLinkage();
	pthread_mutex_lock(&ob->lock);

	if (ob->flags & AG_OBJECT_NON_PERSISTENT) {
		AG_SetError(_("The `%s' object is non-persistent."), ob->name);
		goto fail_lock;
	}
	was_resident = ob->flags & AG_OBJECT_DATA_RESIDENT;
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));
	
	/* Create the save directory. */
	AG_StringCopy(agConfig, "save-path", save_path, sizeof(save_path));
	strlcpy(save_dir, save_path, sizeof(save_dir));
	if (ob->save_pfx != NULL) {
		strlcat(save_dir, ob->save_pfx, sizeof(save_dir));
	}
	strlcat(save_dir, obj_name, sizeof(save_dir));
	if (stat(save_dir, &sta) == -1 &&
	    mkpath(save_dir) == -1) {
		AG_SetError("mkpath %s: %s", save_dir, strerror(errno));
		goto fail_lock;
	}

	/* Page in the data unless it is already resident. */
	if (!was_resident) {
		if (AG_ObjectLoadData(ob) == -1) {
			/*
			 * Assume that this failure means the data has never
			 * been saved before.
			 * XXX
			 */
			dprintf("%s: %s\n", ob->name, AG_GetError());
			ob->flags |= AG_OBJECT_DATA_RESIDENT;
		}
	}

	strlcpy(save_file, save_dir, sizeof(save_file));
	strlcat(save_file, "/", sizeof(save_file));
	strlcat(save_file, ob->name, sizeof(save_file));
	strlcat(save_file, ".", sizeof(save_file));
	strlcat(save_file, ob->type, sizeof(save_file));

	debug(DEBUG_STATE, "saving %s to %s\n", ob->name, save_file);

	backup_object(ob, save_file);

	if ((buf = AG_NetbufOpen(save_file, "wb", AG_NETBUF_BIG_ENDIAN)) == NULL)
		goto fail_reinit;

	AG_WriteVersion(buf, &ag_object_ver);

	data_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	gfx_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);

	AG_WriteUint32(buf, (Uint32)(ob->flags & AG_OBJECT_SAVED_FLAGS));

	/* Encode the object dependencies. */
	count_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	for (dep = TAILQ_FIRST(&ob->deps), count = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), count++) {
		char dep_name[AG_OBJECT_PATH_MAX];
		
		AG_ObjectCopyName(dep->obj, dep_name, sizeof(dep_name));
		AG_WriteString(buf, dep_name);
	}
	AG_PwriteUint32(buf, count, count_offs);

	/* Encode the generic properties. */
	if (AG_PropSave(ob, buf) == -1)
		goto fail;
	
	/* Save the list of child objects. */
	count_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	count = 0;
	TAILQ_FOREACH(child, &ob->children, cobjs) {
		if (child->flags & AG_OBJECT_NON_PERSISTENT) {
			continue;
		}
		AG_WriteString(buf, child->name);
		AG_WriteString(buf, child->type);
		count++;
	}
	AG_PwriteUint32(buf, count, count_offs);

	/* Save the object data. */
	AG_PwriteUint32(buf, AG_NetbufTell(buf), data_offs);
	if (ob->ops->save != NULL &&
	    ob->ops->save(ob, buf) == -1)
		goto fail;

	/* Save the object graphics. */
	AG_PwriteUint32(buf, AG_NetbufTell(buf), gfx_offs);
	if (AG_GfxSave(ob, buf) == -1)
		goto fail;

	AG_NetbufFlush(buf);
	AG_NetbufClose(buf);
	if (!was_resident) {
		AG_ObjectFreeData(ob);
	}
	pthread_mutex_unlock(&ob->lock);
	AG_UnlockLinkage();
	return (0);
fail:
	AG_NetbufClose(buf);
fail_reinit:
	if (!was_resident)
		AG_ObjectFreeData(ob);
fail_lock:
	pthread_mutex_unlock(&ob->lock);
	AG_UnlockLinkage();
	return (-1);
}

/* Override an object's type; thread unsafe. */
void
AG_ObjectSetType(void *p, const char *type)
{
	AG_Object *ob = p;

	strlcpy(ob->type, type, sizeof(ob->type));
}

/* Override an object's name; thread unsafe. */
void
AG_ObjectSetName(void *p, const char *name)
{
	AG_Object *ob = p;
	char *c;

	strlcpy(ob->name, name, sizeof(ob->name));

	for (c = &ob->name[0]; *c != '\0'; c++) {
		if (*c == '/')			/* Pathname separator */
			*c = '_';
	}
}

/* Override an object's ops; thread unsafe. */
void
AG_ObjectSetOps(void *p, const void *ops)
{
	AGOBJECT(p)->ops = ops;
}

/* Add a new dependency or increment the reference count on one. */
AG_ObjectDep *
AG_ObjectAddDep(void *p, void *depobj)
{
	AG_Object *ob = p;
	AG_ObjectDep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == depobj)
			break;
	}
	if (dep != NULL) {
		debug(DEBUG_DEPS, "%s: [%s/%u]\n", ob->name,
		    AGOBJECT(depobj)->name, dep->count);
		if (++dep->count > AG_OBJECT_DEP_MAX) {
			fprintf(stderr, "%s: wiring %s dep (too many refs)\n",
			    ob->name, AGOBJECT(depobj)->name);
			dep->count = AG_OBJECT_DEP_MAX;
		}
	} else {
		debug(DEBUG_DEPS, "%s: +[%s]\n", ob->name,
		    AGOBJECT(depobj)->name);
		dep = Malloc(sizeof(AG_ObjectDep), M_DEP);
		dep->obj = depobj;
		dep->count = 1;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
	}
	return (dep);
}

/* Resolve a given dependency. */
int
AG_ObjectFindDep(const void *p, Uint32 ind, void **objp)
{
	const AG_Object *ob = p;
	AG_ObjectDep *dep;
	Uint32 i;

	if (ind == 0) {
		*objp = NULL;
		return (0);
	} else if (ind == 1) {
		*objp = (void *)ob;
		return (0);
	}

	for (dep = TAILQ_FIRST(&ob->deps), i = 2;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), i++) {
		if (i == ind)
			break;
	}
	if (dep != NULL) {
		*objp = dep->obj;
		return (0);
	}

	AG_SetError(_("Unable to resolve dependency %s:%u."), ob->name, ind);
	return (-1);
}

/*
 * Encode an object dependency. The values 0 and 1 are reserved, 0 is a
 * NULL value and 1 is the parent object itself.
 */
Uint32
AG_ObjectEncodeName(const void *p, const void *depobjp)
{
	const AG_Object *ob = p;
	const AG_Object *depobj = depobjp;
	AG_ObjectDep *dep;
	Uint32 i;

	if (depobjp == NULL) {
		return (0);
	} else if (p == depobjp) {
		return (1);
	}
	for (dep = TAILQ_FIRST(&ob->deps), i = 2;
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
AG_ObjectDelDep(void *p, const void *depobj)
{
	AG_Object *ob = p;
	AG_ObjectDep *dep;
	
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == depobj)
			break;
	}
	if (dep == NULL) {
		dprintf("%s: no such dep: %s\n", ob->name,
		    AGOBJECT(depobj)->name);
		return;
	}

	if (dep->count == AG_OBJECT_DEP_MAX)			/* Wired */
		return;

	if ((dep->count-1) == 0) {
		if ((ob->flags & AG_OBJECT_PRESERVE_DEPS) == 0) {
			debug(DEBUG_DEPS, "%s: -[%s]\n", ob->name,
			    AGOBJECT(depobj)->name);
			TAILQ_REMOVE(&ob->deps, dep, deps);
			Free(dep, M_DEP);
		} else {
			dep->count = 0;
		}
	} else if (dep->count == 0) {
		fatal("neg ref count");
	} else {
		debug(DEBUG_DEPS, "%s: [%s/%u]\n", ob->name,
		    AGOBJECT(depobj)->name, dep->count);
		dep->count--;
	}
}

/* Move an object towards the head of its parent's children list. */
void
AG_ObjectMoveUp(void *p)
{
	AG_Object *ob = p, *prev;
	AG_Object *parent = ob->parent;

	if (parent == NULL || ob == TAILQ_FIRST(&parent->children))
		return;

	prev = TAILQ_PREV(ob, ag_objectq, cobjs);
	TAILQ_REMOVE(&parent->children, ob, cobjs);
	TAILQ_INSERT_BEFORE(prev, ob, cobjs);
}

/* Move an object towards the tail of its parent's children list. */
void
AG_ObjectMoveDown(void *p)
{
	AG_Object *ob = p;
	AG_Object *parent = ob->parent;
	AG_Object *next = TAILQ_NEXT(ob, cobjs);

	if (parent == NULL || next == NULL)
		return;

	TAILQ_REMOVE(&parent->children, ob, cobjs);
	TAILQ_INSERT_AFTER(&parent->children, next, ob, cobjs);
}

/* Make sure that an object's name is unique. */
static void
AG_ObjectRenameUnique(AG_Object *obj)
{
	AG_Object *oob, *oparent = obj->parent;
	char basename[AG_OBJECT_NAME_MAX];
	char newname[AG_OBJECT_NAME_MAX];
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
 * Change the save prefix of an object and its children.
 * The linkage must be locked.
 */
void
AG_ObjectSetSavePfx(void *p, char *path)
{
	AG_Object *ob = p, *cob;

	ob->save_pfx = path;
	TAILQ_FOREACH(cob, &ob->children, cobjs)
		AG_ObjectSetSavePfx(cob, path);
}

/*
 * Remove the data files of an object and its children.
 * The linkage must be locked.
 */
void
AG_ObjectUnlinkDatafiles(void *p)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p, *cob;

	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == 0)
		unlink(path);

	TAILQ_FOREACH(cob, &ob->children, cobjs)
		AG_ObjectUnlinkDatafiles(cob);

	if (AG_ObjectCopyDirname(ob, path, sizeof(path)) == 0)
		rmdir(path);
}

/* Duplicate an object and its children. */
/* XXX EXPERIMENTAL */
void *
AG_ObjectDuplicate(void *p)
{
	char oname[AG_OBJECT_NAME_MAX];
	AG_Object *ob = p;
	AG_Object *dob;
	AG_ObjectType *t;

	for (t = &agTypes[0]; t < &agTypes[agnTypes]; t++)
		if (strcmp(ob->type, t->type) == 0)
			break;
#ifdef DEBUG
	if (t == &agTypes[agnTypes])
		fatal("unrecognized object type");
#endif
	dob = Malloc(t->size, M_OBJECT);

	pthread_mutex_lock(&ob->lock);

	/* Create the duplicate object. */
	if (t->ops->init != NULL) {
		t->ops->init(dob, ob->name);
	} else {
		AG_ObjectInit(dob, ob->type, ob->name, t->ops);
	}

	if (AG_ObjectPageIn(ob, AG_OBJECT_DATA) == -1)
		goto fail;

	/* Change the name and attach to the same parent as the original. */
	AG_ObjectAttach(ob->parent, dob);
	AG_ObjectRenameUnique(dob);
	dob->flags = (ob->flags & AG_OBJECT_DUPED_FLAGS);

	/* Save the state of the original object using the new name. */
	strlcpy(oname, ob->name, sizeof(oname));
	strlcpy(ob->name, dob->name, sizeof(ob->name));
	if (AG_ObjectSave(ob) == -1) {
		AG_ObjectPageOut(ob, AG_OBJECT_DATA);
		goto fail;
	}

	if (AG_ObjectPageOut(ob, AG_OBJECT_DATA) == -1)
		goto fail;

	if (AG_ObjectLoad(dob) == -1)
		goto fail;

	strlcpy(ob->name, oname, sizeof(ob->name));
	pthread_mutex_unlock(&ob->lock);
	return (dob);
fail:
	strlcpy(ob->name, oname, sizeof(ob->name));
	pthread_mutex_unlock(&ob->lock);
	AG_ObjectDestroy(dob);
	Free(dob, M_OBJECT);
	return (NULL);
}

/* Return the icon associated with this object type, if any. */
SDL_Surface *
AG_ObjectIcon(void *p)
{
	AG_Object *obj = p;
	int i;
	
	if (obj == NULL) {
		return (NULL);
	}
	for (i = 0; i < agnTypes; i++) {
		if (strcmp(agTypes[i].type, obj->type) == 0)
			return (agTypes[i].icon >= 0 ? AGICON(agTypes[i].icon) :
			    NULL);
	}
	return (NULL);
}

/* Return a cryptographic digest for an object's last saved state. */
size_t
AG_ObjectCopyChecksum(const void *p, enum ag_object_checksum_alg alg,
    char *digest)
{
	const AG_Object *ob = p;
	char save_path[MAXPATHLEN];
	u_char buf[BUFSIZ];
	FILE *f;
	off_t offs;
	size_t totlen = 0;
	size_t rv;
	
	if (AG_ObjectCopyFilename(ob, save_path, sizeof(save_path)) == -1) {
		return (0);
	}
	/* TODO locking */
	if ((f = fopen(save_path, "r")) == NULL) {
		AG_SetError("%s: %s", save_path, strerror(errno));
		return (0);
	}

	switch (alg) {
	case AG_OBJECT_MD5:
		{
			MD5_CTX ctx;

			MD5Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				MD5Update(&ctx, buf, (u_int)rv);
				totlen += rv;
			}
			MD5End(&ctx, digest);
		}
		break;
	case AG_OBJECT_SHA1:
		{
			SHA1_CTX ctx;

			SHA1Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				SHA1Update(&ctx, buf, (u_int)rv);
				totlen += rv;
			}
			SHA1End(&ctx, digest);
		}
		break;
	case AG_OBJECT_RMD160:
		{
			RMD160_CTX ctx;

			RMD160Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				RMD160Update(&ctx, buf, (u_int)rv);
				totlen += rv;
			}
			RMD160End(&ctx, digest);
		}
		break;
	}
	fclose(f);

	return (totlen);
}

int
AG_ObjectCopyDigest(const void *ob, size_t *len, char *digest)
{
	char md5[MD5_DIGEST_STRING_LENGTH];
	char sha1[SHA1_DIGEST_STRING_LENGTH];
	char rmd160[RMD160_DIGEST_STRING_LENGTH];

	if ((*len = AG_ObjectCopyChecksum(ob, AG_OBJECT_MD5, md5)) == 0 ||
	    AG_ObjectCopyChecksum(ob, AG_OBJECT_SHA1, sha1) == 0 ||
	    AG_ObjectCopyChecksum(ob, AG_OBJECT_RMD160, rmd160) == 0) {
		return (-1);
	}
	if (snprintf(digest, AG_OBJECT_DIGEST_MAX, "(md5|%s sha1|%s rmd160|%s)",
	    md5, sha1, rmd160) >= AG_OBJECT_DIGEST_MAX) {
		AG_SetError("Digest is too big.");
		return (-1);
	}
	return (0);
}

/* Check whether the given object or any of its children has changed. */
int
AG_ObjectChangedAll(void *p)
{
	AG_Object *ob = p, *cob;

	if (AG_ObjectChanged(ob) == 1) {
		dprintf("%s: modified\n", ob->name);
		return (1);
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (AG_ObjectChangedAll(cob) == 1)
			return (1);
	}
	return (0);
}

/* Check whether the given object has changed since last saved. */
int
AG_ObjectChanged(void *p)
{
	char save_sha1[SHA1_DIGEST_STRING_LENGTH];
	char tmp_sha1[SHA1_DIGEST_STRING_LENGTH];
	AG_Object *ob = p;
	char *pfx_save;
	int rv;
#ifdef DEBUG
	extern int agObjMgrHexDiff;
#endif

	if ((ob->flags & AG_OBJECT_NON_PERSISTENT) ||
	    (ob->flags & AG_OBJECT_DATA_RESIDENT) == 0) {
		return (0);
	}
	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_SHA1, save_sha1) == 0)
		return (1);

	pfx_save = ob->save_pfx;
	AG_ObjectSetSavePfx(ob, "/.tmp");
	if (AG_ObjectSave(ob) == -1) {
		fprintf(stderr, "save %s: %s\n", ob->name, AG_GetError());
		goto fail;
	}
	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_SHA1, tmp_sha1) == -1) {
		fprintf(stderr, "md5 %s: %s\n", ob->name, AG_GetError());
		goto fail;
	}
	rv = (strcmp(save_sha1, tmp_sha1) != 0);

#ifdef DEBUG
	if (rv == 1 && agObjMgrHexDiff) {
		char path[MAXPATHLEN];
		char tmp[MAXPATHLEN];
		char cmd[1024];

		AG_StringCopy(agConfig, "save-path", tmp, sizeof(tmp));
		strlcat(tmp, "/.tmp/hexdiff", sizeof(tmp));

		AG_ObjectCopyFilename(ob, path, sizeof(path));
		snprintf(cmd, sizeof(cmd), "hexdump -C '%s'>%s", path, tmp);
		system(cmd);
		
		AG_ObjectSetSavePfx(ob, pfx_save);
		AG_ObjectCopyFilename(ob, path, sizeof(path));
		snprintf(cmd, sizeof(cmd), "hexdump -C '%s'|diff -u - %s",
		    path, tmp);
		system(cmd);
		AG_ObjectSetSavePfx(ob, "/.tmp");
	}
#endif /* DEBUG */

	AG_ObjectUnlinkDatafiles(ob);
	AG_ObjectSetSavePfx(ob, pfx_save);
	return (rv);
fail:
	AG_ObjectUnlinkDatafiles(ob);
	AG_ObjectSetSavePfx(ob, pfx_save);
	return (-1);
}

#ifdef EDITION

static void
poll_deps(int argc, union evarg *argv)
{
	char path[AG_OBJECT_PATH_MAX];
	AG_Tlist *tl = argv[0].p;
	AG_Object *ob = argv[1].p;
	AG_ObjectDep *dep;

	AG_TlistClear(tl);
	AG_LockLinkage();
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		char label[AG_TLIST_LABEL_MAX];
	
		if (dep->obj != NULL) {
			AG_ObjectCopyName(dep->obj, path, sizeof(path));
		} else {
			strlcpy(path, "(NULL)", sizeof(path));
		}
		if (dep->count == AG_OBJECT_DEP_MAX) {
			snprintf(label, sizeof(label), "%s (wired)", path);
		} else {
			snprintf(label, sizeof(label), "%s (%u)", path,
			    dep->count);
		}
		AG_TlistAddPtr(tl, AG_ObjectIcon(dep->obj), label, dep);
	}
	AG_UnlockLinkage();
	AG_TlistRestore(tl);
}

static void
poll_gfx(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_Object *ob = argv[1].p;
	AG_Gfx *gfx = ob->gfx;
	AG_TlistItem *it;
	Uint32 i;

	if (gfx == NULL)
		return;
	
	AG_TlistClear(tl);
	AG_TlistAdd(tl, NULL, "(%u references)", gfx->used);
	for (i = 0; i < gfx->nsprites; i++) {
		AG_Sprite *spr = &gfx->sprites[i];
		SDL_Surface *su = spr->su;
		AG_CachedSprite *csp;

		if (su != NULL) {
			it = AG_TlistAdd(tl, su, "%u. %s - %ux%ux%u (%s)", i,
			    spr->name, su->w, su->h, su->format->BitsPerPixel,
			    agGfxSnapNames[spr->snap_mode]);
		} else {
			it = AG_TlistAdd(tl, su, "%u. (null)", i);
		}
		it->p1 = spr;
		it->depth = 0;

		if (!SLIST_EMPTY(&spr->csprites)) {
			it->flags |= AG_TLIST_HAS_CHILDREN;
		}
		SLIST_FOREACH(csp, &spr->csprites, sprites) {
			char label[AG_TLIST_LABEL_MAX];
			AG_TlistItem *it;

			snprintf(label, sizeof(label), "%u ticks\n",
			    csp->last_drawn);
			AG_TransformPrint(&csp->transforms, label,
			    sizeof(label));

			it = AG_TlistAddPtr(tl, csp->su, label, csp);
			it->depth = 1;
		}
	}
	AG_TlistRestore(tl);
}

static void
poll_props(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_Object *ob = argv[1].p;
	AG_Prop *prop;
	
	AG_TlistClear(tl);
	TAILQ_FOREACH(prop, &ob->props, props) {
		char val[AG_TLIST_LABEL_MAX];

		AG_PropPrint(val, sizeof(val), prop);
		AG_TlistAdd(tl, NULL, "%s = %s", prop->key, val);
	}
	AG_TlistRestore(tl);
}

static void
poll_events(int argc, union evarg *argv)
{
	extern const char *evarg_type_names[];
	AG_Tlist *tl = argv[0].p;
	AG_Object *ob = argv[1].p;
	AG_Event *ev;
	
	AG_TlistClear(tl);
	TAILQ_FOREACH(ev, &ob->events, events) {
		char args[AG_TLIST_LABEL_MAX], arg[16];
		u_int i;

		args[0] = '(';
		args[1] = '\0';
		for (i = 1; i < ev->argc; i++) {
			switch (ev->argt[i]) {
			case AG_EVARG_POINTER:
				snprintf(arg, sizeof(arg), "%p", ev->argv[i].p);
				break;
			case AG_EVARG_STRING:
				snprintf(arg, sizeof(arg), "\"%s\"",
				    ev->argv[i].s);
				break;
			case AG_EVARG_UCHAR:
			case AG_EVARG_CHAR:
				snprintf(arg, sizeof(arg), "'%c'",
				    (u_char)ev->argv[i].i);
				break;
			case AG_EVARG_INT:
				snprintf(arg, sizeof(arg), "%d", ev->argv[i].i);
				break;
			case AG_EVARG_UINT:
				snprintf(arg, sizeof(arg), "%u",
				    (u_int)ev->argv[i].i);
				break;
			case AG_EVARG_LONG:
				snprintf(arg, sizeof(arg), "%li",
				    ev->argv[i].li);
				break;
			case AG_EVARG_ULONG:
				snprintf(arg, sizeof(arg), "%li",
				    (u_long)ev->argv[i].li);
				break;
			case AG_EVARG_FLOAT:
				snprintf(arg, sizeof(arg), "<%g>",
				    ev->argv[i].f);
				break;
			case AG_EVARG_DOUBLE:
				snprintf(arg, sizeof(arg), "<%g>",
				    ev->argv[i].f);
				break;
			}
			strlcat(args, arg, sizeof(args));
			if (i < ev->argc-1) {
				strlcat(args, ", ", sizeof(args));
			} else {
				strlcat(args, ")", sizeof(args));
			}
		}

		AG_TlistAdd(tl, NULL, "%s%s%s %s", ev,
		    (ev->flags & AG_EVENT_ASYNC) ? " <async>" : "",
		    (ev->flags & AG_EVENT_PROPAGATE) ? " <propagate>" : "",
		    args);

	}
	AG_TlistRestore(tl);
}

static void
rename_object(int argc, union evarg *argv)
{
	AG_WidgetBinding *stringb;
	AG_Textbox *tb = argv[0].p;
	AG_Object *ob = argv[1].p;

	AG_ObjectPageIn(ob, AG_OBJECT_DATA);
	AG_ObjectUnlinkDatafiles(ob);
	strlcpy(ob->name, tb->string, sizeof(ob->name));
	AG_ObjectPageOut(ob, AG_OBJECT_DATA);

	AG_PostEvent(NULL, ob, "renamed", NULL);
}

static void
refresh_checksums(int argc, union evarg *argv)
{
	char checksum[128];
	AG_Object *ob = argv[1].p;
	AG_Textbox *tb_md5 = argv[2].p;
	AG_Textbox *tb_sha1 = argv[3].p;
	AG_Textbox *tb_rmd160 = argv[4].p;

	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_MD5, checksum) > 0) {
		AG_TextboxPrintf(tb_md5,  "%s", checksum);
	} else {
		AG_TextboxPrintf(tb_md5,  "(%s)", AG_GetError());
	}
	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_SHA1, checksum) > 0) {
		AG_TextboxPrintf(tb_sha1,  "%s", checksum);
	} else {
		AG_TextboxPrintf(tb_sha1,  "(%s)", AG_GetError());
	}
	if (AG_ObjectCopyChecksum(ob, AG_OBJECT_RMD160, checksum) > 0) {
		AG_TextboxPrintf(tb_rmd160,  "%s", checksum);
	} else {
		AG_TextboxPrintf(tb_rmd160,  "(%s)", AG_GetError());
	}
}

#ifdef NETWORK
static void
refresh_rcs_status(int argc, union evarg *argv)
{
	char objdir[AG_OBJECT_PATH_MAX];
	char digest[AG_OBJECT_DIGEST_MAX];
	AG_Object *ob = argv[1].p;
	AG_Label *lb_status = argv[2].p;
	AG_Tlist *tl = argv[3].p;
	extern const char *agRcsStatusStrings[];
	enum ag_rcs_status status;
	size_t len;
	u_int working_rev, repo_rev;

	if (AG_ObjectCopyName(ob, objdir, sizeof(objdir)) == -1 ||
	    AG_ObjectCopyDigest(ob, &len, digest) == -1) {
		return;
	}
	if (AG_RcsConnect() == -1) {
		return;
	}
	status = AG_RcsStatus(ob, objdir, digest, NULL, NULL, &repo_rev,
	    &working_rev);
	AG_LabelPrintf(lb_status,
	    _("RCS status: %s\n"
	      "Working revision: #%u\n"
	      "Repository revision: #%u\n"),
	    agRcsStatusStrings[status],
	    (status != AG_RCS_UNKNOWN && status != AG_RCS_ERROR) ? working_rev : 0,
	    (status != AG_RCS_UNKNOWN && status != AG_RCS_ERROR) ? repo_rev: 0);

	AG_TlistClear(tl);
	AG_RcsLog(objdir, tl);
	AG_TlistRestore(tl);

	AG_RcsDisconnect();
}

#endif /* NETWORK */

void *
AG_ObjectEdit(void *p)
{
	AG_Object *ob = p;
	AG_Window *win;
	AG_Textbox *tbox;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Tlist *tl;
	AG_Box *box;
	AG_Button *btn;

	win = AG_WindowNew(AG_WINDOW_DETACH, NULL);
	AG_WindowSetCaption(win, _("Object %s"), ob->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_RIGHT, 1);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAddTab(nb, _("Infos"), AG_BOX_VERT);
	{
		char path[AG_OBJECT_PATH_MAX];
		AG_Textbox *tb_md5, *tb_sha1, *tb_rmd160;

		tbox = AG_TextboxNew(ntab, _("Name: "));
		AG_TextboxPrintf(tbox, ob->name);
		AG_SetEvent(tbox, "textbox-return", rename_object, "%p", ob);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
	
		AG_LabelNew(ntab, AG_LABEL_STATIC, _("Type: %s"), ob->type);
		AG_LabelNew(ntab, AG_LABEL_POLLED, _("Flags: 0x%x"),
		    &ob->flags);
		AG_LabelNew(ntab, AG_LABEL_POLLED_MT, _("Parent: %[obj]"),
		    &agLinkageLock, &ob->parent);
		AG_LabelNew(ntab, AG_LABEL_STATIC, _("Save prefix: %s"),
		    ob->save_pfx != NULL ? ob->save_pfx : "/");

		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		AG_LabelNew(ntab, AG_LABEL_POLLED, _("Data references: %[u32]"),
		    &ob->data_used);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		tb_md5 = AG_TextboxNew(ntab, "MD5: ");
		AG_TextboxPrescale(tb_md5, "888888888888888888888888888888888");
		tb_md5->flags &= ~(AG_TEXTBOX_WRITEABLE);
		
		tb_sha1 = AG_TextboxNew(ntab, "SHA1: ");
		AG_TextboxPrescale(tb_sha1, "8888888888888888888888888");
		tb_sha1->flags &= ~(AG_TEXTBOX_WRITEABLE);
		
		tb_rmd160 = AG_TextboxNew(ntab, "RMD160: ");
		AG_TextboxPrescale(tb_rmd160, "88888888888888888888888");
		tb_rmd160->flags &= ~(AG_TEXTBOX_WRITEABLE);

		box = AG_BoxNew(ntab, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|
					            AG_BOX_WFILL);
		{
			btn = AG_ButtonAct(box, _("Refresh checksums"), 0,
			    refresh_checksums, "%p,%p,%p,%p", ob, tb_md5,
			    tb_sha1, tb_rmd160);
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
		}
	}

#ifdef NETWORK
	ntab = AG_NotebookAddTab(nb, _("RCS"), AG_BOX_VERT);
	{
		AG_Label *lb_status;
		AG_Tlist *tl;

		lb_status = AG_LabelNew(ntab, AG_LABEL_STATIC, "...");

		AG_LabelNew(ntab, AG_LABEL_STATIC, _("Revision history:"));
		tl = AG_TlistNew(ntab, 0);

		btn = AG_ButtonAct(ntab, _("Refresh status"), AG_BUTTON_WFILL,
		    refresh_rcs_status, "%p,%p,%p", ob, lb_status, tl);

		if (agRcsMode)
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
	}
#endif /* NETWORK */

	ntab = AG_NotebookAddTab(nb, _("Deps"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL);
		AG_TlistPrescale(tl, "XXXXXXXXXXXX", 6);
		AG_SetEvent(tl, "tlist-poll", poll_deps, "%p", ob);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Graphics"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL);
		AG_TlistSetItemHeight(tl, AGTILESZ);
		AG_SetEvent(tl, "tlist-poll", poll_gfx, "%p", ob);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Events"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL);
		AG_SetEvent(tl, "tlist-poll", poll_events, "%p", ob);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Properties"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL);
		AG_SetEvent(tl, "tlist-poll", poll_props, "%p", ob);
	}
	return (win);
}

#endif /* EDITION */
