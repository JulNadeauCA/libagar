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

#ifdef NETWORK
#include <core/rcs.h>
#endif

#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>

#include <config/lockdebug.h>

AG_ObjectClass agObjectClass = {
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
	
int agObjectIgnoreDataErrors = 0;  /* Don't fail on a data load failure. */
int agObjectIgnoreUnknownObjs = 0; /* Don't fail on unknown object types. */
int agObjectBackups = 1;	   /* Backup object save files. */

void
AG_ObjectInit(void *p, void *cls)
{
	AG_Object *ob = p;
	AG_ObjectClass **hier;
	int i, nHier;

	ob->name[0] = '\0';
	ob->save_pfx = "/world";
	ob->archivePath = NULL;
	ob->cls = (cls != NULL) ? cls : &agObjectClass;
	ob->parent = NULL;
	ob->root = ob;
	ob->flags = 0;
	ob->nevents = 0;

#ifdef LOCKDEBUG
	ob->lockinfo = Malloc(sizeof(char *));
	ob->nlockinfo = 0;
#endif
#ifdef DEBUG
	ob->debugFn = NULL;
	ob->debugPtr = NULL;
#endif
	AG_MutexInitRecursive(&ob->lock);
	
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->children);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->props);
	TAILQ_INIT(&ob->timeouts);
	
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = 0; i < nHier; i++) {
			if (hier[i]->init != NULL)
				hier[i]->init(ob);
		}
		Free(hier);
	} else {
		AG_FatalError("AG_ObjectInit: %s", AG_GetError());
	}
	ob->flags &= ~(AG_OBJECT_RESIDENT);
}

void
AG_ObjectInitStatic(void *p, void *cls)
{
	AG_ObjectInit(p, cls);
	OBJECT(p)->flags |= AG_OBJECT_STATIC;
}

/* Create a new object instance and mark it resident. */
void *
AG_ObjectNew(void *parent, const char *name, AG_ObjectClass *cls)
{
	char nameGen[AG_OBJECT_NAME_MAX];
	AG_Object *obj;

	if (name == NULL) {
		AG_ObjectGenName(parent, cls, nameGen, sizeof(nameGen));
	} else {
		if (parent != NULL &&
		    AG_ObjectFindChild(parent, name) != NULL) {
			AG_SetError(_("%s: Existing child object %s"),
			    OBJECT(parent)->name, name);
			return (NULL);
		}
	}
	
	obj = Malloc(cls->size);
	AG_ObjectInit(obj, cls);
	AG_ObjectSetName(obj, "%s", (name != NULL) ? name : nameGen);
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

	Strlcpy(cname, cn, sizeof(cname));
	Strlcpy(nname, obj->cls->name, sizeof(nname));
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

	AG_ObjectLock(ob);
	if (flags & AG_OBJECT_REMAIN_DATA) {
		ob->flags |= (AG_OBJECT_REMAIN_DATA|AG_OBJECT_RESIDENT);
	} else {
		ob->flags &= ~AG_OBJECT_REMAIN_DATA;
	}
	AG_ObjectUnlock(ob);
}

/*
 * Free an object's dataset. Dependencies are preserved, but they are all
 * assumed to have reference counts of 0.
 */
void
AG_ObjectFreeDataset(void *p)
{
	AG_Object *ob = p;
	int preserveDeps;
	AG_ObjectClass **hier;
	int i, nHier;

	AG_ObjectLock(ob);
	if (!OBJECT_RESIDENT(ob)) {
		goto out;
	}
	preserveDeps = (ob->flags & AG_OBJECT_PRESERVE_DEPS);
	ob->flags |= AG_OBJECT_PRESERVE_DEPS;
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = nHier-1; i >= 0; i--) {
			if (hier[i]->reinit != NULL)
				hier[i]->reinit(ob);
		}
		Free(hier);
	} else {
		AG_FatalError("AG_ObjectFreeDataset: %s: %s", ob->name,
		    AG_GetError());
	}
	ob->flags &= ~(AG_OBJECT_RESIDENT);
	if (!preserveDeps)
		ob->flags &= ~(AG_OBJECT_PRESERVE_DEPS);
out:
	AG_ObjectUnlock(ob);
}

/*
 * Recursive function to construct absolute object names.
 * The object's VFS must be locked.
 */
static int
GenerateObjectPath(void *obj, char *path, size_t path_len)
{
	AG_Object *ob = obj;
	size_t name_len, cur_len;
	int rv = 0;

	AG_ObjectLock(ob);

	cur_len = strlen(path)+1;
	name_len = strlen(ob->name)+1;
	
	if (sizeof("/")+name_len+sizeof("/")+cur_len >= path_len) {
		AG_SetError(_("The path exceeds >= %lu bytes."),
		    (unsigned long)path_len);
		AG_ObjectUnlock(ob);
		return (-1);
	}
	
	/* Prepend / and the object name. */
	memmove(&path[name_len], path, cur_len);    /* Move the NUL as well */
	path[0] = '/';
	memcpy(&path[1], ob->name, name_len-1);	    /* Omit the NUL */

	if (ob->parent != ob->root && ob->parent != NULL) {
		rv = GenerateObjectPath(ob->parent, path, path_len);
	}
	
	AG_ObjectUnlock(ob);
	return (rv);
}

/*
 * Copy the absolute pathname of an object to a fixed-size buffer.
 * The buffer size must be >2 bytes.
 */
int
AG_ObjectCopyName(void *obj, char *path, size_t path_len)
{
	AG_Object *ob = obj;
	int rv = 0;

	path[0] = '/';
	path[1] = '\0';

	AG_LockVFS(ob);
	AG_ObjectLock(ob);
	if (ob != ob->root) {
		Strlcat(path, ob->name, path_len);
	}
	if (ob != ob->root && ob->parent != ob->root && ob->parent != NULL) {
		rv = GenerateObjectPath(ob->parent, path, path_len);
	}
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (rv);
}

/*
 * Search an object and its children for a dependency upon robj.
 * The object's VFS must be locked.
 */
