/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Implementation of the generic Object class.
 */

#include <core/core.h>
#include <core/md5.h>
#include <core/sha1.h>
#include <core/rmd160.h>
#include <core/dir.h>
#include <core/file.h>
#include <core/config.h>
#include <core/typesw.h>

#ifdef NETWORK
#include <core/rcs.h>
#endif

#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

const AG_ObjectOps agObjectOps = {
	"AG_Object",
	sizeof(AG_Object),
	{ 7, 1 },
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

int agObjectIgnoreDataErrors = 0;  /* Don't fail on a data load failure. */
int agObjectIgnoreUnknownObjs = 0; /* Don't fail on unknown object types. */
int agObjectBackups = 1;	   /* Backup object save files. */

void
AG_ObjectInit(void *p, const char *name, const void *opsp)
{
	AG_Object *ob = p;
	char *c;

	strlcpy(ob->name, name, sizeof(ob->name));

	/* Prevent ambiguous characters in the name. */
	for (c = ob->name; *c != '\0'; c++) {
		if (*c == '/' || *c == '.' || *c == ':' || *c == ',')
			*c = '_';
	}

	ob->save_pfx = "/world";
	ob->archivePath = NULL;
	ob->ops = (opsp != NULL) ? opsp : &agObjectOps;
	ob->parent = NULL;
	ob->flags = 0;
	ob->nevents = 0;

	AG_MutexInitRecursive(&ob->lock);
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->children);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	CIRCLEQ_INIT(&ob->timeouts);
}

/* Create a new object instance and mark it resident. */
void *
AG_ObjectNew(void *parent, const char *name, const AG_ObjectOps *ops)
{
	char nameGen[AG_OBJECT_NAME_MAX];
	AG_Object *obj;

	if (name == NULL) {
		AG_ObjectGenName(parent, ops, nameGen, sizeof(nameGen));
	} else {
		if (parent != NULL &&
		    AG_ObjectFindChild(parent, name) != NULL) {
			AG_SetError(_("%s: Existing child object %s"),
			    OBJECT(parent)->name, name);
			return (NULL);
		}
	}
	
	obj = Malloc(ops->size, M_OBJECT);
	if (ops->init != NULL) {
		ops->init(obj, name != NULL ? name : nameGen);
	} else {
		AG_ObjectInit(obj, name != NULL ? name : nameGen, ops);
	}
	obj->flags |= AG_OBJECT_RESIDENT;

	if (parent != NULL) {
		AG_ObjectAttach(parent, obj);
	}
	return (obj);
}

/* Check if an object's class matches the given pattern (general case). */
int
AG_ObjectIsClassGeneral(const AG_Object *obj, const char *cn)
{
	char cname[AG_OBJECT_TYPE_MAX], *cp, *c;
	char nname[AG_OBJECT_TYPE_MAX], *np, *s;

	strlcpy(cname, cn, sizeof(cname));
	strlcpy(nname, obj->ops->type, sizeof(nname));
	cp = cname;
	np = nname;
	while ((c = AG_Strsep(&cp, ":")) != NULL &&
	       (s = AG_Strsep(&np, ":")) != NULL) {
		if (c[0] == '*' && c[1] == '\0')
			continue;
		if (strcmp(c, s) != 0)
			return (0);
	}
	return (1);
}

void
AG_ObjectRemain(void *p, int flags)
{
	AG_Object *ob = p;

	if (flags & AG_OBJECT_REMAIN_DATA) {
		ob->flags |= (AG_OBJECT_REMAIN_DATA|AG_OBJECT_RESIDENT);
	} else {
		ob->flags &= ~AG_OBJECT_REMAIN_DATA;
	}
}

/*
 * Free an object's dataset. Dependencies are preserved, but they are all
 * assumed to have reference counts of 0.
 */
