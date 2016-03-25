/*
 * Copyright (c) 2001-2015 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Base object class.
 */

#include <agar/core/core.h>
#include <agar/core/md5.h>
#include <agar/core/sha1.h>
#include <agar/core/rmd160.h>
#include <agar/core/config.h>

#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include <agar/config/ag_debug_core.h>

AG_ObjectClass agObjectClass = {
	"Agar(Object)",
	sizeof(AG_Object),
	{ 7, 2 },
	NULL,	/* init */
	NULL,	/* reinit */
	NULL,	/* destroy */
	NULL,	/* load */
	NULL,	/* save */
	NULL	/* edit */
};
const AG_Version agPropTblVer = { 2, 1 };
	
int agObjectIgnoreDataErrors = 0;  /* Don't fail on a data load failure. */
int agObjectIgnoreUnknownObjs = 0; /* Don't fail on unknown object types. */
int agObjectBackups = 1;	   /* Backup object save files. */

/* Initialize an AG_Object instance. */
void
AG_ObjectInit(void *p, void *cl)
{
	AG_Object *ob = p;
	AG_ObjectClass **hier;
	int i, nHier;

	ob->name[0] = '\0';
#if defined(_WIN32) || defined(_XBOX)
	ob->save_pfx = "\\world";
#else
	ob->save_pfx = "/world";
#endif
	ob->archivePath = NULL;
	ob->cls = (cl != NULL) ? cl : &agObjectClass;
	ob->parent = NULL;
	ob->root = ob;
	ob->flags = 0;
	ob->attachFn = NULL;
	ob->detachFn = NULL;

	AG_MutexInitRecursive(&ob->lock);
	
	TAILQ_INIT(&ob->vars);
	TAILQ_INIT(&ob->deps);
	TAILQ_INIT(&ob->children);
	TAILQ_INIT(&ob->events);
	TAILQ_INIT(&ob->timers);
	
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = 0; i < nHier; i++) {
			if (hier[i]->init != NULL)
				hier[i]->init(ob);
		}
		free(hier);
	} else {
		AG_FatalError("AG_ObjectInit: %s", AG_GetError());
	}
}