static int
FindObjectInUse(void *p, void *robj)
{
	AG_Object *ob = p, *cob;
	AG_ObjectDep *dep;

	AG_ObjectLock(ob);
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == robj &&
		    robj != ob) {
			AG_SetError(_("The `%s' object is used by `%s'."),
			    OBJECT(robj)->name, ob->name);
			goto used;
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (FindObjectInUse(cob, robj))
			goto used;
	}
	AG_ObjectUnlock(ob);
	return (0);
used:
	AG_ObjectUnlock(ob);
	return (1);
}

/*
 * Return 1 if the given object or one of its children is being referenced.
 * Return value is only valid as long as the VFS is locked.
 */
int
AG_ObjectInUse(void *p)
{
	AG_Object *ob = p, *cob;
	AG_Object *root;

	AG_LockVFS(ob);
	root = AG_ObjectRoot(ob);
	if (FindObjectInUse(root, ob)) {
		goto used;
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (AG_ObjectInUse(cob))
			goto used;
	}
	AG_UnlockVFS(ob);
	return (0);
used:
	AG_UnlockVFS(ob);
	return (1);
}

/* Move an object from one parent object to another. */
void
AG_ObjectMove(void *childp, void *newparentp)
{
	AG_Object *child = childp;
	AG_Object *oparent = child->parent;
	AG_Object *nparent = newparentp;

#ifdef DEBUG
	if (oparent->root != nparent->root)
		AG_FatalError("Cannot move objects across VFSes");
#endif
	AG_LockVFS(oparent);
	AG_ObjectLock(oparent);
	AG_ObjectLock(nparent);
	AG_ObjectLock(child);
	
	TAILQ_REMOVE(&oparent->children, child, cobjs);
	child->parent = NULL;
	AG_PostEvent(oparent, child, "detached", NULL);

	TAILQ_INSERT_TAIL(&nparent->children, child, cobjs);
	child->parent = nparent;
	AG_PostEvent(nparent, child, "attached", NULL);
	AG_PostEvent(oparent, child, "moved", "%p", nparent);

	Debug(child, "Moving from %s to new parent %s\n",
	    oparent->name, nparent->name);
	Debug(oparent, "Detached object: %s (moving to %s)\n",
	    child->name, nparent->name);
	Debug(nparent, "Attached object: %s (originally in %s)\n",
	    child->name, oparent->name);

	AG_ObjectLock(child);
	AG_ObjectLock(nparent);
	AG_ObjectLock(oparent);
	AG_UnlockVFS(oparent);
}

/* Attach an object to another object. */
void
AG_ObjectAttach(void *parentp, void *pChld)
{
	AG_Object *parent = parentp;
	AG_Object *chld = pChld;

	if (parent == NULL)
		return;

	AG_LockVFS(parent);
	AG_ObjectLock(parent);
	AG_ObjectLock(chld);

	if (chld->flags & AG_OBJECT_NAME_ONATTACH) {
		AG_ObjectGenName(parent, chld->cls, chld->name,
		    sizeof(chld->name));
	}
	TAILQ_INSERT_TAIL(&parent->children, chld, cobjs);
	chld->parent = parent;
	chld->root = parent->root;

	AG_PostEvent(parent, chld, "attached", NULL);
	AG_PostEvent(chld, parent, "child-attached", NULL);
	
	Debug(parent, "Attached child object: %s\n", chld->name);
	Debug(chld, "Attached to new parent: %s\n", parent->name);

	AG_ObjectUnlock(chld);
	AG_ObjectUnlock(parent);
	AG_UnlockVFS(parent);
}

/*
 * Attach an object to some other object specified by a pathname specific
 * to the given VFS.
 */
int
AG_ObjectAttachToNamed(void *vfsRoot, const char *path, void *child)
{
	char ppath[MAXPATHLEN];
	void *parent;
	char *p;

	if (Strlcpy(ppath, path, sizeof(ppath)) >= sizeof(ppath)) {
		AG_SetError(_("Path name overflow"));
		return (-1);
	}
	if ((p = strrchr(ppath, '/')) != NULL) {
		*p = '\0';
	} else {
		AG_SetError(_("Not an absolute path: %s"), path);
		return (-1);
	}

	AG_LockVFS(vfsRoot);
	if (ppath[0] == '\0') {
		AG_ObjectAttach(vfsRoot, child);
	} else {
		if ((parent = AG_ObjectFind(vfsRoot, ppath)) == NULL) {
			AG_SetError(_("%s: Cannot attach to %s (%s)"),
			    OBJECT(child)->name, ppath, AG_GetError());
			AG_UnlockVFS(vfsRoot);
			return (-1);
		}
		AG_ObjectAttach(parent, child);
	}
	AG_UnlockVFS(vfsRoot);
	return (0);
}

/* Detach a child object from its parent. */
void
AG_ObjectDetach(void *childp)
{
	AG_Object *child = childp;
	AG_Object *root = child->root;
	AG_Object *parent = child->parent;

	AG_LockVFS(root);
	AG_ObjectLock(parent);
	AG_ObjectLock(child);

	AG_ObjectCancelTimeouts(child, AG_CANCEL_ONDETACH);

	TAILQ_REMOVE(&parent->children, child, cobjs);
	child->parent = NULL;
	child->root = child;
	AG_PostEvent(parent, child, "detached", NULL);
	AG_PostEvent(child, parent, "child-detached", NULL);

	Debug(parent, "Detached child object %s\n", child->name);
	Debug(child, "Detached from parent %s\n", parent->name);

	AG_ObjectUnlock(child);
	AG_ObjectUnlock(parent);
	AG_UnlockVFS(root);
}