void
AG_ObjectFreeDataset(void *p)
{
	AG_Object *ob = p;

	AG_MutexLock(&ob->lock);
	if (OBJECT_RESIDENT(ob)) {
		if (ob->ops->free_dataset != NULL) {
			ob->flags |= AG_OBJECT_PRESERVE_DEPS;
			ob->ops->free_dataset(ob);
			ob->flags &= ~(AG_OBJECT_PRESERVE_DEPS);
		}
		ob->flags &= ~(AG_OBJECT_RESIDENT);
	}
	AG_MutexUnlock(&ob->lock);
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
 * Search an object and its children for a dependency upon robj.
 * The linkage must be locked.
 */
static int
AG_ObjectInUseFind(const void *p, const void *robj)
{
	const AG_Object *ob = p, *cob;
	AG_ObjectDep *dep;

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == robj &&
		    robj != ob) {
			AG_SetError(_("The `%s' object is used by `%s'."),
			    OBJECT(robj)->name, ob->name);
			return (1);
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (AG_ObjectInUseFind(cob, robj))
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
	if (AG_ObjectInUseFind(root, ob))
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
AG_ObjectAttach(void *parentp, void *pChld)
{
	AG_Object *parent = parentp;
	AG_Object *chld = pChld;

	if (parent == NULL)
		return;

	AG_LockLinkage();
	
	if (chld->flags & AG_OBJECT_NAME_ONATTACH) {
		AG_ObjectGenName(parent, chld->ops,
		    chld->name, sizeof(chld->name));
	}
	TAILQ_INSERT_TAIL(&parent->children, chld, cobjs);
	chld->parent = parent;
	AG_PostEvent(parent, chld, "attached", NULL);
	AG_PostEvent(chld, parent, "child-attached", NULL);

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
			    OBJECT(child)->name, ppath, AG_GetError());
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
	AG_MutexLock(&child->lock);
	AG_ObjectCancelTimeouts(child, AG_CANCEL_ONDETACH);

	TAILQ_REMOVE(&parent->children, child, cobjs);
	child->parent = NULL;
	AG_PostEvent(parent, child, "detached", NULL);
	AG_PostEvent(child, parent, "child-detached", NULL);
	debug(DEBUG_LINKAGE, "%s: detached from %s\n", child->name,
	    parent->name);

	AG_MutexUnlock(&child->lock);
	AG_UnlockLinkage();
}

/* Traverse the object tree using a pathname. */
static void *
AG_ObjectSearchPath(const AG_Object *parent, const char *name)
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
			rv = AG_ObjectSearchPath(child, &s[1]);
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
	rv = AG_ObjectSearchPath(agWorld, &name[1]);
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
	rv = AG_ObjectSearchPath(agWorld, &path[1]);
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
 * Remove any entry in the object's dependency table (and its children's)
 * with a reference count of zero.
 */
void
AG_ObjectFreeDummyDeps(AG_Object *ob)
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
		AG_ObjectFreeDummyDeps(cob);
}

/*
 * Detach the child objects, and free them, assuming that none of them
 * are currently in use.
 */