/* Initialize an AG_Object instance (name argument variant). */
void
AG_ObjectInitNamed(void *obj, void *cl, const char *name)
{
	AG_ObjectInit(obj, cl);
	if (name != NULL) {
		AG_ObjectSetNameS(obj, name);
	} else {
		OBJECT(obj)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
}

/* Initialize an AG_Object instance (static variant). */
void
AG_ObjectInitStatic(void *obj, void *cl)
{
	AG_ObjectInit(obj, cl);
	OBJECT(obj)->flags |= AG_OBJECT_STATIC;
}

/*
 * Allocate, initialize and attach a new object instance of the specified
 * class.
 */
void *
AG_ObjectNew(void *parent, const char *name, AG_ObjectClass *cl)
{
	char nameGen[AG_OBJECT_NAME_MAX];
	AG_Object *obj;

	if (name == NULL) {
		AG_ObjectGenName(parent, cl, nameGen, sizeof(nameGen));
	} else {
		if (parent != NULL &&
		    AG_ObjectFindChild(parent, name) != NULL) {
			AG_SetError(_("%s: Existing child object %s"),
			    OBJECT(parent)->name, name);
			return (NULL);
		}
	}
	
	if ((obj = TryMalloc(cl->size)) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(obj, cl);
	AG_ObjectSetNameS(obj, (name != NULL) ? name : nameGen);
	if (parent != NULL) {
		AG_ObjectAttach(parent, obj);
	}
	return (obj);
}

/* Specify the dataset release policy. */
void
AG_ObjectRemain(void *p, Uint flags)
{
	AG_Object *ob = p;

	AG_ObjectLock(ob);
	if (flags & AG_OBJECT_REMAIN_DATA) {
		ob->flags |= (AG_OBJECT_REMAIN_DATA);
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
	preserveDeps = (ob->flags & AG_OBJECT_PRESERVE_DEPS);
	ob->flags |= AG_OBJECT_PRESERVE_DEPS;
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = nHier-1; i >= 0; i--) {
			if (hier[i]->reinit != NULL)
				hier[i]->reinit(ob);
		}
		free(hier);
	} else {
		AG_FatalError("AG_ObjectFreeDataset: %s: %s", ob->name,
		    AG_GetError());
	}
	if (!preserveDeps) {
		ob->flags &= ~(AG_OBJECT_PRESERVE_DEPS);
	}
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

	cur_len = strlen(path);
	name_len = strlen(ob->name);
	
	if (name_len+cur_len+1 > path_len) {
		AG_SetError("Path buffer is %lu bytes (required = %lu)",
		    (Ulong)path_len, (Ulong)(name_len+cur_len+1));
		AG_ObjectUnlock(ob);
		return (-1);
	}
	
	/* Prepend / and the object name. */
	memmove(&path[name_len+1], path, cur_len+1);    /* Move the NUL as well */
	path[0] = AG_PATHSEPCHAR;
	memcpy(&path[1], ob->name, name_len);		/* Omit the NUL */

	if (ob->parent != ob->root && ob->parent != NULL) {
		rv = GenerateObjectPath(ob->parent, path, path_len);
	}
	
	AG_ObjectUnlock(ob);
	return (rv);
}

/*
 * Copy the absolute pathname of an object to a fixed-size buffer.
 * Buffer size must be at least 2 bytes in size.
 */
int
AG_ObjectCopyName(void *obj, char *path, size_t path_len)
{
	AG_Object *ob = obj;
	int rv = 0;

	if (path_len < 2) {
		AG_SetError("Buffer too small");
		return (-1);
	}
	path[0] = AG_PATHSEPCHAR;
	path[1] = '\0';

	AG_LockVFS(ob);
	AG_ObjectLock(ob);
	if (ob == ob->root) {
		Strlcat(path, ob->name, path_len);
	} else {
		rv = GenerateObjectPath(ob, path, path_len);
	}
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (rv);
}

/*
 * Return a newly allocated string containing the absolute pathname of
 * an object.
 */
char *
AG_ObjectGetName(void *obj)
{
	AG_Object *ob = obj;
	AG_Object *pob;
	char *path;
	size_t pathLen = 1;
	
	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	for (pob = ob;
	     pob->parent != NULL;
	     pob = pob->parent) {
		pathLen += strlen(pob->name) + 1;
	}
	if ((path = TryMalloc(pathLen+1)) == NULL) {
		goto fail;
	}
	path[0] = AG_PATHSEPCHAR;
	path[1] = '\0';

	if (ob == ob->root) {
		Strlcat(path, ob->name, pathLen);
	} else {
		GenerateObjectPath(ob, path, pathLen);
	}
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (path);
fail:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (NULL);
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

/* Configure a custom "attach" function. */
void
AG_ObjectSetAttachFn(void *p, void (*fn)(struct ag_event *), const char *fmt, ...)
{
	AG_Object *obj = p;

	AG_ObjectLock(obj);
	if (fn != NULL) {
		obj->attachFn = AG_SetEvent(obj, NULL, fn, NULL);
		AG_EVENT_GET_ARGS(obj->attachFn, fmt);
	} else {
		obj->attachFn = NULL;
	}
	AG_ObjectUnlock(obj);
}

/* Configure a custom "detach" function. */
void
AG_ObjectSetDetachFn(void *p, void (*fn)(struct ag_event *), const char *fmt, ...)
{
	AG_Object *obj = p;

	AG_ObjectLock(obj);
	if (fn != NULL) {
		obj->detachFn = AG_SetEvent(obj, NULL, fn, NULL);
		AG_EVENT_GET_ARGS(obj->detachFn, fmt);
	} else {
		obj->detachFn = NULL;
	}
	AG_ObjectUnlock(obj);
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

	/*
	 * Update the child's "parent" and "root" pointers. If we are using
	 * a custom attach function, we assume that it will follow through.
	 */
	chld->parent = parent;
	chld->root = parent->root;
	
	/* Call the attach function if one is defined. */
	if (chld->attachFn != NULL)  {
		chld->attachFn->fn.fnVoid(chld->attachFn);
		goto out;
	}

	/* Name the object if it has the name-on-attach flag set. */
	if (chld->flags & AG_OBJECT_NAME_ONATTACH) {
		AG_ObjectGenName(parent, chld->cls, chld->name,
		    sizeof(chld->name));
	}
	
	/* Attach the object. */
	TAILQ_INSERT_TAIL(&parent->children, chld, cobjs);

	/* Notify both the parent and child objects. */
	AG_PostEvent(parent, chld, "attached", NULL);
	AG_PostEvent(chld, parent, "child-attached", NULL);

#ifdef AG_DEBUG_CORE
	if (chld->name[0] != '\0') {
		Debug(parent, "Attached child: %s\n", chld->name);
	} else {
		Debug(parent, "Attached child: <%p>\n", chld);
	}
	if (parent->name[0] != '\0') {
		Debug(chld, "New parent: %s\n", parent->name);
	} else {
		Debug(chld, "New parent: <%p>\n", parent);
	}
#endif /* AG_DEBUG_CORE */

out:
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
	char ppath[AG_PATHNAME_MAX];
	void *parent;
	char *p;

	if (Strlcpy(ppath, path, sizeof(ppath)) >= sizeof(ppath)) {
		AG_SetError(_("Path name overflow"));
		return (-1);
	}
	if ((p = strrchr(ppath, AG_PATHSEPCHAR)) != NULL) {
		*p = '\0';
	} else {
		AG_SetError(_("Not an absolute path: %s"), path);
		return (-1);
	}

	AG_LockVFS(vfsRoot);
	if (ppath[0] == '\0') {
		AG_ObjectAttach(vfsRoot, child);
	} else {
		if ((parent = AG_ObjectFindS(vfsRoot, ppath)) == NULL) {
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
AG_ObjectDetach(void *pChld)
{
	AG_Object *chld = pChld;
#ifdef AG_THREADS
	AG_Object *root = chld->root;
#endif
	AG_Object *parent = chld->parent;
	AG_Timer *to, *toNext;

#ifdef AG_THREADS
	AG_LockVFS(root);
#endif
	AG_ObjectLock(parent);
	AG_ObjectLock(chld);

	/* Call the detach function if one is defined. */
	if (chld->detachFn != NULL) {
		chld->detachFn->fn.fnVoid(chld->detachFn);
		goto out;
	}

	/* Cancel any running timer associated with the object. */
	AG_LockTiming();
	for (to = TAILQ_FIRST(&chld->timers);
	     to != TAILQ_END(&chld->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, timers);
		AG_DelTimer(chld, to);
	}
	AG_UnlockTiming();

	/* Detach the object. */
	TAILQ_REMOVE(&parent->children, chld, cobjs);
	chld->parent = NULL;
	chld->root = chld;
	AG_PostEvent(parent, chld, "detached", NULL);
	AG_PostEvent(chld, parent, "child-detached", NULL);

#ifdef AG_DEBUG_CORE
	if (chld->name[0] != '\0') {
		Debug(parent, "Detached child: %s\n", chld->name);
	} else {
		Debug(parent, "Detached child: <%p>\n", chld);
	}
	if (parent->name[0] != '\0') {
		Debug(chld, "New parent: NULL\n");
	}
#endif /* AG_DEBUG_CORE */

out:
	AG_ObjectUnlock(chld);
	AG_ObjectUnlock(parent);
#ifdef AG_THREADS
	AG_UnlockVFS(root);
#endif
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

	if ((s = strchr(node_name, AG_PATHSEPCHAR)) != NULL) {
		*s = '\0';
	}
	TAILQ_FOREACH(child, &parent->children, cobjs) {
		if (strcmp(child->name, node_name) != 0)
			continue;

		if ((s = strchr(name, AG_PATHSEPCHAR)) != NULL &&
		    s[1] != '\0') {
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
 * Search for the named object (absolute path; C string).
 * Return value is only valid as long as VFS is locked.
 */
void *
AG_ObjectFindS(void *vfsRoot, const char *name)
{
	void *rv;

#ifdef AG_DEBUG
	if (name[0] != AG_PATHSEPCHAR)
		AG_FatalError("AG_ObjectFindS: Not an absolute path: %s", name);
#endif
	if (name[0] == AG_PATHSEPCHAR && name[1] == '\0')
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
 * Search for the named object (absolute path; format string).
 * Return value is only valid as long as VFS is locked.
 */
void *
AG_ObjectFind(void *vfsRoot, const char *fmt, ...)
{
	char path[AG_OBJECT_PATH_MAX];
	void *rv;
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);
#ifdef AG_DEBUG
	if (path[0] != AG_PATHSEPCHAR)
		AG_FatalError("AG_ObjectFind: Not an absolute path: %s", path);
#endif
	AG_LockVFS(vfsRoot);
	rv = FindObjectByName(vfsRoot, &path[1]);
	AG_UnlockVFS(vfsRoot);

	if (rv == NULL) {
		AG_SetError(_("The object `%s' does not exist."), path);
	}
	return (rv);
}

/*
 * Traverse an object's ancestry looking for parent object of the given class.
 * Return value is only valid as long as VFS is locked.
 */
void *
AG_ObjectFindParent(void *p, const char *name, const char *t)
{
	AG_Object *ob = AGOBJECT(p);

	AG_LockVFS(p);
	while (ob != NULL) {
		AG_Object *po = AGOBJECT(ob->parent);

		if (po == NULL) {
			goto fail;
		}
		if ((t == NULL || AG_ClassIsNamed(po->cls, t)) &&
		    (name == NULL || strcmp(po->name, name) == 0)) {
			AG_UnlockVFS(p);
			return ((void *)po);
		}
		ob = AGOBJECT(ob->parent);
	}
fail:
	AG_UnlockVFS(p);
	return (NULL);
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
		free(dep);
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
			free(dep);
		}
	}
	TAILQ_FOREACH(cob, &ob->children, cobjs) {
		AG_ObjectFreeDummyDeps(cob);
	}
	AG_ObjectUnlock(ob);
}

/* Detach and free all children. None of them must be in use. */
static void
FreeChildObject(AG_Object *obj)
{
	AG_Object *cob, *ncob;

	AG_ObjectLock(obj);
	for (cob = TAILQ_FIRST(&obj->children);
	     cob != TAILQ_END(&obj->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		FreeChildObject(cob);
	}
	AG_ObjectUnlock(obj);

#ifdef AG_DEBUG_CORE
	if (obj->name[0] != '\0') {
		Debug(NULL, "Freeing: %s\n", obj->name);
	} else {
		Debug(NULL, "Freeing: <%p>\n", obj);
	}
#endif
	AG_ObjectDetach(obj);
	AG_ObjectDestroy(obj);
}
void
AG_ObjectFreeChildren(void *p)
{
	AG_Object *pob = p;
	AG_Object *cob, *ncob;

	AG_ObjectLock(pob);
	for (cob = TAILQ_FIRST(&pob->children);
	     cob != TAILQ_END(&pob->children);
	     cob = ncob) {
		ncob = TAILQ_NEXT(cob, cobjs);
		FreeChildObject(cob);
	}
	TAILQ_INIT(&pob->children);
	AG_ObjectUnlock(pob);
}

/* Destroy the object variables. */
void
AG_ObjectFreeVariables(void *pObj)
{
	AG_Object *ob = pObj;
	AG_Variable *V, *Vnext;

	AG_ObjectLock(ob);
	for (V = TAILQ_FIRST(&ob->vars);
	     V != TAILQ_END(&ob->vars);
	     V = Vnext) {
		Vnext = TAILQ_NEXT(V, vars);
		AG_FreeVariable(V);
		free(V);
	}
	TAILQ_INIT(&ob->vars);
	AG_ObjectUnlock(ob);
}

/* Destroy the event handler structures. */
void
AG_ObjectFreeEvents(AG_Object *ob)
{
	AG_Event *ev, *evNext;

	AG_ObjectLock(ob);
	for (ev = TAILQ_FIRST(&ob->events);
	     ev != TAILQ_END(&ob->events);
	     ev = evNext) {
		evNext = TAILQ_NEXT(ev, events);
		free(ev);
	}
	TAILQ_INIT(&ob->events);
	AG_ObjectUnlock(ob);
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
	AG_Timer *to, *toNext;
	int i, nHier;

#ifdef AG_DEBUG_CORE
	if (ob->parent != NULL) {
		AG_FatalError("AG_ObjectDestroy: %s still attached to %p",
		    ob->name, ob->parent);
	}
	Debug(ob, "Destroying\n");
#endif
	/* Cancel any running timers. */
	AG_LockTiming();
	for (to = TAILQ_FIRST(&ob->timers);
	     to != TAILQ_END(&ob->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, timers);
		AG_DelTimer(ob, to);
	}
	AG_UnlockTiming();

	AG_ObjectFreeChildren(ob);
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);

	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == 0) {
		for (i = nHier-1; i >= 0; i--) {
			if (hier[i]->destroy != NULL)
				hier[i]->destroy(ob);
		}
		free(hier);
	} else {
		AG_FatalError("%s: %s", ob->name, AG_GetError());
	}
	
	AG_ObjectFreeVariables(ob);
	AG_ObjectFreeEvents(ob);
	AG_MutexDestroy(&ob->lock);
	Free(ob->archivePath);
	
	if ((ob->flags & AG_OBJECT_STATIC) == 0)
		free(ob);
}

/*
 * Copy the full pathname to an object's data file to a fixed-size buffer.
 * The path is only valid as long as the VFS is locked.
 */
/* NOTE: Will return data into path even upon failure. */
int
AG_ObjectCopyFilename(void *p, char *path, size_t path_len)
{
	char load_path[AG_PATHNAME_MAX], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	char *dir;

	AG_ObjectLock(ob);
	if (ob->archivePath != NULL) {
		Strlcpy(path, ob->archivePath, path_len);
		goto out;
	}
	AG_GetString(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

	for (dir = Strsep(&loadpathp, AG_PATHSEPMULTI);
	     dir != NULL;
	     dir = Strsep(&loadpathp, AG_PATHSEPMULTI)) {
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
	    obj_name, AG_PATHSEPCHAR, ob->name, ob->cls->name);
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
	char load_path[AG_PATHNAME_MAX], *loadpathp = &load_path[0];
	char obj_name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	char *dir;

	AG_ObjectLock(ob);
	AG_GetString(agConfig, "load-path", load_path, sizeof(load_path));
	AG_ObjectCopyName(ob, obj_name, sizeof(obj_name));

	for (dir = Strsep(&loadpathp, AG_PATHSEPMULTI);
	     dir != NULL;
	     dir = Strsep(&loadpathp, AG_PATHSEPMULTI)) {
		char tmp_path[AG_PATHNAME_MAX];

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
	if (!OBJECT_RESIDENT(ob) &&
	    AG_ObjectLoadData(ob, &dataFound) == -1) {
		if (dataFound == 0) {
			ob->flags |= AG_OBJECT_RESIDENT;
			AG_ObjectUnlock(ob);
			return (0);
		} else {
			AG_ObjectUnlock(ob);
			return (-1);
		}
	}
	AG_ObjectUnlock(ob);
	return (0);
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
	if (!AG_ObjectInUse(ob)) {
		if (AG_FindEventHandler(ob->root, "object-page-out") != NULL) {
			AG_PostEvent(ob, ob->root, "object-page-out", NULL);
		} else {
			if (AG_ObjectSave(ob) == -1)
				goto fail;
		}
		if ((ob->flags & AG_OBJECT_REMAIN_DATA) == 0) {
			AG_ObjectFreeDataset(ob);
			ob->flags &= ~(AG_OBJECT_RESIDENT);
		}
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
		if (dep->obj != NULL) {
			continue;
		}
		if ((dep->obj = AG_ObjectFindS(ob->root, dep->path)) == NULL) {
			AG_SetError(_("%s: Cannot resolve dependency `%s'"),
			    ob->name, dep->path);
			goto fail;
		}
#ifdef AG_DEBUG_CORE
		Debug(ob, "Dependency: %p (%s)\n", dep->obj, dep->obj->name);
#endif
		free(dep->path); dep->path = NULL;
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

/* Read an Agar archive header. */
int
AG_ObjectReadHeader(AG_DataSource *ds, AG_ObjectHeader *oh)
{
	/* Signature and version data */
	if (AG_ReadVersion(ds, agObjectClass.name, &agObjectClass.ver, &oh->ver)
	    == -1)
		return (-1);

	/* Class hierarchy and module references */
	if (oh->ver.minor >= 2) {
		AG_ObjectClassSpec *cs = &oh->cs;
		char *c;

		AG_CopyString(cs->hier, ds, sizeof(cs->hier));
		AG_CopyString(cs->libs, ds, sizeof(cs->libs));

		Strlcpy(cs->spec, cs->hier, sizeof(cs->spec));
		if (cs->libs[0] != '\0') {
			Strlcat(cs->spec, "@", sizeof(cs->spec));
			Strlcat(cs->spec, cs->libs, sizeof(cs->spec));
		}
		if ((c = strrchr(cs->hier, ':')) != NULL && c[1] != '\0') {
			Strlcpy(cs->name, &c[1], sizeof(cs->name));
		} else {
			Strlcpy(cs->name, cs->hier, sizeof(cs->name));
		}
	} else {
		oh->cs.hier[0] = '\0';
		oh->cs.libs[0] = '\0';
		oh->cs.spec[0] = '\0';
		oh->cs.name[0] = '\0';
	}

	/* Dataset start offset */
	oh->dataOffs = AG_ReadUint32(ds);

	/* Object flags */
	oh->flags = (Uint)AG_ReadUint32(ds);

	return (0);
}

/* Load an object dependency table. */
static int
ReadDependencyTable(AG_DataSource *ds, AG_Object *ob)
{
	Uint32 i, count;
	AG_ObjectDep *dep;

	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		if ((dep = TryMalloc(sizeof(AG_ObjectDep))) == NULL) {
			goto fail;
		}
		if ((dep->path = AG_ReadString(ds)) == NULL) {
			free(dep);
			goto fail;
		}
		dep->obj = NULL;
		dep->count = 0;
		dep->persistent = 1;
		TAILQ_INSERT_TAIL(&ob->deps, dep, deps);
	}
	return (0);
fail:
	AG_ObjectFreeDeps(ob);
	return (-1);
}

/* Return the default datafile path for the given object. */
static int
GetDatafile(char *path, AG_Object *ob)
{
	if (ob->archivePath != NULL) {
		Strlcpy(path, ob->archivePath, AG_PATHNAME_MAX);
	} else {
		if (AG_ObjectCopyFilename(ob, path, AG_PATHNAME_MAX) == -1)
			return (-1);
	}
	return (0);
}

/* Load Object variables. */
int
AG_ObjectLoadVariables(void *p, AG_DataSource *ds)
{
	AG_Object *ob = p;
	Uint32 i, count;
	Uint j;

	if (AG_ReadVersion(ds, "AG_PropTbl", &agPropTblVer, NULL) == -1)
		return (-1);

	AG_ObjectLock(ob);

	if (ob->flags & AG_OBJECT_FLOATING_VARS)
		AG_ObjectFreeVariables(ob);

	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		char key[64];
		Sint32 code;
		char *s;

		if (AG_CopyString(key, ds, sizeof(key)) >= sizeof(key)) {
			AG_SetError("Variable name too long: %s", key);
			goto fail;
		}
		code = AG_ReadSint32(ds);
#if 0
		Verbose("%s: %s (code %d)\n", ob->name, key, (int)code);
#endif
		for (j = 0; j < AG_VARIABLE_TYPE_LAST; j++) {
			if (agVariableTypes[j].code == code)
				break;
		}
		if (j == AG_VARIABLE_TYPE_LAST) {
			AG_SetError("Unknown variable code: %u", (Uint)code);
			goto fail;
		}
		switch (agVariableTypes[j].typeTgt) {
		case AG_VARIABLE_UINT:   AG_SetUint(ob, key, (Uint)AG_ReadUint32(ds));	break;
		case AG_VARIABLE_INT:    AG_SetInt(ob, key, (int)AG_ReadSint32(ds));	break;
		case AG_VARIABLE_UINT8:  AG_SetUint8(ob, key, AG_ReadUint8(ds));	break;
		case AG_VARIABLE_SINT8:  AG_SetSint8(ob, key, AG_ReadSint8(ds));	break;
		case AG_VARIABLE_UINT16: AG_SetUint16(ob, key, AG_ReadUint16(ds));	break;
		case AG_VARIABLE_SINT16: AG_SetSint16(ob, key, AG_ReadSint16(ds));	break;
		case AG_VARIABLE_UINT32: AG_SetUint32(ob, key, AG_ReadUint32(ds));	break;
		case AG_VARIABLE_SINT32: AG_SetSint32(ob, key, AG_ReadSint32(ds));	break;
#ifdef AG_HAVE_64BIT
		case AG_VARIABLE_UINT64: AG_SetUint64(ob, key, AG_ReadUint64(ds));	break;
		case AG_VARIABLE_SINT64: AG_SetSint64(ob, key, AG_ReadSint64(ds));	break;
#endif
		case AG_VARIABLE_FLOAT:  AG_SetFloat(ob, key, AG_ReadFloat(ds));	break;
		case AG_VARIABLE_DOUBLE: AG_SetDouble(ob, key, AG_ReadDouble(ds));	break;
#ifdef AG_HAVE_LONG_DOUBLE
		case AG_VARIABLE_LONG_DOUBLE: AG_SetLongDouble(ob, key, AG_ReadLongDouble(ds)); break;
#endif
		case AG_VARIABLE_STRING:
			if ((s = AG_ReadString(ds)) != NULL) {
				AG_SetString(ob, key, s);
				free(s);
			} else {
				AG_SetString(ob, key, "");
			}
			break;
		default:
			AG_SetError("Attempt to load variable of type %s",
			    agVariableTypes[j].name);
			goto fail;
		}
	}
	AG_ObjectUnlock(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
	return (-1);
}

/* Save persistent object variables. */
void
AG_ObjectSaveVariables(void *pObj, AG_DataSource *ds)
{
	AG_Object *ob = pObj;
	off_t countOffs;
	Uint32 count = 0;
	AG_Variable *V;
	
	AG_WriteVersion(ds, "AG_PropTbl", &agPropTblVer);
	countOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	
	AG_ObjectLock(ob);
	TAILQ_FOREACH(V, &ob->vars, vars) {
		const AG_VariableTypeInfo *Vt = &agVariableTypes[V->type];
		void *p;

		if (Vt->code == -1) {
			Verbose("Save: skipping %s (non-persistent)\n", V->name);
			continue;
		}

		AG_LockVariable(V);
		if (V->fn.fnVoid != NULL &&
		    AG_EvalVariable(ob, V) == -1) {
			Verbose("Save: eval %s failed (%s); ignoring\n", V->name,
			    AG_GetError());
			AG_UnlockVariable(V);
			continue;
		}

		AG_WriteString(ds, (char *)V->name);
		AG_WriteSint32(ds, Vt->code);

		p = (agVariableTypes[V->type].indirLvl > 0) ?
		    V->data.p : &V->data;

		switch (AG_VARIABLE_TYPE(V)) {
		case AG_VARIABLE_UINT:	 AG_WriteUint32(ds, (Uint32)*(Uint *)p);	break;
		case AG_VARIABLE_INT:	 AG_WriteSint32(ds, (Sint32)*(int *)p);		break;
		case AG_VARIABLE_UINT8:	 AG_WriteUint8(ds, *(Uint8 *)p);		break;
		case AG_VARIABLE_SINT8:	 AG_WriteSint8(ds, *(Sint8 *)p);		break;
		case AG_VARIABLE_UINT16: AG_WriteUint16(ds, *(Uint16 *)p);		break;
		case AG_VARIABLE_SINT16: AG_WriteSint16(ds, *(Sint16 *)p);		break;
		case AG_VARIABLE_UINT32: AG_WriteUint32(ds, *(Uint32 *)p);		break;
		case AG_VARIABLE_SINT32: AG_WriteSint32(ds, *(Sint32 *)p);		break;
#ifdef AG_HAVE_64BIT
		case AG_VARIABLE_UINT64: AG_WriteUint64(ds, *(Uint64 *)p);		break;
		case AG_VARIABLE_SINT64: AG_WriteSint64(ds, *(Sint64 *)p);		break;
#endif
		case AG_VARIABLE_FLOAT:  AG_WriteFloat(ds, *(float *)p);		break;
		case AG_VARIABLE_DOUBLE: AG_WriteDouble(ds, *(double *)p);		break;
#ifdef AG_HAVE_LONG_DOUBLE
		case AG_VARIABLE_LONG_DOUBLE: AG_WriteLongDouble(ds, *(long double *)p); break;
#endif
		case AG_VARIABLE_STRING: AG_WriteString(ds, V->data.s);			break;
		default: break;
		}
		AG_UnlockVariable(V);

		count++;
	}
	AG_ObjectUnlock(ob);
	AG_WriteUint32At(ds, count, countOffs);
}

/*
 * Load an Agar object (or a virtual filesystem of Agar objects) from an
 * archive file.
 *
 * Only the generic part is read, datasets are skipped and dependencies
 * are left unresolved.
 */
int
AG_ObjectLoadGenericFromFile(void *p, const char *pPath)
{
	AG_Object *ob = p;
	AG_ObjectHeader oh;
	char path[AG_PATHNAME_MAX];
	AG_DataSource *ds;
	Uint32 count, i;
	
	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError(_("Object is non-persistent"));
		return (-1);
	}
	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else {
		if (GetDatafile(path, ob) == -1)
			goto fail_unlock;
	}
#ifdef AG_DEBUG_CORE
	Debug(ob, "Loading generic data from %s\n", path);
#endif
	if ((ds = AG_OpenFile(path, "rb")) == NULL)
		goto fail_unlock;

	/* Free any resident dataset in order to clear the dependencies. */
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);

	/* Object header */
	if (AG_ObjectReadHeader(ds, &oh) == -1) {
		goto fail;
	}
	ob->flags &= ~(AG_OBJECT_SAVED_FLAGS);
	ob->flags |= oh.flags;

	/* Dependencies, properties */
	if (ReadDependencyTable(ds, ob) == -1)
		goto fail;
	if (AG_ObjectLoadVariables(ob, ds) == -1)
		goto fail;
	
	/* Load the generic part of the archived child objects. */
	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		char cname[AG_OBJECT_NAME_MAX];
		char hier[AG_OBJECT_HIER_MAX];
		AG_Object *chld;
		AG_ObjectClass *cl;

	 	/* TODO check that there are no duplicate names. */
		AG_CopyString(cname, ds, sizeof(cname));
		AG_CopyString(hier, ds, sizeof(hier));

		/* Look for an existing object of the given name. */
		if ((chld = AG_ObjectFindChild(ob, cname)) != NULL) {
			if (strcmp(chld->cls->hier, hier) != 0) {
				AG_SetError("Archived object `%s' clashes with "
				            "existing object of different type",
					    cname);
				goto fail;
			}
			if (!OBJECT_PERSISTENT(chld)) {
				AG_SetError("Archived object `%s' clashes with "
				            "existing non-persistent object",
					    cname);
				goto fail;
			}
			if (AG_ObjectLoadGeneric(chld) == -1) {
				goto fail;
			}
			continue;
		}

		/* Create a new child object. */
		if ((cl = AG_LoadClass(hier)) == NULL) {
			AG_SetError("%s: %s", ob->name, AG_GetError());
			if (agObjectIgnoreUnknownObjs) {
#ifdef AG_DEBUG_CORE
				Debug(ob, "%s; ignoring\n", AG_GetError());
#endif
				continue;
			} else {
				goto fail;
			}
			goto fail;
		}
		if ((chld = TryMalloc(cl->size)) == NULL) {
			goto fail;
		}
		AG_ObjectInit(chld, cl);
		AG_ObjectSetNameS(chld, cname);
		AG_ObjectAttach(ob, chld);
		if (AG_ObjectLoadGeneric(chld) == -1)
			goto fail;
	}

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

/* Load an Agar object dataset from an object archive file. */
int
AG_ObjectLoadDataFromFile(void *p, int *dataFound, const char *pPath)
{
	AG_ObjectHeader oh;
	char path[AG_PATHNAME_MAX];
	AG_Object *ob = p;
	AG_DataSource *ds;
	AG_Version ver;
	AG_ObjectClass **hier;
	int i, nHier;
	
	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob)) {
		goto out;
	}
	*dataFound = 1;

	/* Open the file. */
	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else {
		if (GetDatafile(path, ob) == -1) {
			*dataFound = 0;
			goto fail_unlock;
		}
	}
#ifdef AG_DEBUG_CORE
	Debug(ob, "Loading dataset from %s\n", path);
#endif
	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		*dataFound = 0;
		goto fail_unlock;
	}

	/* Seek to the start of the dataset. */
	if (AG_ObjectReadHeader(ds, &oh) == -1 ||
	    AG_Seek(ds, oh.dataOffs, AG_SEEK_SET) == -1 ||
	    AG_ReadVersion(ds, ob->cls->name, &ob->cls->ver, &ver) == -1)
		goto fail;

	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 1);
	}
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == -1)
		goto fail;

	AG_ObjectFreeDataset(ob);

	for (i = 0; i < nHier; i++) {
#ifdef AG_DEBUG_CORE
		Debug(ob, "Loading as %s\n", hier[i]->name);
#endif
		if (hier[i]->load == NULL)
			continue;
		if (hier[i]->load(ob, ds, &ver) == -1) {
			AG_SetError("<0x%x>:%s", (Uint)AG_Tell(ds),
			    AG_GetError());
			free(hier);
			goto fail;
		}
	}
	free(hier);

	AG_CloseFile(ds);
	AG_PostEvent(ob, ob->root, "object-post-load-data", "%s", path);
out:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
fail:
	AG_CloseFile(ds);
fail_unlock:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (-1);
}

