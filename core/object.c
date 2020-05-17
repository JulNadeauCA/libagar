/*
 * Copyright (c) 2001-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/core/config.h>

#include <stdarg.h>
#include <ctype.h>
#include <string.h>

/* Expensive debugging output related to AG_Object VFS operations. */
/* #define DEBUG_OBJECT */

/* Extra debugging output related to serialization. */
/* #define DEBUG_SERIALIZATION */

AG_ObjectClass agObjectClass = {
	"AG_Object",
	sizeof(AG_Object),
	{ 7,3 },
	NULL,	/* init */
	NULL,	/* reset */
	NULL,	/* destroy */
	NULL,	/* load */
	NULL,	/* save */
	NULL	/* edit */
};

#ifdef AG_SERIALIZATION
const AG_Version agPropTblVer = { 2, 1 };
	
int agObjectIgnoreDataErrors = 0;  /* Don't fail on a data load failure. */
int agObjectIgnoreUnknownObjs = 0; /* Don't fail on unknown object types. */
int agObjectBackups = 1;	   /* Backup object save files. */
#endif

/* Import inlinables */
#undef AG_INLINE_HEADER
#include <agar/core/inline_object.h>

/* Initialize an AG_Object instance. */
void
AG_ObjectInit(void *pObj, void *pClass)
{
	AG_Object *ob = pObj;
	AG_ObjectClass *cl = (pClass != NULL) ? AGCLASS(pClass) : &agObjectClass;
	AG_ObjectClass **hier;
	int i, nHier;
	
#ifdef AG_TYPE_SAFETY
	Strlcpy(ob->tag, AG_OBJECT_TYPE_TAG, sizeof(ob->tag));
#endif
#ifdef AG_DEBUG
	memset(ob->name, '\0', sizeof(ob->name));
#else
	ob->name[0] = '\0';
#endif
	ob->cls = cl;
	ob->parent = NULL;
	ob->root = ob;
	ob->flags = 0;

	AG_MutexInitRecursive(&ob->lock);
	
	TAILQ_INIT(&ob->events);
#ifdef AG_TIMERS
	TAILQ_INIT(&ob->timers);
#endif
	TAILQ_INIT(&ob->vars);
	TAILQ_INIT(&ob->children);

	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) != 0) {
		AG_FatalError(NULL);
	}
	for (i = 0; i < nHier; i++) {
		if (hier[i]->init != NULL)
			hier[i]->init(ob);
	}
	free(hier);
}

/* Initialize an AG_Object instance (and set the STATIC flag on it). */
void
AG_ObjectInitStatic(void *obj, void *C)
{
	AG_ObjectInit(obj, C);
	OBJECT(obj)->flags |= AG_OBJECT_STATIC;
}

/*
 * Allocate, initialize and attach a new object instance of the specified
 * class.
 */
void *
AG_ObjectNew(void *parent, const char *name, AG_ObjectClass *C)
{
	char nameGen[AG_OBJECT_NAME_MAX];
	AG_Object *obj;

	if (name == NULL) {
		AG_ObjectGenName(parent, C, nameGen, sizeof(nameGen));
	} else {
		if (parent != NULL &&
		    AG_ObjectFindChild(parent, name) != NULL) {
			AG_SetErrorV("E7", _("Existing child object"));
			return (NULL);
		}
	}

	if ((obj = TryMalloc(C->size)) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(obj, C);
	AG_ObjectSetNameS(obj, (name != NULL) ? name : (const char *)nameGen);
	if (parent != NULL) {
		AG_ObjectAttach(parent, obj);
	}
	return (obj);
}

/*
 * Restore an object to an initial state prior to deserialization or release.
 */
void
AG_ObjectReset(void *p)
{
	AG_Object *ob = p;
	AG_ObjectClass **hier;
	int i, nHier;

	AG_ObjectLock(ob);

	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) != 0) {
		AG_FatalError(NULL);
	}
	for (i = nHier-1; i >= 0; i--) {
		if (hier[i]->reset != NULL)
			hier[i]->reset(ob);
	}
	AG_ObjectUnlock(ob);

	free(hier);
}

#if AG_MODEL != AG_SMALL
/*
 * Recursive function to construct absolute object names.
 * The Object and its parent VFS must be locked.
 */
static int
GenerateObjectPath(void *_Nonnull obj, char *_Nonnull path, AG_Size path_len)
{
	AG_Object *ob = obj;
	AG_Size name_len, cur_len;
	int rv = 0;

	cur_len = strlen(path);
	name_len = strlen(ob->name);
	
	if (name_len+cur_len+1 > path_len) {
		AG_SetErrorV("E4", _("Path buffer overflow"));
		return (-1);
	}
	
	/* Prepend separator, object name. */
	memmove(&path[name_len+1], path, cur_len+1);    /* Move the NUL as well */
	path[0] = AG_PATHSEPCHAR;
	memcpy(&path[1], ob->name, name_len);		/* Omit the NUL */

	if (ob->parent != ob->root && ob->parent != NULL) {
		rv = GenerateObjectPath(ob->parent, path, path_len);
	}
	return (rv);
}

/*
 * Copy the absolute pathname of an object to a fixed-size buffer.
 * Buffer size must be at least 2 bytes in size.
 */