void
AG_ObjectFreeChildren(AG_Object *pob)
{
	AG_Object *cob, *ncob;

	AG_MutexLock(&pob->lock);
	for (cob = TAILQ_FIRST(&pob->children);
	     cob != TAILQ_END(&pob->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		debug(DEBUG_GC, "%s: freeing %s\n", pob->name, cob->name);
		AG_ObjectDetach(cob);
		AG_ObjectDestroy(cob);
		if ((cob->flags & AG_OBJECT_STATIC) == 0)
			Free(cob, M_OBJECT);
	}
	TAILQ_INIT(&pob->children);
	AG_MutexUnlock(&pob->lock);
}

/* Clear an object's property table. */
void
AG_ObjectFreeProps(AG_Object *ob)
{
	AG_Prop *prop, *nextprop;

	AG_MutexLock(&ob->lock);
	for (prop = TAILQ_FIRST(&ob->props);
	     prop != TAILQ_END(&ob->props);
	     prop = nextprop) {
		nextprop = TAILQ_NEXT(prop, props);
		AG_PropDestroy(prop);
		Free(prop, M_PROP);
	}
	TAILQ_INIT(&ob->props);
	AG_MutexUnlock(&ob->lock);
}

/*
 * Destroy the event handlers registered by an object, cancelling
 * any scheduled execution.
 */
void
AG_ObjectFreeEvents(AG_Object *ob)
{
	AG_Event *eev, *neev;

	AG_MutexLock(&ob->lock);
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
	AG_MutexUnlock(&ob->lock);
}

/* Cancel any scheduled timeout event associated with the object. */
void
AG_ObjectCancelTimeouts(void *p, Uint flags)
{
	AG_Object *ob = p, *tob;
	extern struct ag_objectq agTimeoutObjQ;
	AG_Event *ev;

	AG_LockTiming();
	AG_MutexLock(&ob->lock);

	TAILQ_FOREACH(ev, &ob->events, events) {
		if ((ev->flags & AG_EVENT_SCHEDULED) &&
		    (ev->timeout.flags & flags)) {
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

	AG_MutexUnlock(&ob->lock);
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

#ifdef DEBUG
	if (ob->parent != NULL) {
		fatal("%s attached to %p", ob->name, ob->parent);
	}
	debug(DEBUG_GC, "destroy %s\n", ob->name);
#endif
	AG_ObjectCancelTimeouts(ob, 0);
	AG_ObjectFreeChildren(ob);
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);

	if (ob->ops->destroy != NULL)
		ob->ops->destroy(ob);

	AG_ObjectFreeProps(ob);
	AG_ObjectFreeEvents(ob);
	AG_MutexDestroy(&ob->lock);
	Free(ob->archivePath,0);
}

/* Copy the full pathname to an object's data file to a fixed-size buffer. */
/* NOTE: Will return data into path even upon failure. */
int
AG_ObjectCopyFilename(const void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	const AG_Object *ob = p;
	char *dir;

	if (ob->archivePath != NULL) {
		strlcpy(path, ob->archivePath, path_len);
		return (0);
	}

	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

	for (dir = AG_Strsep(&loadpathp, ":");
	     dir != NULL;
	     dir = AG_Strsep(&loadpathp, ":")) {
	     	strlcpy(path, dir, path_len);
		if (ob->save_pfx != NULL) {
			strlcat(path, ob->save_pfx, path_len);
		}
		strlcat(path, obj_name, path_len);
		strlcat(path, AG_PATHSEP, path_len);
		strlcat(path, ob->name, path_len);
		strlcat(path, ".", path_len);
		strlcat(path, ob->ops->type, path_len);

		if (AG_FileExists(path))
			return (0);
	}
	AG_SetError(_("The %s%s%c%s.%s file is not in load-path."),
	    ob->save_pfx != NULL ? ob->save_pfx : "",
	    obj_name, AG_PATHSEPC, ob->name, ob->ops->type);
	return (-1);
}

/* Copy the full pathname of an object's data dir to a fixed-size buffer. */
int
AG_ObjectCopyDirname(const void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	const AG_Object *ob = p;
	char *dir;

	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

	for (dir = AG_Strsep(&loadpathp, ":");
	     dir != NULL;
	     dir = AG_Strsep(&loadpathp, ":")) {
		char tmp_path[MAXPATHLEN];

	     	strlcpy(tmp_path, dir, sizeof(tmp_path));
		if (ob->save_pfx != NULL) {
			strlcat(tmp_path, ob->save_pfx, sizeof(tmp_path));
		}
		strlcat(tmp_path, obj_name, sizeof(tmp_path));
		if (AG_FileExists(tmp_path)) {
			strlcpy(path, tmp_path, path_len);
			return (0);
		}
	}
	AG_SetError(_("The %s directory is not in load-path."), obj_name);
	return (-1);
}

/*
 * Load an object's data into memory and mark it resident. If the object
 * is either marked non-persistent or no data can be found in storage,
 * just mark the object resident.
 */
int
AG_ObjectPageIn(void *p)
{
	AG_Object *ob = p;
	int dataFound;

	AG_MutexLock(&ob->lock);
	if (!OBJECT_PERSISTENT(ob)) {
		ob->flags |= AG_OBJECT_RESIDENT;
		goto out;
	}
	if (!OBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (dataFound == 0) {
				/*
				 * Data not found in storage, just assume
				 * the object has never been saved before.
				 */
				ob->flags |= AG_OBJECT_RESIDENT;
				goto out;
			} else {
				goto fail;
			}
		}
	}
out:
	AG_MutexUnlock(&ob->lock);
	return (0);
fail:
	AG_MutexUnlock(&ob->lock);
	return (-1);
}

/*
 * If the given object is persistent and no longer referenced, save
 * its data and free it from memory. The object must be resident.
 */
int
AG_ObjectPageOut(void *p)
{
	AG_Object *ob = p;
	
	AG_MutexLock(&ob->lock);
	if (!OBJECT_PERSISTENT(ob)) {
		goto done;
	}
	if (!OBJECT_RESIDENT(ob)) {
		AG_SetError(_("Object is non-resident"));
		goto fail;
	}
	if (!AG_ObjectInUse(ob)) {
		if (AG_FindEventHandler(agWorld, "object-page-out") != NULL) {
			AG_PostEvent(ob, agWorld, "object-page-out", NULL);
		} else {
			if (AG_ObjectSave(ob) == -1)
				goto fail;
		}
		if ((ob->flags & AG_OBJECT_REMAIN_DATA) == 0)
			AG_ObjectFreeDataset(ob);
	}
done:
	AG_MutexUnlock(&ob->lock);
	return (0);
fail:
	AG_MutexUnlock(&ob->lock);
	return (-1);
}

/* Load both the generic part and the dataset of an object from file. */
int
AG_ObjectLoadFromFile(void *p, const char *path)
{
	AG_Object *ob = p;
	int dataFound;

	AG_LockLinkage();
	AG_MutexLock(&ob->lock);
	if (AG_ObjectLoadGenericFromFile(ob, path) == -1 ||
	    AG_ObjectResolveDeps(ob) == -1 ||
	    AG_ObjectLoadDataFromFile(ob, &dataFound, path) == -1) {
		goto fail;
	}
	AG_MutexUnlock(&ob->lock);
	AG_UnlockLinkage();
	return (0);
fail:
	AG_MutexUnlock(&ob->lock);
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
		if (!OBJECT_PERSISTENT(cob)) {
			continue;
		}
		if (AG_ObjectResolveDeps(cob) == -1)
			return (-1);
	}
	return (0);
}