static void
BackupObjectFile(void *p, const char *orig)
{
	char path[AG_PATHNAME_MAX];

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

/* Serialize an object to an arbitrary AG_DataSource(3). */
int
AG_ObjectSerialize(void *p, AG_DataSource *ds)
{
	AG_Object *ob = p;
	off_t countOffs, dataOffs;
	Uint32 count;
	AG_Object *chld;
	AG_ObjectDep *dep;
	AG_ObjectClass **hier;
	int i, nHier;

	AG_ObjectLock(ob);
	
	/* Header */
	AG_WriteVersion(ds, agObjectClass.name, &agObjectClass.ver);
	AG_WriteString(ds, ob->cls->hier);
	AG_WriteString(ds, ob->cls->libs);
	dataOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);					/* Data offs */
	AG_WriteUint32(ds, (Uint32)(ob->flags & AG_OBJECT_SAVED_FLAGS));

	/* Interobject dependency table */
	countOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	for (dep = TAILQ_FIRST(&ob->deps), count = 0;
	     dep != TAILQ_END(&ob->deps);
	     dep = TAILQ_NEXT(dep, deps)) {
		char depName[AG_OBJECT_PATH_MAX];
		
		if (!dep->persistent) {
			continue;
		}
		AG_ObjectCopyName(dep->obj, depName, sizeof(depName));
		AG_WriteString(ds, depName);
		count++;
	}
	AG_WriteUint32At(ds, count, countOffs);

	/* Persistent object variables */
	AG_ObjectSaveVariables(ob, ds);
	
	/* Table of child objects */
	if (ob->flags & AG_OBJECT_CHLD_AUTOSAVE) {
		countOffs = AG_Tell(ds);
		AG_WriteUint32(ds, 0);
		count = 0;
		TAILQ_FOREACH(chld, &ob->children, cobjs) {
			if (!OBJECT_PERSISTENT(chld)) {
				continue;
			}
			AG_WriteString(ds, chld->name);
			AG_WriteString(ds, chld->cls->hier);
			count++;
		}
		AG_WriteUint32At(ds, count, countOffs);
	} else {
		AG_WriteUint32(ds, 0);
	}

	/* Dataset */
	AG_WriteUint32At(ds, AG_Tell(ds), dataOffs);
	AG_WriteVersion(ds, ob->cls->name, &ob->cls->ver);

	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 1);
	}
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == -1) {
		goto fail;
	}
	for (i = 0; i < nHier; i++) {
#ifdef AG_DEBUG_CORE
		Debug(ob, "Saving as %s\n", hier[i]->name);
#endif
		if (hier[i]->save == NULL)
			continue;
		if (hier[i]->save(ob, ds) == -1) {
			free(hier);
			goto fail;
		}
	}
	free(hier);

	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 0);
	}
	AG_ObjectUnlock(ob);
	return (0);