/* Traverse the object tree using a pathname. */
static void *
FindObjectByName(const AG_Object *parent, const char *name)
{
	char node_name[AG_OBJECT_PATH_MAX];
	void *rv;
	char *s;
	AG_Object *child;

	Strlcpy(node_name, name, sizeof(node_name));

	if ((s = strchr(node_name, '/')) != NULL) {
		*s = '\0';
	}
	TAILQ_FOREACH(child, &parent->children, cobjs) {
		if (strcmp(child->name, node_name) != 0)
			continue;

		if ((s = strchr(name, '/')) != NULL) {
			rv = FindObjectByName(child, &s[1]);
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

/*
 * Search for the named object (absolute path). Return value is only valid
 * as long as VFS is locked.
 */
void *
AG_ObjectFind(void *vfsRoot, const char *name)
{
	void *rv;

#ifdef DEBUG
	if (name[0] != '/')
		AG_FatalError("AG_ObjectFind: Not an absolute path: %s", name);
#endif
	if (name[0] == '/' && name[1] == '\0')
		return (vfsRoot);
	
	AG_LockVFS(vfsRoot);
	rv = FindObjectByName(vfsRoot, &name[1]);
	AG_UnlockVFS(vfsRoot);

	if (rv == NULL) {
		AG_SetError(_("The object `%s' does not exist."), name);
	}
	return (rv);
}

/*
 * Search for the named object (absolute path), using format string parameter.
 * Return value is only valid as long as VFS is locked.
 */
void *
AG_ObjectFindF(void *vfsRoot, const char *fmt, ...)
{
	char path[AG_OBJECT_PATH_MAX];
	void *rv;
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);
#ifdef DEBUG
	if (path[0] != '/')
		AG_FatalError("AG_ObjectFindF: Not an absolute path: %s", path);
#endif
	AG_LockVFS(vfsRoot);
	rv = FindObjectByName(vfsRoot, &path[1]);
	AG_UnlockVFS(vfsRoot);

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

	AG_ObjectLock(ob);
	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);
		Free(dep);
	}
	TAILQ_INIT(&ob->deps);
	AG_ObjectUnlock(ob);
}

/*
 * Remove any entry in the object's dependency table with a reference count
 * of zero. Also applies to the object's children.
 */
void
AG_ObjectFreeDummyDeps(AG_Object *ob)
{
	AG_Object *cob;
	AG_ObjectDep *dep, *ndep;

	AG_ObjectLock(ob);
	for (dep = TAILQ_FIRST(&ob->deps);
	     dep != TAILQ_END(&ob->deps);
	     dep = ndep) {
		ndep = TAILQ_NEXT(dep, deps);
		if (dep->count == 0) {
			TAILQ_REMOVE(&ob->deps, dep, deps);
			Free(dep);
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		AG_ObjectFreeDummyDeps(cob);
	}
	AG_ObjectUnlock(ob);
}

/*
 * Detach the child objects, and free them, assuming that none of them
 * are currently in use.
 */
void
AG_ObjectFreeChildren(AG_Object *pob)
{
	AG_Object *cob, *ncob;

	AG_ObjectLock(pob);
	for (cob = TAILQ_FIRST(&pob->children);
	     cob != TAILQ_END(&pob->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		Debug(pob, "Freeing child %s\n", cob->name);
		AG_ObjectDetach(cob);
		AG_ObjectDestroy(cob);
	}
	TAILQ_INIT(&pob->children);
	AG_ObjectUnlock(pob);
}

/* Clear an object's property table. */
void
AG_ObjectFreeProps(AG_Object *ob)
{
	AG_Prop *prop, *nextprop;

	AG_ObjectLock(ob);
	for (prop = TAILQ_FIRST(&ob->props);
	     prop != TAILQ_END(&ob->props);
	     prop = nextprop) {
		nextprop = TAILQ_NEXT(prop, props);
		AG_PropDestroy(prop);
		Free(prop);
	}
	TAILQ_INIT(&ob->props);
	AG_ObjectUnlock(ob);
}

/*
 * Destroy the event handlers registered by an object, cancelling
 * any scheduled execution.
 */
void
AG_ObjectFreeEvents(AG_Object *ob)
{
	AG_Event *eev, *neev;

	AG_ObjectLock(ob);
	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = neev) {
		neev = TAILQ_NEXT(eev, events);
	
		if (eev->flags & AG_EVENT_SCHEDULED) {
			AG_CancelEvent(ob, eev->name);
		}
		Free(eev);
	}
	TAILQ_INIT(&ob->events);
	AG_ObjectUnlock(ob);
}

/* Cancel any scheduled timeout event associated with the object. */
void
AG_ObjectCancelTimeouts(void *p, Uint flags)
{
	AG_Object *ob = p, *tob;
	extern struct ag_objectq agTimeoutObjQ;
	AG_Event *ev;

	AG_LockTiming();
	AG_ObjectLock(ob);

	TAILQ_FOREACH(ev, &ob->events, events) {
		if ((ev->flags & AG_EVENT_SCHEDULED) &&
		    (ev->timeout.flags & flags)) {
			Debug(ob, "Cancelling scheduled event <%s>\n",
			    ev->name);
			AG_DelTimeout(ob, &ev->timeout);
			ev->flags &= ~(AG_EVENT_SCHEDULED);
		}
	}
	TAILQ_FOREACH(tob, &agTimeoutObjQ, tobjs) {
		if (tob == ob)
			TAILQ_REMOVE(&agTimeoutObjQ, ob, tobjs);
	}
	TAILQ_INIT(&ob->timeouts);

	AG_ObjectUnlock(ob);
	AG_UnlockTiming();
}

/*
 * Return an array of class structures describing the inheritance
 * hierarchy of an object.
 */
int
AG_ObjectGetInheritHier(void *obj, AG_ObjectClass ***hier, int *nHier)
{
	char cname[AG_OBJECT_TYPE_MAX], *c;
	AG_ObjectClass *cl;
	int i, stop = 0;

	if (AGOBJECT(obj)->cls->name[0] == '\0') {
		(*nHier) = 0;
		return (0);
	}
	(*nHier) = 1;
	Strlcpy(cname, AGOBJECT(obj)->cls->name, sizeof(cname));
	for (c = &cname[0]; *c != '\0'; c++) {
		if (*c == ':')
			(*nHier)++;
	}
	*hier = Malloc((*nHier)*sizeof(AG_ObjectClass *));
	i = 0;
	for (c = &cname[0];; c++) {
		if (*c != ':' && *c != '\0') {
			continue;
		}
		if (*c == '\0') {
			stop++;
		} else {
			*c = '\0';
		}
		if ((cl = AG_FindClass(cname)) == NULL) {
			Free(*hier);
			return (-1);
		}
		*c = ':';
		(*hier)[i++] = cl;
		
		if (stop)
			break;
	}
	return (0);
}

/*
 * Release the resources allocated by an object and its children, assuming
 * that none of them are currently in use.
 */
void
AG_ObjectDestroy(void *p)
{
	AG_Object *ob = p;
	AG_ObjectClass **hier;
	int i, nHier;

#ifdef DEBUG
	if (ob->parent != NULL) {
		AG_FatalError("AG_ObjectDestroy: %s still attached to %p",
		    ob->name, ob->parent);
	}
	Debug(ob, "Destroying\n");
#endif
	AG_ObjectCancelTimeouts(ob, 0);
	AG_ObjectFreeChildren(ob);
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);

	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = nHier-1; i >= 0; i--) {
			if (hier[i]->destroy != NULL)
				hier[i]->destroy(ob);
		}
		Free(hier);
	} else {
		AG_FatalError("%s: %s", ob->name, AG_GetError());
	}

	AG_ObjectFreeProps(ob);
	AG_ObjectFreeEvents(ob);
	AG_MutexDestroy(&ob->lock);
	Free(ob->archivePath);
	
	if ((ob->flags & AG_OBJECT_STATIC) == 0)
		Free(ob);
}