/*
 * Load the generic part of an object from archive. If the archived
 * object has children, create instances for them and load their
 * generic part as well.
 *
 * The object and linkage must be locked.
 */
int
AG_ObjectLoadGenericFromFile(void *p, const char *pPath)
{
	AG_Version ver;
	char path[MAXPATHLEN];
	AG_Object *ob = p;
	AG_ObjectDep *dep;
	AG_DataSource *ds;
	Uint32 count, i;
	Uint flags, flags_save;
	
	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError(_("Object is non-persistent"));
		return (-1);
	}
	AG_LockLinkage();
	AG_MutexLock(&ob->lock);
	AG_ObjectCancelTimeouts(ob, AG_CANCEL_ONLOAD);

	if (pPath != NULL) {
		strlcpy(path, pPath, sizeof(path));
	} else {
		if (ob->archivePath != NULL) {
			strlcpy(path, ob->archivePath, sizeof(path));
		} else {
			if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1)
				goto fail_unlock;
		}
	}
	debug(DEBUG_STATE, "%s: Loading GENERIC from %s\n", ob->name, path);

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		goto fail_unlock;
	}
	if (AG_ReadVersion(ds, agObjectOps.type, &agObjectOps.ver, &ver) == -1)
		goto fail;

	/*
	 * Must free the resident data in order to clear the dependencies.
	 * Sets the WAS_RESIDENT flag to be used at data load stage.
	 */
	if (OBJECT_RESIDENT(ob)) {
		ob->flags |= AG_OBJECT_WAS_RESIDENT;
		AG_ObjectFreeDataset(ob);
	}
	AG_ObjectFreeDeps(ob);

	(void)AG_ReadUint32(ds);				/* Data offs */
	if (ver.minor < 1) { (void)AG_ReadUint32(ds); }		/* Gfx offs */

	/* Read and verify the generic object flags. */
	flags_save = ob->flags;
	flags = (int)AG_ReadUint32(ds);
	if (flags & (AG_OBJECT_NON_PERSISTENT|AG_OBJECT_RESIDENT|
	             AG_OBJECT_WAS_RESIDENT)) {
		AG_SetError("%s: inconsistent flags (0x%08x)", ob->name,
		    flags);
		goto fail;
	}
	ob->flags = flags | (flags_save & AG_OBJECT_WAS_RESIDENT);

	/* Decode the saved dependencies (to be resolved later). */
	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		dep = Malloc(sizeof(AG_ObjectDep), M_DEP);
		dep->path = AG_ReadString(ds);
		dep->obj = NULL;
		dep->count = 0;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
		debug(DEBUG_DEPS, "%s: depends on `%s'\n", ob->name, dep->path);
	}

	/* Decode the generic properties. */
	if (AG_PropLoad(ob, ds) == -1)
		goto fail;

	/* Load the generic part of the archived child objects. */
	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		char cname[AG_OBJECT_NAME_MAX];
		char classID[AG_OBJECT_TYPE_MAX];
		AG_Object *eob, *child;

	 	/* XXX ensure that there are no duplicate names. */
		AG_CopyString(cname, ds, sizeof(cname));
		AG_CopyString(classID, ds, sizeof(classID));

		OBJECT_FOREACH_CHILD(eob, ob, ag_object) {
			if (strcmp(eob->name, cname) == 0) 
				break;
		}
		if (eob != NULL) {
			/*
			 * XXX TODO Allow these cases to be handled by a
			 * special callback function.
			 */
			if (strcmp(eob->ops->type, classID) != 0) {
				fatal("existing object of different type");
			}
			if (!OBJECT_PERSISTENT(eob)) {
				fatal("existing non-persistent object");
			}
			if (AG_ObjectLoadGeneric(eob) == -1) {
				goto fail;
			}
		} else {
			const AG_ObjectOps *cl;

			if ((cl = AG_FindClass(classID)) == NULL) {
				AG_SetError("%s: %s", ob->name, AG_GetError());
				if (agObjectIgnoreUnknownObjs) {
					dprintf("%s; ignoring\n",
					    AG_GetError());
					continue;
				} else {
					goto fail;
				}
				goto fail;
			}

			child = Malloc(cl->size, M_OBJECT);
			if (cl->init != NULL) {
				cl->init(child, cname);
			} else {
				AG_ObjectInit(child, cname, cl);
			}
			AG_ObjectAttach(ob, child);
			if (AG_ObjectLoadGeneric(child) == -1)
				goto fail;
		}
	}
	AG_PostEvent(ob, agWorld, "object-post-load-generic", "%s", path);
	AG_CloseFile(ds);
	AG_MutexUnlock(&ob->lock);
	AG_UnlockLinkage();
	return (0);