fail:
	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 0);
	}
	AG_ObjectUnlock(ob);
	return (-1);
}

/*
 * Unserialize single object (not a VFS) from an arbitrary AG_DataSource(3).
 * To unserialize complete virtual filesystems, see AG_ObjectLoadFromFile().
 */
int
AG_ObjectUnserialize(void *p, AG_DataSource *ds)
{
	AG_Object *ob = p;
	AG_ObjectHeader oh;
	AG_Version ver;
	AG_ObjectClass **hier = NULL;
	int i, nHier;
	
	AG_ObjectLock(ob);

	/* Object header */
	if (AG_ObjectReadHeader(ds, &oh) == -1) {
		goto fail;
	}
	ob->flags &= ~(AG_OBJECT_SAVED_FLAGS);
	ob->flags |= oh.flags;

	/* Dependencies, properties */
	if (ReadDependencyTable(ds, ob) == -1)
		goto fail;
	if (AG_ObjectLoadVariables(ob, ds) == -1)
		goto fail;

	/* Table of child objects, expected empty. */
	if (AG_ReadUint32(ds) != 0) {
		AG_SetError("Archived object has children");
		goto fail;
	}

	/* Dataset */
	if (AG_ReadVersion(ds, ob->cls->name, &ob->cls->ver, &ver) == -1)
		goto fail;

	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 1);
	}
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == -1) {
		goto fail;
	}
	for (i = 0; i < nHier; i++) {
#ifdef AG_DEBUG_CORE
		Debug(ob, "Loading as %s\n", hier[i]->name);
#endif
		if (hier[i]->load == NULL)
			continue;
		if (hier[i]->load(ob, ds, &ver) == -1) {
			AG_SetError("<0x%x>:%s", (Uint)AG_Tell(ds), AG_GetError());
			free(hier);
			goto fail;
		}
	}
	free(hier);

	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 0);
	}
	AG_ObjectUnlock(ob);
	return (0);