/*
 * Copy the full pathname to an object's data file to a fixed-size buffer.
 * The path is only valid as long as the VFS is locked.
 */
/* NOTE: Will return data into path even upon failure. */
int
AG_ObjectCopyFilename(void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	char *dir;

	AG_ObjectLock(ob);
	if (ob->archivePath != NULL) {
		Strlcpy(path, ob->archivePath, path_len);
		goto out;
	}
	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

	for (dir = AG_Strsep(&loadpathp, ":");
	     dir != NULL;
	     dir = AG_Strsep(&loadpathp, ":")) {
	     	Strlcpy(path, dir, path_len);
		if (ob->save_pfx != NULL) {
			Strlcat(path, ob->save_pfx, path_len);
		}
		Strlcat(path, obj_name, path_len);
		Strlcat(path, AG_PATHSEP, path_len);
		Strlcat(path, ob->name, path_len);
		Strlcat(path, ".", path_len);
		Strlcat(path, ob->cls->name, path_len);

		if (AG_FileExists(path))
			goto out;
	}
	AG_SetError(_("The %s%s%c%s.%s file is not in load-path."),
	    ob->save_pfx != NULL ? ob->save_pfx : "",
	    obj_name, AG_PATHSEPC, ob->name, ob->cls->name);
	AG_ObjectUnlock(ob);
	return (-1);
out:
	AG_ObjectUnlock(ob);
	return (0);
}

/*
 * Copy the full pathname of an object's data dir to a fixed-size buffer.
 * The path is only valid as long as the VFS is locked.
 */