fail:
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);
	AG_CloseFile(ds);
fail_unlock:
	AG_MutexUnlock(&ob->lock);
	AG_UnlockLinkage();
	return (-1);
}

/*
 * Load only the data part of an object archive.
 * The object must be locked.
 */
int
AG_ObjectLoadDataFromFile(void *p, int *dataFound, const char *pPath)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p;
	AG_DataSource *ds;
	off_t dataOffs;
	AG_Version ver;
	
	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError(_("Object is non-persistent"));
		return (-1);
	}
	AG_LockLinkage();
	AG_MutexLock(&ob->lock);
	AG_ObjectCancelTimeouts(ob, AG_CANCEL_ONLOAD);

	*dataFound = 1;

	if (pPath != NULL) {
		strlcpy(path, pPath, sizeof(path));
	} else {
		if (ob->archivePath != NULL) {
			strlcpy(path, ob->archivePath, sizeof(path));
		} else {
			if (AG_ObjectCopyFilename(ob, path, sizeof(path))
			    == -1) {
				*dataFound = 0;
				return (-1);
			}
		}
	}
	debug(DEBUG_STATE, "%s: Loading DATA from %s\n", ob->name, path);

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		*dataFound = 0;
		return (-1);
	}
	if (AG_ReadVersion(ds, agObjectOps.type, &agObjectOps.ver, &ver) == -1)
		goto fail;

	dataOffs = (off_t)AG_ReadUint32(ds);		/* User data offset */
	if (ver.minor < 1) { (void)AG_ReadUint32(ds); }	/* Gfx offs */
	AG_Seek(ds, dataOffs, AG_SEEK_SET);

	if (OBJECT_RESIDENT(ob)) {
		AG_ObjectFreeDataset(ob);
	}
	if (ob->ops->load != NULL &&
	    ob->ops->load(ob, ds) == -1)
		goto fail;

	ob->flags |= AG_OBJECT_RESIDENT;
	AG_CloseFile(ds);
	AG_PostEvent(ob, agWorld, "object-post-load-data", "%s", path);
	AG_MutexUnlock(&ob->lock);
	AG_UnlockLinkage();
	return (0);
fail:
	AG_CloseFile(ds);
	return (-1);
}