fail:
	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		AG_SetSourceDebug(ds, 0);
	}
	AG_ObjectFreeDataset(ob);
	AG_ObjectFreeDeps(ob);
	AG_ObjectUnlock(ob);
	return (-1);
}

/* Archive an object to a file. */
int
AG_ObjectSaveToFile(void *p, const char *pPath)
{
	char pathDir[AG_PATHNAME_MAX];
	char path[AG_PATHNAME_MAX];
	char name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	AG_DataSource *ds;

	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetError("Object (%s) is non-persistent", ob->name);
		goto fail_unlock;
	}

	AG_ObjectCopyName(ob, name, sizeof(name));

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else if (ob->archivePath == NULL) {
		/* Create the save directory if needed. */
		AG_GetString(agConfig, "save-path", pathDir, sizeof(pathDir));
		if (ob->save_pfx != NULL) {
			Strlcat(pathDir, ob->save_pfx, sizeof(pathDir));
		}
		Strlcat(pathDir, name, sizeof(pathDir));
		if (AG_FileExists(pathDir) == 0 &&
		    AG_MkPath(pathDir) == -1)
			goto fail_unlock;
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
#ifdef AG_DEBUG_CORE
	Debug(ob, "Saving object to %s\n", path);
#endif
	if (agObjectBackups) {
		BackupObjectFile(ob, path);
	} else {
		AG_FileDelete(path);
	}
	if ((ds = AG_OpenFile(path, "wb")) == NULL) {
		goto fail_unlock;
	}
	if (AG_ObjectSerialize(ob, ds) == -1) {
		goto fail;
	}
	AG_CloseFile(ds);
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (0);
fail:
	AG_CloseFile(ds);
fail_unlock:
	AG_ObjectUnlock(ob);
	AG_UnlockVFS(ob);
	return (-1);
}