int
AG_ObjectCopyName(void *obj, char *path, AG_Size path_len)
{
	AG_Object *ob = obj;
	int rv = 0;

	if (path_len < 2) {
		AG_SetErrorV("E5", _("Path buffer overflow"));
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
 * Return the name of the class which obj belongs to.
 * 
 * If full=0, return only the last subclass (eg. "AG_Button").
 * If full=1, return the full inheritance hierarchy (eg. "AG_Widget:AG_Button").
 * The caller should free(3) the returned string after use.
 */
char *
AG_ObjectGetClassName(const void *obj, int full)
{
	const AG_Object *ob = obj;

	return Strdup(full ? AGOBJECT_CLASS(ob)->hier :
	                     AGOBJECT_CLASS(ob)->name);
}

/*
 * Return the fullpath of an object (relative to the root of its parent VFS).
 *
 * The returned path is true for as long as the VFS remains locked (the
 * caller may wish to use AG_LockVFS() to guarantee atomicity of operations
 * which are dependent on the returned path being true).
 *
 * The caller should free(3) the returned string after use.
 *
 * This routine uses recursion. It will fail and return NULL if insufficient
 * memory is available to construct the complete path.
 */
char *
AG_ObjectGetName(void *obj)
{
	AG_Object *ob = obj;
	AG_Object *pob;
	char *path;
	AG_Size pathLen = 1;
	
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
#endif /* !AG_SMALL */

#ifdef AG_SERIALIZATION
/*
 * Search an object and its children for a dependency upon robj.
 * The object's VFS must be locked.
 */
static int _Pure_Attribute_If_Unthreaded
FindObjectInUse(void *_Nonnull obj, void *_Nonnull robj)
{
	AG_Object *ob = obj;
	AG_Variable *V;

	AG_ObjectLock(ob);
	TAILQ_FOREACH(V, &ob->vars, vars) {
		switch (V->type) {
		case AG_VARIABLE_P_OBJECT:
		case AG_VARIABLE_P_VARIABLE:
			if (V->data.p == robj) {
				goto used;
			}
			break;
		default:
			break;
		}
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
#endif /* AG_SERIALIZATION */

/* Set a variable which is a pointer to a function (with optional arguments). */
void
AG_SetFn(void *p, const char *key, AG_EventFn fn, const char *fmt, ...)
{
	AG_Object *obj = p;
	AG_Event *ev;

	AG_ObjectLock(obj);
	if (fn != NULL) {
		ev = AG_EventNew(fn, obj, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(ev, fmt, ap);
			va_end(ap);
		}
		AG_SetPointer(obj, key, ev);
	} else {
		AG_Unset(obj, key);
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
#ifdef AG_DEBUG
	if (parent == chld)
		AG_FatalErrorV("E31", "parent == chld");
#endif
#ifdef AG_TYPE_SAFETY
	if (!AG_OBJECT_VALID(parent)) { AG_FatalErrorV("E38a", "Parent object is invalid"); }
	if (!AG_OBJECT_VALID(chld))   { AG_FatalErrorV("E38b", "Child object is invalid"); }
#endif
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
	if (AG_Defined(chld, "attach-fn")) {
		AG_Event *ev;
		if ((ev = AG_GetPointer(chld,"attach-fn")) != NULL &&
		     ev->fn != NULL) {
			ev->fn(ev);
		}
		goto out;
	}

	/* Name the object if it has the name-on-attach flag set. */
	if (chld->flags & AG_OBJECT_NAME_ONATTACH) {
		AG_ObjectGenName(parent, chld->cls, chld->name,
		    sizeof(chld->name));
	}
	
	/* Attach the object. */
	TAILQ_INSERT_TAIL(&parent->children, chld, cobjs);
      
	/* Notify the child object. */
	AG_PostEvent(chld, "attached", "%p", parent);

#ifdef DEBUG_OBJECT
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
#endif /* DEBUG_OBJECT */

out:
	AG_ObjectUnlock(chld);
	AG_ObjectUnlock(parent);
	AG_UnlockVFS(parent);
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
#ifdef AG_TIMERS
	AG_Timer *to, *toNext;
#endif
#ifdef AG_TYPE_SAFETY
	if (!AG_OBJECT_VALID(chld))   { AG_FatalErrorV("E36a", "Child object is invalid"); }
	if (!AG_OBJECT_VALID(parent)) { AG_FatalErrorV("E36b", "Parent object is invalid"); }
#endif
#ifdef AG_THREADS
	AG_LockVFS(root);
#endif
	AG_ObjectLock(parent);
	AG_ObjectLock(chld);
	
	/* Call the detach function if one is defined. */
	if (AG_Defined(chld, "detach-fn")) {
		AG_Event *ev;
		if ((ev = AG_GetPointer(chld,"detach-fn")) != NULL &&
		     ev->fn != NULL) {
			ev->fn(ev);
		}
		goto out;
	}
#ifdef AG_TIMERS
	/* Cancel any running timer associated with the object. */
	AG_LockTiming();
	for (to = TAILQ_FIRST(&chld->timers);
	     to != TAILQ_END(&chld->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, pvt.timers);
		AG_DelTimer(chld, to);
	}
	AG_UnlockTiming();
#endif
	AG_PostEvent(chld, "detached", "%p", parent);
	
	/* Detach the object. */
	TAILQ_REMOVE(&parent->children, chld, cobjs);
	chld->parent = NULL;
	chld->root = chld;

#ifdef DEBUG_OBJECT
	if (chld->name[0] != '\0') {
		Debug(parent, "Detached child: %s\n", chld->name);
	} else {
		Debug(parent, "Detached child: <%p>\n", chld);
	}
	if (parent->name[0] != '\0') {
		Debug(chld, "New parent: NULL\n");
	}
#endif /* DEBUG_OBJECT */

out:
	AG_ObjectUnlock(chld);
	AG_ObjectUnlock(parent);
#ifdef AG_THREADS
	AG_UnlockVFS(root);
#endif
}

/* Traverse the object tree using a pathname. */
static void *_Nullable _Pure_Attribute
FindObjectByName(const AG_Object *_Nonnull parent, const char *_Nonnull name)
{
	char chldName[AG_OBJECT_PATH_MAX];
	void *rv;
	char *s;
	AG_Object *child;

	if (Strlcpy(chldName, name, sizeof(chldName)) >= sizeof(chldName)) {
		AG_SetErrorS(_("Path overflow"));
		return (NULL);
	}

	if ((s = strchr(chldName, AG_PATHSEPCHAR)) != NULL) {
		*s = '\0';
	}
	TAILQ_FOREACH(child, &parent->children, cobjs) {
		if (strcmp(child->name, chldName) != 0)
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
 * Search for the named object (absolute path).
 *
 * Returned pointer is guaranteed to be valid as long as the VFS is locked.
 * If no such object exists, return NULL (without SetError).
 */
void *
AG_ObjectFindS(void *vfsRoot, const char *name)
{
	void *rv;

#ifdef AG_DEBUG
	if (name[0] != AG_PATHSEPCHAR)
		AG_FatalErrorV("E32", "Not an absolute path");
#endif
	if (name[0] == AG_PATHSEPCHAR && name[1] == '\0') {
		return (vfsRoot);
	}
	AG_LockVFS(vfsRoot);
	rv = FindObjectByName(vfsRoot, &name[1]);
	AG_UnlockVFS(vfsRoot);
	return (rv);
}

/*
 * Search for the named object (absolute path as format string).
 *
 * Returned pointer is guaranteed to be valid as long as the VFS is locked.
 * If no such object exists, return NULL (without SetError).
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
		AG_FatalErrorV("E32", "Not an absolute path");
#endif
	AG_LockVFS(vfsRoot);
	rv = FindObjectByName(vfsRoot, &path[1]);
	AG_UnlockVFS(vfsRoot);
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

/* Detach and destroy all child objects of a given parent object. */
void
AG_ObjectFreeChildren(void *obj)
{
	AG_Object *parent = obj;
	AG_Object *child, *childNext;

	AG_ObjectLock(parent);
	for (child = TAILQ_FIRST(&parent->children);
	     child != TAILQ_END(&parent->children);
	     child = childNext) {
		childNext = TAILQ_NEXT(child, cobjs);
#ifdef DEBUG_OBJECT
		if (child->name[0] != '\0') {
			Debug(parent, "Freeing child: %s\n", child->name);
		} else {
			Debug(parent, "Freeing child: <%p>\n", child);
		}
#endif
		AG_ObjectDetach(child);
		AG_ObjectDestroy(child);
	}
	TAILQ_INIT(&parent->children);
	AG_ObjectUnlock(parent);
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
	int i, nHier;

#ifdef AG_TYPE_SAFETY
	if (!AG_OBJECT_VALID(ob))
		AG_FatalErrorV("E36", "Object is invalid");
#endif
#ifdef AG_DEBUG
	if (ob->parent != NULL) {
		AG_Debug(ob, "I'm still attached to %s\n", OBJECT(ob->parent)->name);
		AG_FatalErrorV("E33", "Object is still attached");
	}
#endif
	AG_ObjectFreeChildren(ob);
	AG_ObjectReset(ob);
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) != 0) {
		AG_FatalError(NULL);
	}
	for (i = nHier-1; i >= 0; i--) {
		if (hier[i]->destroy != NULL)
			hier[i]->destroy(ob);
	}
	free(hier);
	
	AG_ObjectFreeVariables(ob);
	AG_ObjectFreeEvents(ob);
	AG_MutexDestroy(&ob->lock);
#ifdef AG_TYPE_SAFETY
	memcpy(ob->tag, "FreeMem", 7);
# ifdef AG_DEBUG
	memcpy(ob->name, "invalid", 7);
	ob->cls = NULL;
# endif
#endif
	if ((ob->flags & AG_OBJECT_STATIC) == 0)
		free(ob);
}

#ifdef AG_SERIALIZATION
/*
 * Search the AG_CONFIG_PATH_DATA path group for an object file matching
 * "<obj-name>.<obj-ext>" and return its fullpath into a fixed-size buffer.
 *
 * If set, the special AG_Object variable "archive-path" will override the
 * default path and arrange for the object to be loaded from a specific file.
 *
 * Under threads the returned path is only valid as long as the VFS is locked.
 * This function will clobber path even when it fails and returns -1.
 */
int
AG_ObjectCopyFilename(void *p, char *path, AG_Size pathSize)
{
	char name[AG_OBJECT_PATH_MAX];
	AG_ConfigPathQ *pathGroup = &agConfig->paths[AG_CONFIG_PATH_DATA];
	AG_ConfigPath *loadPath;
	const char *archivePath;
	AG_Object *ob = p;

	AG_ObjectLock(ob);
	if (AG_Defined(ob, "archive-path") &&
	    (archivePath = AG_GetStringP(ob,"archive-path")) != NULL &&
	     archivePath[0] != '\0') {
		Strlcpy(path, archivePath, pathSize);
		goto out;
	}

	AG_ObjectCopyName(ob, name, sizeof(name));

	TAILQ_FOREACH(loadPath, pathGroup, paths) {
	     	Strlcpy(path, loadPath->s, pathSize);
		Strlcat(path, name, pathSize);
		Strlcat(path, AG_PATHSEP, pathSize);
		Strlcat(path, ob->name, pathSize);
		Strlcat(path, ".", pathSize);
		Strlcat(path, ob->cls->name, pathSize);

		/* TODO: check signature */
		if (AG_FileExists(path))
			goto out;
	}
# ifdef AG_VERBOSITY
	AG_SetError(_("File not found: %s%c%s.%s (not in load-path)"),
	    name, AG_PATHSEPCHAR, ob->name, ob->cls->name);
# else
	AG_SetErrorS("E5");
# endif
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
AG_ObjectCopyDirname(void *p, char *path, AG_Size pathSize)
{
	char tp[AG_PATHNAME_MAX];
	char name[AG_OBJECT_PATH_MAX];
	AG_ConfigPathQ *pathGroup = &agConfig->paths[AG_CONFIG_PATH_DATA];
	AG_ConfigPath *loadPath;
	AG_Object *ob = p;

	AG_ObjectLock(ob);
	AG_ObjectCopyName(ob, name, sizeof(name));

	TAILQ_FOREACH(loadPath, pathGroup, paths) {
	     	Strlcpy(tp, loadPath->s, sizeof(tp));
		Strlcat(tp, name, sizeof(tp));

		/* TODO: check if directory */
		/* TODO: check directory contents and signature */
		if (AG_FileExists(tp)) {
			Strlcpy(path, tp, pathSize);
			goto out;
		}
	}
#ifdef AG_VERBOSITY
	AG_SetError(_("The %s directory is not in load-path."), name);
#else
	AG_SetErrorS("E6");
#endif
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
		AG_Event *ev;

		if ((ev = AG_FindEventHandler(ob->root, "object-page-out"))) {
			AG_PostEventByPtr(ob->root, ev, "%p", ob);
		} else {
			if (AG_ObjectSave(ob) == -1)
				goto fail;
		}
		if ((ob->flags & AG_OBJECT_REMAIN_DATA) == 0) {
			AG_ObjectReset(ob);
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

int
AG_ObjectLoad(void *p)
{
	return AG_ObjectLoadFromFile(p, NULL);
}
int
AG_ObjectLoadData(void *p, int *dataFound)
{
	return AG_ObjectLoadDataFromFile(p, dataFound, NULL);
}
int AG_ObjectLoadGeneric(void *p)
{
	return AG_ObjectLoadGenericFromFile(p, NULL);
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
#ifdef AG_ENABLE_DSO
		AG_CopyString(cs->libs, ds, sizeof(cs->libs));
#else
		AG_SkipString(ds);
#endif
		Strlcpy(cs->spec, cs->hier, sizeof(cs->spec));
#ifdef AG_ENABLE_DSO
		if (cs->libs[0] != '\0') {
			Strlcat(cs->spec, "@", sizeof(cs->spec));
			Strlcat(cs->spec, cs->libs, sizeof(cs->spec));
		}
#endif
		if ((c = strrchr(cs->hier, ':')) != NULL && c[1] != '\0') {
			Strlcpy(cs->name, &c[1], sizeof(cs->name));
		} else {
			Strlcpy(cs->name, cs->hier, sizeof(cs->name));
		}
	} else {
		oh->cs.hier[0] = '\0';
		oh->cs.spec[0] = '\0';
		oh->cs.name[0] = '\0';
#ifdef AG_ENABLE_DSO
		oh->cs.libs[0] = '\0';
#endif
	}

	/* Dataset start offset */
	oh->dataOffs = AG_ReadUint32(ds);

	/* Object flags */
	oh->flags = (Uint)AG_ReadUint32(ds);

	return (0);
}

/* Load Object variables. */
int
AG_ObjectLoadVariables(void *p, AG_DataSource *ds)
{
	AG_Object *ob = p;
	Uint count, i, j;

	/* TODO 2.0 remove this redundant signature */
	if (AG_ReadVersion(ds, "AG_PropTbl", &agPropTblVer, NULL) == -1)
		return (-1);

	AG_ObjectLock(ob);

	if (ob->flags & AG_OBJECT_FLOATING_VARS)
		AG_ObjectFreeVariables(ob);

	count = (Uint)AG_ReadUint32(ds);
#if AG_MODEL != AG_SMALL
	if (count > AG_OBJECT_MAX_VARIABLES) {
		AG_SetErrorS(_("Too many variables"));
		return (-1);
	}
#endif
	for (i = 0; i < count; i++) {
		char key[64];
		Sint8 code;
		char *s;

		if (AG_CopyString(key, ds, sizeof(key)) >= sizeof(key)) {
#ifdef AG_VERBOSITY
			AG_SetError(_("Variable name too long: %s"), key);
#else
			AG_SetErrorS("E8");
#endif
			goto fail;
		}
		code = (Sint8)AG_ReadSint32(ds);		/* XXX */
#if 0
		Verbose("%s: %s (code %d)\n", ob->name, key, (int)code);
#endif
		for (j = 0; j < AG_VARIABLE_TYPE_LAST; j++) {
			if (agVariableTypes[j].code == code)
				break;
		}
		if (j == AG_VARIABLE_TYPE_LAST) {
#ifdef AG_VERBOSITY
			AG_SetError(_("Unknown variable code: %u"), (Uint)code);
#else
			AG_SetErrorS("E9");
#endif
			goto fail;
		}
		switch (agVariableTypes[j].typeTgt) {
#if AG_MODEL == AG_SMALL
		case AG_VARIABLE_UINT:   AG_SetUint(ob,  key, AG_ReadUint16(ds));	break;
		case AG_VARIABLE_INT:    AG_SetInt(ob,   key, AG_ReadSint16(ds));	break;
#else
		case AG_VARIABLE_UINT:   AG_SetUint(ob,  key,  (Uint)AG_ReadUint32(ds)); break;
		case AG_VARIABLE_INT:    AG_SetInt(ob,   key,   (int)AG_ReadSint32(ds)); break;
		case AG_VARIABLE_ULONG:  AG_SetUlong(ob, key, (Ulong)AG_ReadUint64(ds)); break;
		case AG_VARIABLE_LONG:   AG_SetLong(ob,  key,  (long)AG_ReadSint64(ds)); break;
		case AG_VARIABLE_UINT32: AG_SetUint32(ob, key, AG_ReadUint32(ds));	break;
		case AG_VARIABLE_SINT32: AG_SetSint32(ob, key, AG_ReadSint32(ds));	break;
#endif /* MD or LG */
		case AG_VARIABLE_UINT8:  AG_SetUint8(ob,  key, AG_ReadUint8(ds));	break;
		case AG_VARIABLE_SINT8:  AG_SetSint8(ob,  key, AG_ReadSint8(ds));	break;
		case AG_VARIABLE_UINT16: AG_SetUint16(ob, key, AG_ReadUint16(ds));	break;
		case AG_VARIABLE_SINT16: AG_SetSint16(ob, key, AG_ReadSint16(ds));	break;
#ifdef AG_HAVE_64BIT
		case AG_VARIABLE_UINT64: AG_SetUint64(ob, key, AG_ReadUint64(ds));	break;
		case AG_VARIABLE_SINT64: AG_SetSint64(ob, key, AG_ReadSint64(ds));	break;
#endif
#ifdef AG_HAVE_FLOAT
		case AG_VARIABLE_FLOAT:  AG_SetFloat(ob,  key, AG_ReadFloat(ds));	break;
		case AG_VARIABLE_DOUBLE: AG_SetDouble(ob, key, AG_ReadDouble(ds));	break;
#endif
		case AG_VARIABLE_STRING:
			if ((s = AG_ReadString(ds)) != NULL) {
				AG_SetStringNODUP(ob, key, s);
			} else {
				AG_SetString(ob, key, "");
			}
			break;
		default:
#ifdef AG_VERBOSITY
			AG_SetError(_("Attempt to load variable of type %s"),
			    agVariableTypes[j].name);
#else
			AG_SetErrorS("E10");
#endif
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
	AG_Offset countOffs;
	Uint32 count = 0;
	AG_Variable *V;

	/* TODO 2.0 remove this redundant signature */
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
		AG_WriteString(ds, (char *)V->name);
		AG_WriteSint32(ds, (Sint32)Vt->code);		/* XXX */

		p = (agVariableTypes[V->type].indirLvl > 0) ?
		    V->data.p : (void *)&V->data;

		switch (AG_VARIABLE_TYPE(V)) {
#if AG_MODEL == AG_SMALL
		case AG_VARIABLE_UINT:	 AG_WriteUint16(ds, *(Uint *)p);	break;
		case AG_VARIABLE_INT:	 AG_WriteSint16(ds, *(int *)p);		break;
#else /* MEDIUM or LARGE */
		case AG_VARIABLE_UINT:	 AG_WriteUint32(ds, (Uint32)*(Uint *)p);	break;
		case AG_VARIABLE_INT:	 AG_WriteSint32(ds, (Sint32)*(int *)p);		break;
		case AG_VARIABLE_ULONG:	 AG_WriteUint64(ds, (Uint64)*(Ulong *)p);	break;
		case AG_VARIABLE_LONG:	 AG_WriteSint64(ds, (Sint64)*(long *)p);	break;
#endif
		case AG_VARIABLE_UINT8:	 AG_WriteUint8(ds, *(Uint8 *)p);		break;
		case AG_VARIABLE_SINT8:	 AG_WriteSint8(ds, *(Sint8 *)p);		break;
		case AG_VARIABLE_UINT16: AG_WriteUint16(ds, *(Uint16 *)p);		break;
		case AG_VARIABLE_SINT16: AG_WriteSint16(ds, *(Sint16 *)p);		break;
#if AG_MODEL != AG_SMALL
		case AG_VARIABLE_UINT32: AG_WriteUint32(ds, *(Uint32 *)p);		break;
		case AG_VARIABLE_SINT32: AG_WriteSint32(ds, *(Sint32 *)p);		break;
#endif
#ifdef AG_HAVE_64BIT
		case AG_VARIABLE_UINT64: AG_WriteUint64(ds, *(Uint64 *)p);		break;
		case AG_VARIABLE_SINT64: AG_WriteSint64(ds, *(Sint64 *)p);		break;
#endif
#ifdef AG_HAVE_FLOAT
		case AG_VARIABLE_FLOAT:  AG_WriteFloat(ds, *(float *)p);		break;
		case AG_VARIABLE_DOUBLE: AG_WriteDouble(ds, *(double *)p);		break;
#endif
		case AG_VARIABLE_STRING: AG_WriteString(ds, V->data.s);			break;
		default: break;
		}
		AG_UnlockVariable(V);

		if (++count > AG_OBJECT_MAX_VARIABLES) {
			AG_FatalErrorV("E34", "Too many variables to save");
		/*	break; */
		}
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
#if AG_MODEL == AG_SMALL
	AG_ObjectHeader *oh;
#else
	AG_ObjectHeader oh;
#endif
	char path[AG_PATHNAME_MAX];
	AG_DataSource *ds;
	Uint32 count, i;
	
	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetErrorV("E11", _("Object is non-persistent"));
		return (-1);
	}
	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else {
		if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1)
			goto fail_unlock;
	}
#ifdef DEBUG_SERIALIZATION
	Debug(ob, "Loading generic data from %s\n", path);
#endif
	if ((ds = AG_OpenFile(path, "rb")) == NULL)
		goto fail_unlock;

	/* Free any resident dataset in order to clear the dependencies. */
	AG_ObjectReset(ob);

#if AG_MODEL == AG_SMALL
	if ((oh = TryMalloc(sizeof(AG_ObjectHeader))) == NULL) {
		goto fail;
	}
	if (AG_ObjectReadHeader(ds, oh) == -1) {
		goto fail;
	}
	ob->flags &= ~(AG_OBJECT_SAVED_FLAGS);
	ob->flags |= oh->flags;
	free(oh);
#else
	if (AG_ObjectReadHeader(ds, &oh) == -1) {
		goto fail;
	}
	ob->flags &= ~(AG_OBJECT_SAVED_FLAGS);
	ob->flags |= oh.flags;
#endif
	/* Skip over legacy (pre-1.6) dependency table */
	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++)
		AG_SkipString(ds);

	/* Load the set of Variables */
	if (AG_ObjectLoadVariables(ob, ds) == -1)
		goto fail;
	
	/* Load the generic part of the archived child objects. */
	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		char cname[AG_OBJECT_NAME_MAX];
		char hier[AG_OBJECT_HIER_MAX];
		AG_Object *chld;
		AG_ObjectClass *C;

	 	/* TODO check that there are no duplicate names. */
		AG_CopyString(cname, ds, sizeof(cname));
		AG_CopyString(hier, ds, sizeof(hier));

		/* Look for an existing object of the given name. */
		if ((chld = AG_ObjectFindChild(ob, cname)) != NULL) {
			if (strcmp(chld->cls->hier, hier) != 0) {
#ifdef AG_VERBOSITY
				AG_SetError(_("Archived object `%s' clashes with "
				              "existing object of different type"),
					      cname);
#else
				AG_SetErrorS("E12");
#endif
				goto fail;
			}
			if (!OBJECT_PERSISTENT(chld)) {
#ifdef AG_VERBOSITY
				AG_SetError(_("Archived object `%s' clashes with "
				              "existing non-persistent object"),
					      cname);
#else
				AG_SetErrorS("E13");
#endif
				goto fail;
			}
			if (AG_ObjectLoadGeneric(chld) == -1) {
				goto fail;
			}
			continue;
		}

		/* Create a new child object. */
#ifdef AG_ENABLE_DSO
		C = AG_LoadClass(hier);
#else
		C = AG_LookupClass(hier);
#endif
		if (C == NULL) {
#ifdef AG_VERBOSITY
			AG_SetError("%s: %s", ob->name, AG_GetError());
#else
			AG_SetErrorS("E14");
#endif
			if (agObjectIgnoreUnknownObjs) {
#ifdef DEBUG_SERIALIZATION
				Debug(ob, "%s; ignoring\n", AG_GetError());
#endif
				continue;
			} else {
				goto fail;
			}
			goto fail;
		}
		if ((chld = TryMalloc(C->size)) == NULL) {
			goto fail;
		}
		AG_ObjectInit(chld, C);
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
	AG_ObjectReset(ob);
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
		if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1) {
			*dataFound = 0;
			goto fail_unlock;
		}
	}
#ifdef DEBUG_SERIALIZATION
	Debug(ob, "Loading dataset from %s\n", path);
#endif
	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		*dataFound = 0;
		goto fail_unlock;
	}
	if (AG_ObjectReadHeader(ds, &oh) == -1 ||
	    AG_Seek(ds, oh.dataOffs, AG_SEEK_SET) == -1 ||
	    AG_ReadVersion(ds, ob->cls->name, &ob->cls->ver, &ver) == -1) {
		goto fail;
	}
	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
#ifdef AG_DEBUG
		AG_SetSourceDebug(ds, 1);
#else
		AG_SetErrorV("E15", _("Can't read without DEBUG"));
		goto fail;
#endif
	}
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == -1)
		goto fail;

	AG_ObjectReset(ob);

	for (i = 0; i < nHier; i++) {
#ifdef DEBUG_SERIALIZATION
		Debug(ob, "Loading as %s\n", hier[i]->name);
#endif
		if (hier[i]->load == NULL)
			continue;
		if (hier[i]->load(ob, ds, &ver) == -1) {
#ifdef AG_VERBOSITY
			AG_SetError("<0x%x>: %s", (Uint)AG_Tell(ds),
			    AG_GetError());
#else
			AG_SetErrorS("E16");
#endif
			free(hier);
			goto fail;
		}
	}
	free(hier);

	AG_CloseFile(ds);
	AG_PostEvent(ob->root, "object-post-load", "%p,%s", ob, path);
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
BackupObjectFile(const char *_Nonnull orig)
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
	AG_Offset countOffs, dataOffs;
	Uint32 count;
	AG_Object *chld;
	AG_ObjectClass **hier;
	int i, nHier;
#ifdef AG_DEBUG
	int debugSave;
#endif
	AG_ObjectLock(ob);
	
	/* Header */
	AG_WriteVersion(ds, agObjectClass.name, &agObjectClass.ver);
	AG_WriteString(ds, ob->cls->hier);
#ifdef AG_ENABLE_DSO
	AG_WriteString(ds, ob->cls->pvt.libs);
#else
	AG_WriteString(ds, "");
#endif
	dataOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);					/* Data offs */
	AG_WriteUint32(ds, (Uint32)(ob->flags & AG_OBJECT_SAVED_FLAGS));

	/* No legacy (pre-1.6) dependency table. */
	AG_WriteUint32(ds, 0);

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

#ifdef AG_DEBUG
	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
		debugSave = AG_SetSourceDebug(ds, 1); 
	} else {
		debugSave = 0;
	}
#endif
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == -1) {
		goto fail;
	}
	for (i = 0; i < nHier; i++) {
#ifdef DEBUG_SERIALIZATION
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

#ifdef AG_DEBUG
	if (ob->flags & AG_OBJECT_DEBUG_DATA) AG_SetSourceDebug(ds, debugSave);
#endif
	AG_ObjectUnlock(ob);
	return (0);
fail:
#ifdef AG_DEBUG
	if (ob->flags & AG_OBJECT_DEBUG_DATA) AG_SetSourceDebug(ds, debugSave);
#endif
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
	Uint32 count;
	int i, nHier;
#ifdef AG_DEBUG
	int debugSave;
#endif

	AG_ObjectLock(ob);

	/* Object header */
	if (AG_ObjectReadHeader(ds, &oh) == -1) {
		goto fail;
	}
	ob->flags &= ~(AG_OBJECT_SAVED_FLAGS);
	ob->flags |= oh.flags;

	/* Skip over legacy (pre-1.6) dependency table */
	count = AG_ReadUint32(ds);
	for (i = 0; i < count; i++)
		AG_SkipString(ds);

	/* Load the set of Variables */
	if (AG_ObjectLoadVariables(ob, ds) == -1)
		goto fail;

	/* Table of child objects, expected empty. */
	if (AG_ReadUint32(ds) != 0) {
		AG_SetErrorV("E17", "nChildren != 0");
		goto fail;
	}

	/* Dataset */
	if (AG_ReadVersion(ds, ob->cls->name, &ob->cls->ver, &ver) == -1)
		goto fail;

	if (ob->flags & AG_OBJECT_DEBUG_DATA) {
#ifdef AG_DEBUG
		debugSave = AG_SetSourceDebug(ds, 1);
#else
		AG_SetErrorV("E15", _("Can't read without DEBUG"));
		goto fail;
#endif
	} else {
#ifdef AG_DEBUG
		debugSave = 0;
#endif
	}
	if (AG_ObjectGetInheritHier(ob, &hier, &nHier) == -1) {
		goto fail_dbg;
	}
	for (i = 0; i < nHier; i++) {
#ifdef DEBUG_SERIALIZATION
		Debug(ob, "Loading as %s\n", hier[i]->name);
#endif
		if (hier[i]->load == NULL)
			continue;
		if (hier[i]->load(ob, ds, &ver) == -1) {
#ifdef AG_VERBOSITY
			AG_SetError("<0x%x>: %s", (Uint)AG_Tell(ds), AG_GetError());
#else
			AG_SetErrorS("E18");
#endif
			free(hier);
			goto fail_dbg;
		}
	}
	free(hier);

#ifdef AG_DEBUG
	if (ob->flags & AG_OBJECT_DEBUG_DATA) AG_SetSourceDebug(ds, debugSave);
#endif
	AG_ObjectUnlock(ob);
	return (0);
fail_dbg:
#ifdef AG_DEBUG
	if (ob->flags & AG_OBJECT_DEBUG_DATA) AG_SetSourceDebug(ds, debugSave);
#endif
fail:
	AG_ObjectReset(ob);
	AG_ObjectUnlock(ob);
	return (-1);
}

