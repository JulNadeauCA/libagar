/*	Public domain	*/

#ifdef AG_NAMESPACES
/*
 * Lookup a registered namespace by name.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Namespace *_Nullable
AG_GetNamespace(const char *_Nonnull ns)
#else
AG_Namespace *
ag_get_namespace(const char *ns)
#endif
{
	int i;

	for (i = 0; i < agNamespaceCount; i++) {
		if (strcmp(agNamespaceTbl[i].name, ns) == 0)
			return (&agNamespaceTbl[i]);
	}
	AG_SetError("No such namespace: %s", ns);
	return (NULL);
}
#endif /* AG_NAMESPACES */

/*
 * Compare the inheritance hierarchy of a class against a given pattern.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ClassIsNamed(const void *_Nonnull pClass, const char *_Nonnull pat)
#else
int
ag_class_is_named(const void *pClass, const char *pat)
#endif
{
	AG_ObjectClass *cls = (AG_ObjectClass *)pClass;
	const char *c;
	int nwild = 0;
	AG_Size patSize;

	for (c = &pat[0]; *c != '\0'; c++) {
		if (*c == '*')
			nwild++;
	}
	if (nwild == 0) {
		return (strcmp(cls->hier, pat) == 0);
	} else if (nwild == 1) {			/* Optimized case */
		if (pat[strlen(pat)-1] == '*') {
			for (c = &pat[0]; *c != '\0'; c++) {
				if (c[0] != ':' || c[1] != '*' || c[2] != '\0')
					continue;
			
				patSize = c - &pat[0];
				if (c == &pat[0]) {
					return (1);
				}
				if (!strncmp(cls->hier, pat, patSize) &&
				    (cls->hier[patSize] == ':' ||
				     cls->hier[patSize] == '\0')) {
					return (1);
				}
			}
		} else if (pat[0] == '*') {
			return (1);
		} else {
			return AG_ClassIsNamedGeneral(cls, pat);
		}
		return (0);
	}
	return AG_ClassIsNamedGeneral(cls, pat);	/* General case */
}

/*
 * Test whether an object's class matches a given pattern.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int
AG_OfClass(const void *_Nonnull obj, const char *_Nonnull spec)
#else
int
ag_of_class(const void *obj, const char *spec)
#endif
{
	return AG_ClassIsNamed(AGOBJECT(obj)->cls, spec);
}

/*
 * Return a pointer to the root of the given object's VFS.
 * The result is valid as long as the VFS is locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Object *_Nonnull _Pure_Attribute
AG_ObjectRoot(const void *_Nonnull obj)
#else
AG_Object *
ag_object_root(const void *obj)
#endif
{
	return AGOBJECT(obj)->root;
}

/*
 * Return a pointer to the parent of the given object, if any.
 * The result is valid as long as the VFS is locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Object *_Nullable
AG_ObjectParent(const void *_Nonnull obj)
#else
AG_Object *
ag_object_parent(const void *obj)
#endif
{
	return AGOBJECT(obj)->parent;
}

/*
 * Detach and destroy an object.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ObjectDelete(void *_Nonnull pObj)
#else
void
ag_object_delete(void *pObj)
#endif
{
	AG_Object *obj = AGOBJECT(pObj);

	if (obj->parent != NULL) {
		AG_ObjectDetach(obj);
	}
	AG_ObjectDestroy(obj);
}

/*
 * Lookup a direct child object by name.
 * Result is valid as long as parent object's VFS is locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable _Pure_Attribute_If_Unthreaded
AG_ObjectFindChild(void *_Nonnull pParent, const char *_Nonnull name)
#else
void *
ag_object_find_child(void *pParent, const char *name)
#endif
{
	AG_Object *pObj = AGOBJECT(pParent);
	AG_Object *cObj;

	AG_LockVFS(pObj);
	AGOBJECT_FOREACH_CHILD(cObj, pObj, ag_object) {
		if (strcmp(cObj->name, name) == 0)
			break;
	}
	AG_UnlockVFS(pObj);
	return (cObj);
}

/* Return a pointer to the description of the superclass an object. */
#ifdef AG_INLINE_HEADER
static __inline__ AG_ObjectClass *_Nullable _Pure_Attribute
AG_ObjectSuperclass(const void *_Nonnull p)
#else
AG_ObjectClass *_Nullable _Pure_Attribute
ag_object_superclass(void *p)
#endif
{
	return AGOBJECT(p)->cls->super;
}

#ifdef AG_THREADS