static void
BackupObjectFile(void *p, const char *orig)
{
	char path[MAXPATHLEN];

	if (AG_FileExists(orig)) {
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
		if (!OBJECT_PERSISTENT(cobj)) {
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

/* Archive an object to a file. */
int
AG_ObjectSaveToFile(void *p, const char *pPath)
{
	char pathDir[MAXPATHLEN];
	char path[MAXPATHLEN];
	char name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	AG_DataSource *ds;
	AG_Object *child;
	off_t countOffs, dataOffs;
	Uint32 count;
	AG_ObjectDep *dep;
	int pagedTemporarily;
	int dataFound;

	AG_LockLinkage();
	AG_MutexLock(&ob->lock);

	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError(_("The `%s' object is non-persistent."), ob->name);
		goto fail_lock;
	}
	AG_ObjectCopyName(ob, name, sizeof(name));

	if (pPath != NULL) {
		strlcpy(path, pPath, sizeof(path));
	} else if (ob->archivePath == NULL) {
		/* Create the save directory if needed. */
		strlcpy(pathDir, AG_String(agConfig,"save-path"),
		    sizeof(pathDir));
		if (ob->save_pfx != NULL) {
			strlcat(pathDir, ob->save_pfx, sizeof(pathDir));
		}
		strlcat(pathDir, name, sizeof(pathDir));
		if (AG_FileExists(pathDir) == 0 &&
		    AG_MkPath(pathDir) == -1)
			goto fail_lock;
	}

	/*
	 * If we are trying to save a non-resident object, page it in
	 * temporarily for the duration of the operation.
	 * 
	 * XXX TODO Allow partial generic and data modifications instead.
	 */
	if (!OBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (!dataFound) {
				/*
				 * Data has not been found, just assume
				 * that this object has never been saved
				 * before and mark it resident.
				 */
				ob->flags |= AG_OBJECT_RESIDENT;
			} else {
				AG_SetError("Failed to load non-resident "
				            "object for save: %s",
				    AG_GetError());
				goto fail_lock;
			}
		}
		pagedTemporarily = 1;
	} else {
		pagedTemporarily = 0;
	}

	if (pPath == NULL) {
		if (ob->archivePath != NULL) {
			strlcpy(path, ob->archivePath, sizeof(path));
		} else {
			strlcpy(path, pathDir, sizeof(path));
			strlcat(path, AG_PATHSEP, sizeof(path));
			strlcat(path, ob->name, sizeof(path));
			strlcat(path, ".", sizeof(path));
			strlcat(path, ob->ops->type, sizeof(path));
		}
	}
	debug(DEBUG_STATE, "%s: Saving to %s\n", ob->name, path);

	if (agObjectBackups) {
		BackupObjectFile(ob, path);
	} else {
		AG_FileDelete(path);
	}
	if ((ds = AG_OpenFile(path, "wb")) == NULL)
		goto fail_reinit;

	AG_WriteVersion(ds, agObjectOps.type, &agObjectOps.ver);

	dataOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);					/* Data offs */
	AG_WriteUint32(ds, (Uint32)(ob->flags & AG_OBJECT_SAVED_FLAGS));

	/* Encode the object dependencies. */
	countOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	for (dep = TAILQ_FIRST(&ob->deps), count = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), count++) {
		char dep_name[AG_OBJECT_PATH_MAX];
		
		AG_ObjectCopyName(dep->obj, dep_name, sizeof(dep_name));
		AG_WriteString(ds, dep_name);
	}
	AG_WriteUint32At(ds, count, countOffs);

	/* Encode the generic properties. */
	if (AG_PropSave(ob, ds) == -1)
		goto fail;
	
	/* Save the list of child objects. */
	countOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	count = 0;
	TAILQ_FOREACH(child, &ob->children, cobjs) {
		if (!OBJECT_PERSISTENT(child)) {
			continue;
		}
		AG_WriteString(ds, child->name);
		AG_WriteString(ds, child->ops->type);
		count++;
	}
	AG_WriteUint32At(ds, count, countOffs);

	/* Save the user data. */
	AG_WriteUint32At(ds, AG_Tell(ds), dataOffs);
	if (ob->ops->save != NULL &&
	    ob->ops->save(ob, ds) == -1)
		goto fail;

	AG_CloseFile(ds);
	if (pagedTemporarily) { AG_ObjectFreeDataset(ob); }
	AG_MutexUnlock(&ob->lock);
	AG_UnlockLinkage();
	return (0);