int
AG_ObjectCopyDirname(void *p, char *path, size_t path_len)
{
	char load_path[MAXPATHLEN], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	char *dir;

	AG_ObjectLock(ob);
	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

	for (dir = AG_Strsep(&loadpathp, ":");
	     dir != NULL;
	     dir = AG_Strsep(&loadpathp, ":")) {
		char tmp_path[MAXPATHLEN];

	     	Strlcpy(tmp_path, dir, sizeof(tmp_path));
		if (ob->save_pfx != NULL) {
			Strlcat(tmp_path, ob->save_pfx, sizeof(tmp_path));
		}
		Strlcat(tmp_path, obj_name, sizeof(tmp_path));
		if (AG_FileExists(tmp_path)) {
			Strlcpy(path, tmp_path, path_len);
			goto out;
		}
	}
	AG_SetError(_("The %s directory is not in load-path."), obj_name);
	AG_ObjectUnlock(ob);
	return (-1);
out:
	AG_ObjectUnlock(ob);
	return (0);
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

	AG_ObjectLock(ob);
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
	AG_ObjectUnlock(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
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
	
	AG_ObjectLock(ob);
	if (!OBJECT_PERSISTENT(ob)) {
		goto out;
	}
	if (!OBJECT_RESIDENT(ob)) {
		AG_SetError(_("Object is non-resident"));
		goto fail;
	}
	if (!AG_ObjectInUse(ob)) {
		if (AG_FindEventHandler(ob->root, "object-page-out") != NULL) {
			AG_PostEvent(ob, ob->root, "object-page-out", NULL);
		} else {
			if (AG_ObjectSave(ob) == -1)
				goto fail;
		}
		if ((ob->flags & AG_OBJECT_REMAIN_DATA) == 0)
			AG_ObjectFreeDataset(ob);
	}
out:
	AG_ObjectUnlock(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
	return (-1);
}

/* Load both the generic part and the dataset of an object from file. */
int
AG_ObjectLoadFromFile(void *p, const char *path)
{
	AG_Object *ob = p;
	int dataFound;

	AG_LockVFS(ob);
	AG_ObjectLock(ob);
	if (AG_ObjectLoadGenericFromFile(ob, path) == -1 ||
	    AG_ObjectResolveDeps(ob) == -1 ||
	    AG_ObjectLoadDataFromFile(ob, &dataFound, path) == -1) {
		goto fail;
	}
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (-1);
}

/*
 * Resolve the encoded dependencies of an object and its children.
 * The object's VFS must be locked.
 */
int
AG_ObjectResolveDeps(void *p)
{
	AG_Object *ob = p, *cob;
	AG_ObjectDep *dep;

	AG_ObjectLock(ob);

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		Debug(ob, "Resolving dependency: %s...\n", dep->path);
		if (dep->obj != NULL) {
			Debug(ob, "Already resolved\n");
			continue;
		}
		if ((dep->obj = AG_ObjectFind(ob->root, dep->path)) == NULL) {
			Debug(ob, "Failed to resolve dependency: %s\n",
			    dep->path);
			AG_SetError(_("%s: Cannot resolve dependency `%s'"),
			    ob->name, dep->path);
			goto fail;
		}
		Debug(ob, "Dependency resolves to %p (%s)\n", dep->obj,
		    dep->obj->name);
		Free(dep->path);
		dep->path = NULL;
	}

	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		AG_ObjectLock(cob);
		if (!OBJECT_PERSISTENT(cob)) {
			AG_ObjectUnlock(cob);
			continue;
		}
		if (AG_ObjectResolveDeps(cob) == -1) {
			AG_ObjectUnlock(cob);
			goto fail;
		}
		AG_ObjectUnlock(cob);
	}
	AG_ObjectUnlock(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
	return (-1);
}

/*
 * Load the generic part of an object from archive. If the archived
 * object has children, create instances for them and load their
 * generic part as well.
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
	AG_LockVFS(ob);
	AG_ObjectLock(ob);
	AG_ObjectCancelTimeouts(ob, AG_CANCEL_ONLOAD);

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else {
		if (ob->archivePath != NULL) {
			Strlcpy(path, ob->archivePath, sizeof(path));
		} else {
			if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1)
				goto fail_unlock;
		}
	}
	Debug(ob, "Loading generic data from %s\n", path);

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		goto fail_unlock;
	}
	if (AG_ReadVersion(ds, agObjectClass.name, &agObjectClass.ver, &ver)
	    == -1)
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

	/* Skip dataset offset */
	(void)AG_ReadUint32(ds);

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
		dep = Malloc(sizeof(AG_ObjectDep));
		dep->path = AG_ReadString(ds);
		dep->obj = NULL;
		dep->count = 0;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
		Debug(ob, "Dependency: %s\n", dep->path);
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
			if (strcmp(eob->cls->name, classID) != 0) {
				AG_FatalError("AG_ObjectLoad: Existing object "
				              "of different type in archive");
			}
			if (!OBJECT_PERSISTENT(eob)) {
				AG_FatalError("AG_ObjectLoad: Existing "
				              "non-persistent object");
			}
			if (AG_ObjectLoadGeneric(eob) == -1) {
				goto fail;
			}
		} else {
			AG_ObjectClass *cl;

			if ((cl = AG_FindClass(classID)) == NULL) {
				AG_SetError("%s: %s", ob->name, AG_GetError());
				if (agObjectIgnoreUnknownObjs) {
					Debug(ob, "%s; ignoring\n",
					    AG_GetError());
					continue;
				} else {
					goto fail;
				}
				goto fail;
			}

			child = Malloc(cl->size);
			AG_ObjectInit(child, cl);
			AG_ObjectSetName(child, "%s", cname);
			AG_ObjectAttach(ob, child);
			if (AG_ObjectLoadGeneric(child) == -1)
				goto fail;
		}
	}
	AG_PostEvent(ob, ob->root, "object-post-load-generic", "%s", path);
	AG_CloseFile(ds);
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
fail:
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);
	AG_CloseFile(ds);
fail_unlock:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (-1);
}

/* Load only the dataset part of an object archive. */
int
AG_ObjectLoadDataFromFile(void *p, int *dataFound, const char *pPath)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p;
	AG_DataSource *ds;
	off_t dataOffs;
	AG_Version version;
	AG_ObjectClass **hier;
	int i, nHier;
	
	AG_LockVFS(ob);
	AG_ObjectLock(ob);
	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError(_("Object is non-persistent"));
		goto out;
	}
	AG_ObjectCancelTimeouts(ob, AG_CANCEL_ONLOAD);

	*dataFound = 1;

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else {
		if (ob->archivePath != NULL) {
			Strlcpy(path, ob->archivePath, sizeof(path));
		} else {
			if (AG_ObjectCopyFilename(ob, path, sizeof(path))
			    == -1) {
				*dataFound = 0;
				goto fail_unlock;
			}
		}
	}
	Debug(ob, "Loading dataset from %s\n", path);

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		*dataFound = 0;
		goto fail_unlock;
	}
	if (AG_ReadVersion(ds, agObjectClass.name, &agObjectClass.ver, NULL)
	    == -1)
		goto fail;

	/* Write dataset offset */
	dataOffs = (off_t)AG_ReadUint32(ds);
	AG_Seek(ds, dataOffs, AG_SEEK_SET);

	if (OBJECT_RESIDENT(ob)) {
		AG_ObjectFreeDataset(ob);
	}
	if (AG_ReadVersion(ds, ob->cls->name, &ob->cls->ver, &version) == -1) {
		goto fail;
	}
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = 0; i < nHier; i++) {
			if (hier[i]->load == NULL)
				continue;
			if (hier[i]->load(ob, ds, &version) == -1)
				goto fail;
		}
		Free(hier);
	} else {
		AG_FatalError("AG_ObjectLoad: %s: %s", ob->name, AG_GetError());
	}
	ob->flags |= AG_OBJECT_RESIDENT;
	AG_CloseFile(ds);
	AG_PostEvent(ob, ob->root, "object-post-load-data", "%s", path);
out:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
fail:
	AG_CloseFile(ds);
fail_unlock:
	AG_UnlockVFS(ob);
	return (-1);
}

static void
BackupObjectFile(void *p, const char *orig)
{
	char path[MAXPATHLEN];

	if (AG_FileExists(orig)) {
		Strlcpy(path, orig, sizeof(path));
		Strlcat(path, ".bak", sizeof(path));
		rename(orig, path);
	}
}