/* Load an object from an AG_Db database entry. */
int
AG_ObjectLoadFromDB(void *obj, AG_Db *db, const AG_Dbt *key)
{
	AG_DataSource *ds;
	AG_Dbt val;

	if (AG_DbGet(db, key, &val) == -1) {
		return (-1);
	}
	if ((ds = AG_OpenCore(val.data, val.size)) == NULL) {
		return (-1);
	}
	if (AG_ObjectUnserialize(obj, ds) == -1) {
		AG_CloseCore(ds);
		return (-1);
	}
	return (0);
}

/* Archive an object to an AG_Db database entry. */
int
AG_ObjectSaveToDB(void *pObj, AG_Db *db, const AG_Dbt *key)
{
	AG_Object *obj = pObj;
	AG_DataSource *ds;
	AG_Dbt dbKey, dbVal;
	int rv;

	if ((ds = AG_OpenAutoCore()) == NULL)
		return (-1);

	AG_LockVFS(obj);
	AG_ObjectLock(obj);
	rv = AG_ObjectSerialize(obj, ds);
	AG_ObjectUnlock(obj);
	AG_UnlockVFS(obj);

	if (rv == -1)
		goto fail;

	dbKey.data = obj->name;
	dbKey.size = strlen(obj->name)+1;
	dbVal.data = AG_CORE_SOURCE(ds)->data;
	dbVal.size = AG_CORE_SOURCE(ds)->size;
	rv = AG_DbPut(db, &dbKey, &dbVal);
	AG_CloseAutoCore(ds);
	return (rv);
fail:
	AG_CloseAutoCore(ds);
	return (-1);
}