fail:
	AG_CloseFile(ds);
fail_reinit:
	if (pagedTemporarily) { AG_ObjectFreeDataset(ob); }
fail_lock:
	AG_MutexUnlock(&ob->lock);
	AG_UnlockLinkage();
	return (-1);
}

/* Override an object's name; thread unsafe. */
void
AG_ObjectSetName(void *p, const char *name)
{
	AG_Object *ob = p;
	char *c;

	strlcpy(ob->name, name, sizeof(ob->name));

	for (c = &ob->name[0]; *c != '\0'; c++) {
		if (*c == '/' || *c == '\\')		/* Pathname separator */
			*c = '_';
	}
}

void
AG_ObjectGetArchivePath(void *p, char *buf, size_t buf_len)
{
	AG_Object *ob = p;

	AG_MutexLock(&ob->lock);
	strlcpy(buf, ob->archivePath, buf_len);
	AG_MutexUnlock(&ob->lock);
}

/* Set the default path to use with ObjectLoad() and ObjectSave(). */
void
AG_ObjectSetArchivePath(void *p, const char *path)
{
	AG_Object *ob = p;

	AG_MutexLock(&ob->lock);
	ob->archivePath = Strdup(path);
	AG_MutexUnlock(&ob->lock);
}

/* Override an object's ops; thread unsafe. */
void
AG_ObjectSetOps(void *p, const void *ops)
{
	OBJECT(p)->ops = ops;
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
		    OBJECT(depobj)->name, (Uint)dep->count);
		if (++dep->count > AG_OBJECT_DEP_MAX) {
			fprintf(stderr, "%s: wiring %s dep (too many refs)\n",
			    ob->name, OBJECT(depobj)->name);
			dep->count = AG_OBJECT_DEP_MAX;
		}
	} else {
		debug(DEBUG_DEPS, "%s: +[%s]\n", ob->name,
		    OBJECT(depobj)->name);
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

	AG_SetError(_("Unable to resolve dependency %s:%u."), ob->name, 
	    (Uint)ind);
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
		    OBJECT(depobj)->name);
		return;
	}

	if (dep->count == AG_OBJECT_DEP_MAX)			/* Wired */
		return;

	if ((dep->count-1) == 0) {
		if ((ob->flags & AG_OBJECT_PRESERVE_DEPS) == 0) {
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
		    OBJECT(depobj)->name, (Uint)dep->count);
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
		AG_FileDelete(path);

	TAILQ_FOREACH(cob, &ob->children, cobjs)
		AG_ObjectUnlinkDatafiles(cob);

	if (AG_ObjectCopyDirname(ob, path, sizeof(path)) == 0)
		AG_RmDir(path);
}

/* Duplicate an object and its children. */
/* XXX EXPERIMENTAL */
void *
AG_ObjectDuplicate(void *p, const char *newName)
{
	char nameSave[AG_OBJECT_NAME_MAX];
	AG_Object *ob = p;
	const AG_ObjectOps *ops = ob->ops;
	AG_Object *dob;

	dob = Malloc(ops->size, M_OBJECT);

	AG_MutexLock(&ob->lock);

	/* Create the duplicate object. */
	if (ops->init != NULL) {
		ops->init(dob, newName);
	} else {
		AG_ObjectInit(dob, newName, ops);
	}

	if (AG_ObjectPageIn(ob) == -1)
		goto fail;

	/* Change the name and attach to the same parent as the original. */
	AG_ObjectAttach(ob->parent, dob);
	dob->flags = (ob->flags & AG_OBJECT_DUPED_FLAGS);

	/* Save the state of the original object using the new name. */
	/* XXX Save to temp location!! */
	strlcpy(nameSave, ob->name, sizeof(nameSave));
	strlcpy(ob->name, dob->name, sizeof(ob->name));
	if (AG_ObjectSave(ob) == -1) {
		AG_ObjectPageOut(ob);
		goto fail;
	}

	if (AG_ObjectPageOut(ob) == -1)
		goto fail;

	if (AG_ObjectLoad(dob) == -1)
		goto fail;

	strlcpy(ob->name, nameSave, sizeof(ob->name));
	AG_MutexUnlock(&ob->lock);
	return (dob);
fail:
	strlcpy(ob->name, nameSave, sizeof(ob->name));
	AG_MutexUnlock(&ob->lock);
	AG_ObjectDestroy(dob);
	Free(dob, M_OBJECT);
	return (NULL);
}