/* Save the state of an object and its children. */
int
AG_ObjectSaveAll(void *p)
{
	AG_Object *obj = p, *cobj;

	AG_LockVFS(obj);
	AG_ObjectLock(obj);

	if (AG_ObjectSave(obj) == -1) {
		goto fail;
	}
	TAILQ_FOREACH(cobj, &obj->children, cobjs) {
		AG_ObjectLock(cobj);
		if (!OBJECT_PERSISTENT(cobj)) {
			AG_ObjectUnlock(cobj);
			continue;
		}
		if (AG_ObjectSaveAll(cobj) == -1) {
			AG_ObjectUnlock(cobj);
			goto fail;
		}
		AG_ObjectUnlock(cobj);
	}

	AG_ObjectUnlock(obj);
	AG_UnlockVFS(obj);
	return (0);
fail:
	AG_ObjectUnlock(obj);
	AG_UnlockVFS(obj);
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
	AG_ObjectClass **hier;
	int i, nHier;

	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError(_("The `%s' object is non-persistent."), ob->name);
		goto fail_lock;
	}
	AG_ObjectCopyName(ob, name, sizeof(name));

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else if (ob->archivePath == NULL) {
		/* Create the save directory if needed. */
		Strlcpy(pathDir, AG_String(agConfig,"save-path"),
		    sizeof(pathDir));
		if (ob->save_pfx != NULL) {
			Strlcat(pathDir, ob->save_pfx, sizeof(pathDir));
		}
		Strlcat(pathDir, name, sizeof(pathDir));
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
			Strlcpy(path, ob->archivePath, sizeof(path));
		} else {
			Strlcpy(path, pathDir, sizeof(path));
			Strlcat(path, AG_PATHSEP, sizeof(path));
			Strlcat(path, ob->name, sizeof(path));
			Strlcat(path, ".", sizeof(path));
			Strlcat(path, ob->cls->name, sizeof(path));
		}
	}
	Debug(ob, "Saving object to %s\n", path);

	if (agObjectBackups) {
		BackupObjectFile(ob, path);
	} else {
		AG_FileDelete(path);
	}
	if ((ds = AG_OpenFile(path, "wb")) == NULL)
		goto fail_reinit;

	AG_WriteVersion(ds, agObjectClass.name, &agObjectClass.ver);

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
		AG_WriteString(ds, child->cls->name);
		count++;
	}
	AG_WriteUint32At(ds, count, countOffs);

	/* Save the dataset. */
	AG_WriteUint32At(ds, AG_Tell(ds), dataOffs);
	AG_WriteVersion(ds, ob->cls->name, &ob->cls->ver);
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = 0; i < nHier; i++) {
			if (hier[i]->save == NULL)
				continue;
			if (hier[i]->save(ob, ds) == -1)
				goto fail;
		}
		Free(hier);
	} else {
		AG_FatalError("AG_ObjectSave: %s: %s", ob->name, AG_GetError());
	}

	AG_CloseFile(ds);
	if (pagedTemporarily) { AG_ObjectFreeDataset(ob); }
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
fail:
	AG_CloseFile(ds);
fail_reinit:
	if (pagedTemporarily) { AG_ObjectFreeDataset(ob); }
fail_lock:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (-1);
}

/*
 * Change the name of an object.
 * The parent VFS, if any, must be locked.
 */
void
AG_ObjectSetName(void *p, const char *fmt, ...)
{
	AG_Object *ob = p;
	va_list ap;
	char *c;

	AG_ObjectLock(ob);

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(ob->name, sizeof(ob->name), fmt, ap);
		va_end(ap);
	} else {
		ob->name[0] = '\0';
	}

	for (c = &ob->name[0]; *c != '\0'; c++) {
		if (*c == '/' || *c == '\\')		/* Pathname separator */
			*c = '_';
	}
	
	AG_ObjectUnlock(ob);
}

/* Specify a function to invoke in order to process Debug() messages. */
void
AG_ObjectSetDebugFn(void *pObj, void (*fn)(void *, void *, const char *),
    void *pUser)
{
#ifdef DEBUG
	AG_Object *obj = pObj;
	
	AG_ObjectLock(obj);
	obj->debugFn = fn;
	obj->debugPtr = pUser;
	AG_ObjectUnlock(obj);
#endif
}

/*
 * Return the archive path of an object into a fixed buffer. Returned path
 * is valid as long as the object is locked.
 */
void
AG_ObjectGetArchivePath(void *p, char *buf, size_t buf_len)
{
	AG_Object *ob = p;

	AG_ObjectLock(ob);
	Strlcpy(buf, ob->archivePath, buf_len);
	AG_ObjectUnlock(ob);
}

/* Set the default path to use with ObjectLoad() and ObjectSave(). */
void
AG_ObjectSetArchivePath(void *p, const char *path)
{
	AG_Object *ob = p;

	AG_ObjectLock(ob);
	Free(ob->archivePath);
	ob->archivePath = Strdup(path);
	AG_ObjectUnlock(ob);
}

/* Change an object's class. This is thread unsafe and should not be used. */
void
AG_ObjectSetClass(void *p, void *cls)
{
	OBJECT(p)->cls = cls;
}

/* Add a new dependency or increment the reference count on one. */
AG_ObjectDep *
AG_ObjectAddDep(void *p, void *depobj)
{
	AG_Object *ob = p;
	AG_ObjectDep *dep;

	AG_ObjectLock(ob);
	AG_ObjectLock(depobj);

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == depobj)
			break;
	}
	if (dep != NULL) {
		Debug(ob, "Increment dependency on %s (#%u)\n",
		    OBJECT(depobj)->name, (Uint)dep->count);
		if (++dep->count > AG_OBJECT_DEP_MAX) {
			Debug(ob, "Wiring dependency: %s (too many refs!)\n",
			    OBJECT(depobj)->name);
			dep->count = AG_OBJECT_DEP_MAX;
		}
	} else {
		Debug(ob, "Create dependency on %s\n", OBJECT(depobj)->name);
		dep = Malloc(sizeof(AG_ObjectDep));
		dep->obj = depobj;
		dep->count = 1;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
	}

	AG_ObjectUnlock(depobj);
	AG_ObjectUnlock(ob);
	return (dep);
}