/*
 * Change the name of an object (C string).
 * The parent VFS, if any, must be locked.
 */
void
AG_ObjectSetNameS(void *p, const char *name)
{
	AG_Object *ob = p;
	char *c;

	AG_ObjectLock(ob);
	Strlcpy(ob->name, name, sizeof(ob->name));
	for (c = &ob->name[0]; *c != '\0'; c++) {
		if (*c == '/' || *c == '\\')		/* Pathname separator */
			*c = '_';
	}
	AG_ObjectUnlock(ob);
}

/*
 * Change the name of an object (format string).
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
AG_ObjectSetClass(void *p, void *cl)
{
	OBJECT(p)->cls = cl;
}

/* Add a new dependency or increment the reference count on one. */
AG_ObjectDep *
AG_ObjectAddDep(void *p, void *depobj, int persistent)
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
		if (++dep->count > AG_OBJECT_DEP_MAX)
			dep->count = AG_OBJECT_DEP_MAX;
	} else {
		dep = Malloc(sizeof(AG_ObjectDep));
		dep->obj = depobj;
		dep->count = 1;
		dep->persistent = persistent;
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
		goto out;
	}
	if (dep->count == AG_OBJECT_DEP_MAX) {			/* Wired */
		goto out;
	}
	if ((dep->count-1) == 0) {
		if ((ob->flags & AG_OBJECT_PRESERVE_DEPS) == 0) {
			TAILQ_REMOVE(&ob->deps, dep, deps);
			free(dep);
		} else {
			dep->count = 0;
		}
	} else if (dep->count == 0) {
		AG_FatalError("AG_ObjectDelDep: Negative refcount");
	} else {
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

/* Move an object to the head of its parent's children list. */
void
AG_ObjectMoveToHead(void *p)
{
	AG_Object *ob = p;
	AG_Object *parent = ob->parent;

	AG_LockVFS(parent);
	if (parent != NULL) {
		TAILQ_REMOVE(&parent->children, ob, cobjs);
		TAILQ_INSERT_HEAD(&parent->children, ob, cobjs);
	}
	AG_UnlockVFS(parent);
}

/* Move an object to the tail of its parent's children list. */
void
AG_ObjectMoveToTail(void *p)
{
	AG_Object *ob = p;
	AG_Object *parent = ob->parent;

	AG_LockVFS(parent);
	if (parent != NULL) {
		TAILQ_REMOVE(&parent->children, ob, cobjs);
		TAILQ_INSERT_TAIL(&parent->children, ob, cobjs);
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
	char path[AG_PATHNAME_MAX];
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

/*
 * Return a cryptographic digest of an object's most recent archive. The
 * digest is accurate as long as the object is locked.
 */
size_t
AG_ObjectCopyChecksum(void *p, enum ag_object_checksum_alg alg,
    char *digest)
{
	AG_Object *ob = p;
	char path[AG_PATHNAME_MAX];
	Uchar buf[AG_BUFFER_MAX];
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
			AG_MD5_CTX ctx;

			AG_MD5Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				AG_MD5Update(&ctx, buf, (Uint)rv);
				totlen += rv;
			}
			AG_MD5End(&ctx, digest);
		}
		break;
	case AG_OBJECT_SHA1:
		{
			AG_SHA1_CTX ctx;

			AG_SHA1Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				AG_SHA1Update(&ctx, buf, (Uint)rv);
				totlen += rv;
			}
			AG_SHA1End(&ctx, digest);
		}
		break;
	case AG_OBJECT_RMD160:
		{
			AG_RMD160_CTX ctx;

			AG_RMD160Init(&ctx);
			while ((rv = fread(buf, 1, sizeof(buf), f)) > 0) {
				AG_RMD160Update(&ctx, buf, (Uint)rv);
				totlen += rv;
			}
			AG_RMD160End(&ctx, digest);
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
	char md5[AG_MD5_DIGEST_STRING_LENGTH];
	char sha1[AG_SHA1_DIGEST_STRING_LENGTH];
	char rmd160[AG_RMD160_DIGEST_STRING_LENGTH];

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
	char bufCur[AG_BUFFER_MAX], bufLast[AG_BUFFER_MAX];
	char pathCur[AG_PATHNAME_MAX];
	char pathLast[AG_PATHNAME_MAX];
	AG_Object *ob = p;
	FILE *fLast, *fCur;
	size_t rvLast, rvCur;

	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob)) {
		AG_ObjectUnlock(ob);
		return (0);
	}
	AG_ObjectCopyFilename(ob, pathLast, sizeof(pathLast));
	if ((fLast = fopen(pathLast, "r")) == NULL) {
		AG_ObjectUnlock(ob);
		return (1);
	}
	AG_GetString(agConfig, "tmp-path", pathCur, sizeof(pathCur));
	Strlcat(pathCur, AG_PATHSEP, sizeof(pathCur));
	Strlcat(pathCur, "_chg.", sizeof(pathCur));
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
 * object are locked. The class name is used as prefix.
 */
void
AG_ObjectGenName(void *p, AG_ObjectClass *cl, char *name, size_t len)
{
	AG_Object *pobj = p;
	Uint i = 0;
	AG_Object *ch;

tryname:
	Strlcpy(name, cl->name, len);
	Strlcat(name, " #", len);
	StrlcatUint(name, i, len);
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

/* Generate a unique object name using the specified prefix. */
void
AG_ObjectGenNamePfx(void *p, const char *pfx, char *name, size_t len)
{
	AG_Object *pobj = p;
	Uint i = 1;
	AG_Object *ch;

tryname:
	Strlcpy(name, pfx, len);
	StrlcatUint(name, i, len);
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