/* Return a cryptographic digest of an object's most recent archive. */
size_t
AG_ObjectCopyChecksum(const void *p, enum ag_object_checksum_alg alg,
    char *digest)
{
	const AG_Object *ob = p;
	char path[MAXPATHLEN];
	Uchar buf[BUFSIZ];
	FILE *f;
	size_t totlen = 0;
	size_t rv;
	
	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1) {
		return (0);
	}
	/* TODO locking */
	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (0);
	}

	switch (alg) {
	case AG_OBJECT_MD5:
		{
			MD5_CTX ctx;

			MD5Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				MD5Update(&ctx, buf, (Uint)rv);
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
				SHA1Update(&ctx, buf, (Uint)rv);
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
				RMD160Update(&ctx, buf, (Uint)rv);
				totlen += rv;
			}
			RMD160End(&ctx, digest);
		}
		break;
	}
	fclose(f);

	return (totlen);
}

/* Return a set of cryptographic digests for an object's most recent archive. */
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

/*
 * Check whether the dataset of the given object or any of its children are
 * different with respect to the last archive.
 */
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

/*
 * Check whether the dataset of the given object is different with respect
 * to its last archive.
 */
int
AG_ObjectChanged(void *p)
{
	char bufCur[BUFSIZ], bufLast[BUFSIZ];
	char pathCur[MAXPATHLEN];
	char pathLast[MAXPATHLEN];
	AG_Object *ob = p;
	FILE *fLast, *fCur;
	size_t rvLast, rvCur;

	if (!OBJECT_PERSISTENT(ob) || !OBJECT_RESIDENT(ob))
		return (0);
	
	AG_ObjectCopyFilename(ob, pathLast, sizeof(pathLast));
	if ((fLast = fopen(pathLast, "r")) == NULL)
		return (1);

	strlcpy(pathCur, AG_String(agConfig,"tmp-path"), sizeof(pathCur));
	strlcat(pathCur, "/_chg.", sizeof(pathCur));
	strlcat(pathCur, ob->name, sizeof(pathCur));
	if (AG_ObjectSaveToFile(ob, pathCur) == -1) {
		dprintf("%s: %s\n", ob->name, AG_GetError());
		fclose(fLast);
		return (1);
	}
	if ((fCur = fopen(pathCur, "r")) == NULL) {
		dprintf("%s: %s\n", pathCur, strerror(errno));
		fclose(fLast);
		return (1);
	}
	for (;;) {
		rvLast = fread(bufLast, 1, sizeof(bufLast), fLast);
		rvCur = fread(bufCur, 1, sizeof(bufCur), fCur);
	
		if (rvLast != rvCur ||
		   (rvLast > 0 && memcmp(bufLast, bufCur, rvLast) != 0)) {
			goto changed;
		}
		if (feof(fLast)) {
			if (!feof(fCur)) { goto changed; }
			break;
		}
		if (feof(fCur)) {
			if (!feof(fLast)) { goto changed; }
			break;
		}
	}
	AG_FileDelete(pathCur);
	fclose(fCur);
	fclose(fLast);
	return (0);
changed:
	AG_FileDelete(pathCur);
	fclose(fCur);
	fclose(fLast);
	return (1);
}

/* Generate an object name that is unique in the given parent object. */
void
AG_ObjectGenName(AG_Object *pobj, const AG_ObjectOps *ops, char *name,
    size_t len)
{
	char tname[AG_OBJECT_TYPE_MAX];
	Uint i = 0;
	AG_Object *ch;
	char *s;
	
	if ((s = strrchr(ops->type, ':')) != NULL && s[1] != '\0') {
		strlcpy(tname, &s[1], sizeof(tname));
	} else {
		strlcpy(tname, ops->type, sizeof(tname));
	}
	tname[0] = (char)toupper(tname[0]);
tryname:
	snprintf(name, len, "%s #%u", tname, i);
	if (pobj != NULL) {
		TAILQ_FOREACH(ch, &pobj->children, cobjs) {
			if (strcmp(ch->name, name) == 0)
				break;
		}
		if (ch != NULL) {
			i++;
			goto tryname;
		}
	}
}