/* Archive an object to a file. */
int
AG_ObjectSaveToFile(void *p, const char *pPath)
{
	char dirPath[AG_PATHNAME_MAX];
	char path[AG_PATHNAME_MAX];
	char name[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	AG_DataSource *ds;
	int hasArchivePath;

	AG_LockVFS(ob);
	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob)) {
		AG_SetErrorV("E19", _("Non-persistent object"));
		goto fail_unlock;
	}

	AG_ObjectCopyName(ob, name, sizeof(name));
	hasArchivePath = AG_Defined(ob, "archive-path");

	if (pPath != NULL) {
		Strlcpy(path, pPath, sizeof(path));
	} else if (!hasArchivePath) {
		/*
		 * Create the save directory if needed (but never do this
		 * if an archive-path is set).
		 */
		if (AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, dirPath, sizeof(dirPath)) >= sizeof(dirPath) ||
		    Strlcat(dirPath, name, sizeof(dirPath)) >= sizeof(dirPath)) {
			AG_SetErrorV("E4", _("Path overflow"));
			goto fail_unlock;
		}
		if (AG_FileExists(dirPath) == 0 &&
		    AG_MkPath(dirPath) == -1)
			goto fail_unlock;
	}

	if (pPath == NULL) {
		if (hasArchivePath) {
			AG_GetString(ob, "archive-path", path, sizeof(path));
		} else {
			Strlcpy(path, dirPath, sizeof(path));
			Strlcat(path, AG_PATHSEP, sizeof(path));
			Strlcat(path, ob->name, sizeof(path));
			Strlcat(path, ".", sizeof(path));
			Strlcat(path, ob->cls->name, sizeof(path));
		}
	}