/* Resolve a given dependency. */
int
AG_ObjectFindDep(void *p, Uint32 ind, void **objp)
{
	AG_Object *ob = p;
	AG_ObjectDep *dep;
	Uint32 i;

	if (ind == 0) {
		*objp = NULL;
		return (0);
	} else if (ind == 1) {
		*objp = (void *)ob;
		return (0);
	}

	AG_ObjectLock(ob);
	for (dep = TAILQ_FIRST(&ob->deps), i = 2;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), i++) {
		if (i == ind)
			break;
	}
	if (dep == NULL) {
		AG_SetError(_("Unable to resolve dependency %s:%u."), ob->name, 
		    (Uint)ind);
		AG_ObjectUnlock(ob);
		return (-1);
	}
	*objp = dep->obj;
	AG_ObjectUnlock(ob);
	return (0);
}

/*
 * Encode an object dependency. The values 0 and 1 are reserved, 0 is a
 * NULL value and 1 is the parent object itself.
 */
Uint32
AG_ObjectEncodeName(void *p, const void *depobjp)
{
	AG_Object *ob = p;
	const AG_Object *depobj = depobjp;
	AG_ObjectDep *dep;
	Uint32 i;

	if (depobjp == NULL) {
		return (0);
	} else if (p == depobjp) {
		return (1);
	}

	AG_ObjectLock(ob);
	for (dep = TAILQ_FIRST(&ob->deps), i = 2;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps), i++) {
		if (dep->obj == depobj) {
			AG_ObjectUnlock(ob);
			return (i);
		}
	}
	AG_FatalError("AG_ObjectEncodeName: %s: No such dep", depobj->name);
	AG_ObjectUnlock(ob);
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

	AG_ObjectLock(ob);

	TAILQ_FOREACH(dep, &ob->deps, deps) {
		if (dep->obj == depobj)
			break;
	}
	if (dep == NULL) {
		Debug(ob, "Attempt to remove invalid dep: %s\n",
		    OBJECT(depobj)->name);
		goto out;
	}
	if (dep->count == AG_OBJECT_DEP_MAX) {			/* Wired */
		goto out;
	}
	if ((dep->count-1) == 0) {
		if ((ob->flags & AG_OBJECT_PRESERVE_DEPS) == 0) {
			Debug(ob, "Remove dependency on %s\n",
			    OBJECT(depobj)->name);
			TAILQ_REMOVE(&ob->deps, dep, deps);
			Free(dep);
		} else {
			dep->count = 0;
		}
	} else if (dep->count == 0) {
		AG_FatalError("AG_ObjectDelDep: Negative refcount");
	} else {
		Debug(ob, "Decrement dependency on %s (#%u)\n",
		    OBJECT(depobj)->name, (Uint)dep->count);
		dep->count--;
	}
out:
	AG_ObjectUnlock(ob);
}

/* Move an object towards the head of its parent's children list. */
void
AG_ObjectMoveUp(void *p)
{
	AG_Object *ob = p, *prev;
	AG_Object *parent = ob->parent;

	AG_LockVFS(parent);
	if (parent != NULL && ob != TAILQ_FIRST(&parent->children)) {
		prev = TAILQ_PREV(ob, ag_objectq, cobjs);
		TAILQ_REMOVE(&parent->children, ob, cobjs);
		TAILQ_INSERT_BEFORE(prev, ob, cobjs);
	}
	AG_UnlockVFS(parent);
}

/* Move an object towards the tail of its parent's children list. */
void
AG_ObjectMoveDown(void *p)
{
	AG_Object *ob = p;
	AG_Object *parent = ob->parent;
	AG_Object *next = TAILQ_NEXT(ob, cobjs);

	AG_LockVFS(parent);
	if (parent != NULL && next != NULL) {
		TAILQ_REMOVE(&parent->children, ob, cobjs);
		TAILQ_INSERT_AFTER(&parent->children, next, ob, cobjs);
	}
	AG_UnlockVFS(parent);
}

/* Change the save prefix of an object and its children. */
void
AG_ObjectSetSavePfx(void *p, char *path)
{
	AG_Object *ob = p, *cob;

	AG_LockVFS(ob);
	AG_ObjectLock(ob);
	ob->save_pfx = path;
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		AG_ObjectSetSavePfx(cob, path);
	}
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
}

/*
 * Remove the data files of an object and its children.
 * The object's VFS must be locked.
 */
void
AG_ObjectUnlinkDatafiles(void *p)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p, *cob;

	AG_ObjectLock(ob);
	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == 0) {
		AG_FileDelete(path);
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		AG_ObjectUnlinkDatafiles(cob);
	}
	if (AG_ObjectCopyDirname(ob, path, sizeof(path)) == 0) {
		AG_RmDir(path);
	}
	AG_ObjectUnlock(ob);
}

/* Duplicate an object and its children. */
/* XXX EXPERIMENTAL */
void *
AG_ObjectDuplicate(void *p, const char *newName)
{
	char nameSave[AG_OBJECT_NAME_MAX];
	AG_Object *ob = p;
	AG_ObjectClass *cls = ob->cls;
	AG_Object *dob;

	dob = Malloc(cls->size);
	AG_ObjectLock(ob);
	AG_ObjectInit(dob, cls);
	AG_ObjectSetName(dob, "%s", newName);
	if (AG_ObjectPageIn(ob) == -1) {
		goto fail;
	}
	/* Change the name and attach to the same parent as the original. */
	AG_ObjectAttach(ob->parent, dob);
	dob->flags = (ob->flags & AG_OBJECT_DUPED_FLAGS);

	/* Save the state of the original object using the new name. */
	/* XXX Save to temp location!! */
	Strlcpy(nameSave, ob->name, sizeof(nameSave));
	Strlcpy(ob->name, dob->name, sizeof(ob->name));
	if (AG_ObjectSave(ob) == -1) {
		AG_ObjectPageOut(ob);
		goto fail;
	}
	if (AG_ObjectPageOut(ob) == -1) {
		goto fail;
	}
	if (AG_ObjectLoad(dob) == -1) {
		goto fail;
	}
	Strlcpy(ob->name, nameSave, sizeof(ob->name));
	AG_ObjectUnlock(ob);
	return (dob);
fail:
	Strlcpy(ob->name, nameSave, sizeof(ob->name));
	AG_ObjectUnlock(ob);
	AG_ObjectDestroy(dob);
	return (NULL);
}