/* Acquire the mutex protecting all resources owned by an object. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ObjectLock(void *_Nonnull p)
#else
void
ag_object_lock(void *p)
#endif
{
	AG_MutexLock(&AGOBJECT(p)->pvt.lock);
}

/* Release the mutex protecting all resources owned by an object. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_ObjectUnlock(void *_Nonnull p)
#else
void
ag_object_unlock(void *p)
#endif
{
	AG_MutexUnlock(&AGOBJECT(p)->pvt.lock);
}

/* Acquire the mutex protecting all resources owned by an object. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_LockVFS(void *_Nonnull p)
#else
void
ag_lock_vfs(void *p)
#endif
{
	AG_ObjectLock(AGOBJECT(p)->root);
}

/* Release the mutex protecting all resources owned by an object. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_UnlockVFS(void *_Nonnull p)
#else
void
ag_unlock_vfs(void *p)
#endif
{
	AG_ObjectUnlock(AGOBJECT(p)->root);
}

#ifdef AG_TIMERS

/* Lock the timer queue (and optionally all timers owned by a given object). */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_LockTimers(void *_Nullable p)
#else
void
ag_lock_timers(void *p)
#endif
{
	AG_Object *ob = (p != NULL) ? AGOBJECT(p) : &agTimerMgr;

	AG_ObjectLock(ob);
	AG_LockTiming();
}

/* Unlock the timer queue (and optionally all timers owned by a given object). */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_UnlockTimers(void *_Nullable p)
#else
void
ag_unlock_timers(void *p)
#endif
{
	AG_Object *ob = (p != NULL) ? AGOBJECT(p) : &agTimerMgr;

	AG_UnlockTiming();
	AG_ObjectUnlock(ob);
}
#endif /* AG_TIMERS */

#endif /* AG_THREADS */

/*
 * Evaluate whether the named object variable exists.
 * The object must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_Defined(void *_Nonnull pObj, const char *_Nonnull name)
#else
int
ag_defined(void *pObj, const char *name)
#endif
{
	AG_Object *obj = AGOBJECT(pObj);
	AG_Variable *V;

	AG_TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(name, V->name) == 0)
			return (1);
	}
	return (0);
}

/*
 * If the named variable exists, return a pointer to it.
 * If not, allocate a new one. The Object must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Variable *_Nonnull
AG_FetchVariable(void *_Nonnull pObj, const char *_Nonnull name,
    enum ag_variable_type type)
#else
AG_Variable *
ag_fetch_variable(void *pObj, const char *name, enum ag_variable_type type)
#endif
{
	AG_Object *obj = (AG_Object *)pObj;
	AG_Variable *V;

	AG_TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (V == NULL) {
		V = AG_Malloc(sizeof(AG_Variable));
		AG_InitVariable(V, type, name);
		AG_TAILQ_INSERT_TAIL(&obj->vars, V, vars);
	}
	return (V);
}

/*
 * Mutating variant of AG_FetchVariable(). If the named variable exists,
 * reinitialize it as a variable of the specified type.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Variable *_Nonnull
AG_FetchVariableOfType(void *_Nonnull obj, const char *_Nonnull name,
    enum ag_variable_type type)
#else
AG_Variable *
ag_fetch_variable_of_type(void *obj, const char *name,
    enum ag_variable_type type)
#endif
{
	AG_Variable *V;

	V = AG_FetchVariable(obj, name, type);
	if (V->type != type) {
#ifdef AG_DEBUG
		AG_Debug(obj, "Mutating \"%s\": From (%s) to (%s)\n", name,
		    agVariableTypes[V->type].name,
		    agVariableTypes[type].name);
#endif
		AG_FreeVariable(V);
		AG_InitVariable(V, type, name);
	}
	return (V);
}

/*
 * Lookup an object variable by name and return a locked AG_Variable.
 * The object must be locked.
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Variable *_Nullable _Pure_Attribute_If_Unthreaded
AG_AccessVariable(void *_Nonnull pObj, const char *_Nonnull name)
#else
AG_Variable *
ag_access_variable(void *pObj, const char *name)
#endif
{
	AG_Object *obj = AGOBJECT(pObj);
	AG_Variable *V, *Vtgt;

	AG_TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(name, V->name) == 0)
			break;
	}
	if (V == NULL) {
		return (NULL);
	}
	AG_LockVariable(V);
	if (V->type == AG_VARIABLE_P_VARIABLE) {
#if 0
		AG_Debug(obj, "Aliasing \"%s\" -> %s<%s>:\"%s\"", name,
		    AGOBJECT(V->data.p)->name,
		    AGOBJECT_CLASS(V->data.p)->name,
		    V->info.varName);
#endif
		Vtgt = AG_AccessVariable(AGOBJECT(V->data.p), V->info.varName);
		AG_UnlockVariable(V);
		return (Vtgt);
	}
	return (V);
}

/* Accessor routine for AG_OBJECT_NAMED() macro in AG_Event(3). */
#ifdef AG_INLINE_HEADER
static __inline__ void *_Nonnull _Pure_Attribute
AG_GetNamedObject(AG_Event *_Nonnull event, const char *_Nonnull key,
    const char *_Nonnull classSpec)
#else
void *
ag_get_named_object(AG_Event *event, const char *key, const char *classSpec)
#endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, key);
#ifdef AG_TYPE_SAFETY
	if (!AG_OfClass((struct ag_object *)V->data.p, classSpec)) {
		AG_FatalError("Illegal AG_OBJECT_NAMED() access");
	}
#endif
	return (V->data.p);
}