#ifdef DEBUG_SERIALIZATION
	Debug(ob, "Saving object to %s\n", path);
#endif
	if (agObjectBackups) {
		BackupObjectFile(path);
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

/* Shorthand for AG_ObjectSaveToFile() */
int
AG_ObjectSave(void *p)
{
	return AG_ObjectSaveToFile(p, NULL);
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

#endif /* AG_SERIALIZATION */

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
	if (name == NULL) {
		ob->name[0] = '\0';
	} else {
#ifdef AG_DEBUG
		if (Strlcpy(ob->name, name, sizeof(ob->name)) >= sizeof(ob->name))
			Verbose("Truncated object name: \"%s\"", ob->name);
#else
		Strlcpy(ob->name, name, sizeof(ob->name));
#endif
		for (c = &ob->name[0]; *c != '\0'; c++) {
			if (*c == '/' || *c == '\\')		/* Pathname separator */
				*c = '_';
		}
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

#if AG_MODEL != AG_SMALL
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
#endif /* !AG_SMALL */

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

#ifdef AG_SERIALIZATION
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
#if AG_MODEL == AG_SMALL
	char *bufCur = Malloc(AG_BUFFER_MAX);
	char *bufLast = Malloc(AG_BUFFER_MAX);
#else
	char bufCur[AG_BUFFER_MAX];
	char bufLast[AG_BUFFER_MAX];
#endif
	char pathCur[AG_PATHNAME_MAX];
	char pathLast[AG_PATHNAME_MAX];
	AG_Object *ob = p;
	FILE *fLast, *fCur;
	AG_Size rvLast, rvCur;
	int rv = 0;

	AG_ObjectLock(ob);

	if (!OBJECT_PERSISTENT(ob)) {
		goto out;
	}
	AG_ObjectCopyFilename(ob, pathLast, sizeof(pathLast));
	if ((fLast = fopen(pathLast, "r")) == NULL) {
		rv = 1;
		goto out;
	}
	AG_ConfigGetPath(AG_CONFIG_PATH_TEMP, 0, pathCur, sizeof(pathCur));
	Strlcat(pathCur, AG_PATHSEP, sizeof(pathCur));
	Strlcat(pathCur, "_chg.", sizeof(pathCur));
	Strlcat(pathCur, ob->name, sizeof(pathCur));
	if (AG_ObjectSaveToFile(ob, pathCur) == -1) {
		fclose(fLast);
		rv = 1;
		goto out;
	}
	if ((fCur = fopen(pathCur, "r")) == NULL) {
		fclose(fLast);
		rv = 1;
		goto out;
	}
	for (;;) {
		rvLast = fread(bufLast, 1, sizeof(bufLast), fLast);
		rvCur = fread(bufCur, 1, sizeof(bufCur), fCur);
	
		if (rvLast != rvCur ||
		   (rvLast > 0 && memcmp(bufLast, bufCur, rvLast) != 0)) {
			rv = 1;
			break;
		}
		if (feof(fLast)) {
			if (!feof(fCur)) { rv = 1; }
			break;
		}
		if (feof(fCur)) {
			if (!feof(fLast)) { rv = 1; }
			break;
		}
	}
	AG_FileDelete(pathCur);
	fclose(fCur);
	fclose(fLast);
out:
	AG_ObjectUnlock(ob);
#if AG_MODEL == AG_SMALL
	free(bufCur);
	free(bufLast);
#endif
	return (rv);
}
#endif /* AG_SERIALIZATION */

/*
 * Generate an object name that is unique in the given parent object. The
 * name is only guaranteed to remain unique as long as the VFS and parent
 * object are locked.
 */
void
AG_ObjectGenName(void *p, AG_ObjectClass *C, char *name, AG_Size len)
{
	AG_Object *pobj = p, *chld;
	char *ccBase, *cc, *dBase;
	Uint i;

	if ((ccBase = strchr(C->name, '_')) != NULL) {	/* Skip any prefix */
		ccBase++;
	} else {
		ccBase = C->name;
	}
	if (len < 4) {
		return;
	}
	name[0] = tolower(ccBase[0]);
	len--;
	for (cc = &ccBase[1], dBase = &name[1];
	    *cc != '\0' && len > 0;
	     cc++, dBase++) {
		*dBase = *cc;
		len--;
	}
	*dBase = '\0';
	i = 0;
tryname:
	StrlcpyUint(dBase, i, len);
	if (pobj != NULL) {
		AG_LockVFS(pobj);
		TAILQ_FOREACH(chld, &pobj->children, cobjs) {
			if (strcmp(chld->name, name) == 0)
				break;
		}
		AG_UnlockVFS(pobj);

		if (chld != NULL) {
			i++;
			goto tryname;
		}
	}
}

#if AG_MODEL != AG_SMALL
/* Generate a unique object name using the specified prefix. */
void
AG_ObjectGenNamePfx(void *p, const char *pfx, char *name, AG_Size len)
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
#endif /* !AG_SMALL */

#ifdef AG_LEGACY
void
AG_ObjectSetArchivePath(void *obj, const char *path)
{
	AG_SetString(obj, "archive-path", path);
}
#endif /* !AG_LEGACY */