/*
 * Return a cryptographic digest of an object's most recent archive. The
 * digest is accurate as long as the object is locked.
 */
size_t
AG_ObjectCopyChecksum(void *p, enum ag_object_checksum_alg alg,
    char *digest)
{
	AG_Object *ob = p;
	char path[MAXPATHLEN];
	Uchar buf[BUFSIZ];
	FILE *f;
	size_t totlen = 0;
	size_t rv;

	AG_ObjectLock(ob);

	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1) {
		goto fail;
	}
	/* TODO locking */
	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("Unable to open %s", path);
		goto fail;
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

	AG_ObjectUnlock(ob);
	return (totlen);
fail:
	AG_ObjectUnlock(ob);
	return (0);
}

/*
 * Return a set of cryptographic digests for an object's most recent archive.
 * The digests are accurate as long as the object is locked.
 */
int
AG_ObjectCopyDigest(void *ob, size_t *len, char *digest)
{
	char md5[MD5_DIGEST_STRING_LENGTH];
	char sha1[SHA1_DIGEST_STRING_LENGTH];
	char rmd160[RMD160_DIGEST_STRING_LENGTH];

	AG_ObjectLock(ob);
	if ((*len = AG_ObjectCopyChecksum(ob, AG_OBJECT_MD5, md5)) == 0 ||
	    AG_ObjectCopyChecksum(ob, AG_OBJECT_SHA1, sha1) == 0 ||
	    AG_ObjectCopyChecksum(ob, AG_OBJECT_RMD160, rmd160) == 0) {
		goto fail;
	}
	if (Snprintf(digest, AG_OBJECT_DIGEST_MAX, "(md5|%s sha1|%s rmd160|%s)",
	    md5, sha1, rmd160) >= AG_OBJECT_DIGEST_MAX) {
		AG_SetError(_("String overflow"));
		goto fail;
	}
	AG_ObjectUnlock(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
	return (-1);
}

/*
 * Check whether the dataset of the given object or any of its children are
 * different with respect to the last archive. The result is only valid as
 * long as the object and VFS are locked, and this assumes that no other
 * application is concurrently accessing the datafiles.
 */
int
AG_ObjectChangedAll(void *p)
{
	AG_Object *ob = p, *cob;

	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (AG_ObjectChanged(ob) == 1) {
		goto changed;
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		if (AG_ObjectChangedAll(cob) == 1)
			goto changed;
	}

	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
changed:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (1);
}

/*
 * Check whether the dataset of the given object is different with respect
 * to its last archive. The result is only valid as long as the object is
 * locked, and this assumes no other application is concurrently accessing
 * the datafiles.
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

	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob) || !OBJECT_RESIDENT(ob)) {
		AG_ObjectUnlock(ob);
		return (0);
	}
	AG_ObjectCopyFilename(ob, pathLast, sizeof(pathLast));
	if ((fLast = fopen(pathLast, "r")) == NULL) {
		AG_ObjectUnlock(ob);
		return (1);
	}
	Strlcpy(pathCur, AG_String(agConfig,"tmp-path"), sizeof(pathCur));
	Strlcat(pathCur, "/_chg.", sizeof(pathCur));
	Strlcat(pathCur, ob->name, sizeof(pathCur));
	if (AG_ObjectSaveToFile(ob, pathCur) == -1) {
		fclose(fLast);
		AG_ObjectUnlock(ob);
		return (1);
	}
	if ((fCur = fopen(pathCur, "r")) == NULL) {
		fclose(fLast);
		AG_ObjectUnlock(ob);
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
	AG_ObjectUnlock(ob);
	return (0);
changed:
	AG_FileDelete(pathCur);
	fclose(fCur);
	fclose(fLast);
	AG_ObjectUnlock(ob);
	return (1);
}

/*
 * Generate an object name that is unique in the given parent object. The
 * name is only guaranteed to remain unique as long as the VFS and parent
 * object are locked.
 */
void
AG_ObjectGenName(AG_Object *pobj, AG_ObjectClass *cls, char *name, size_t len)
{
	char tname[AG_OBJECT_TYPE_MAX];
	Uint i = 0;
	AG_Object *ch;
	char *s;
	
	if ((s = strrchr(cls->name, ':')) != NULL && s[1] != '\0') {
		Strlcpy(tname, &s[1], sizeof(tname));
	} else {
		Strlcpy(tname, cls->name, sizeof(tname));
	}
	tname[0] = (char)toupper(tname[0]);
tryname:
	Snprintf(name, len, "%s #%u", tname, i);
	if (pobj != NULL) {
		AG_LockVFS(pobj);
		TAILQ_FOREACH(ch, &pobj->children, cobjs) {
			if (strcmp(ch->name, name) == 0)
				break;
		}
		AG_UnlockVFS(pobj);
		if (ch != NULL) {
			i++;
			goto tryname;
		}
	}
}

#ifdef LOCKDEBUG
void
AG_ObjectLockDebug(AG_Object *ob, const char *info)
{
	if (agDebugLvl >= 10) { AG_Debug(ob, "Locking (%s)...", info); }
	AG_MutexLock(&ob->lock);
	if (agDebugLvl >= 10) { AG_Debug(ob, "OK\n"); }

	ob->lockinfo = (const char **)AG_Realloc(ob->lockinfo,
	    (ob->nlockinfo+1)*sizeof(char *));
	ob->lockinfo[ob->nlockinfo++] = info;
}

void
AG_ObjectUnlockDebug(AG_Object *ob, const char *info)
{
	ob->nlockinfo--;
	AG_MutexUnlock(&ob->lock);
	if (agDebugLvl >= 10) { AG_Debug(ob, "Unlocked (%s)\n", info); }
}
#endif /* LOCKDEBUG */
