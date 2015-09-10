/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
#define _AGAR_CORE_OBJECT_H_

#define AG_OBJECT_HIER_MAX 256
#define AG_OBJECT_TYPE_MAX 48
#define AG_OBJECT_NAME_MAX 128
#define AG_OBJECT_PATH_MAX 1024
#define AG_OBJECT_LIBS_MAX 128
#define AG_OBJECT_DIGEST_MAX 170

#define AGOBJECT(ob) ((struct ag_object *)(ob))
#define AGOBJECT_CLASS(obj) ((struct ag_object_class *)(AGOBJECT(obj)->cls))
#define AGCLASS(obj) ((struct ag_object_class *)(obj))

struct ag_object;
struct ag_db;
struct ag_dbt;

#include <agar/core/text.h>
#include <agar/core/variable.h>
#include <agar/core/event.h>
#include <agar/core/time.h>
#include <agar/core/class.h>

#include <agar/core/begin.h>

AG_TAILQ_HEAD(ag_objectq, ag_object);

/* Entry in object dependency table. */
typedef struct ag_object_dep {
	int persistent;			/* Include in archives */
	char *path;			/* Unresolved object path */
	struct ag_object *obj;		/* Resolved object */
	Uint32 count;			/* Reference count */
#define AG_OBJECT_DEP_MAX (0xffffffff-2)
	AG_TAILQ_ENTRY(ag_object_dep) deps;
} AG_ObjectDep;

/* Object instance data. */
typedef struct ag_object {
	char name[AG_OBJECT_NAME_MAX];	/* Object ID (unique in parent) */
	char *archivePath;		/* Path to archive (app-specific) */
	char *save_pfx;			/* Prefix for default save paths */
	AG_ObjectClass *cls;		/* Object class data */
	Uint flags;
#define AG_OBJECT_FLOATING_VARS	 0x00001	/* Clear variables before load */
#define AG_OBJECT_NON_PERSISTENT 0x00002	/* Never include in saves */
#define AG_OBJECT_INDESTRUCTIBLE 0x00004	/* Not destructible (advisory) */
#define AG_OBJECT_RESIDENT	 0x00008	/* Data part is resident */
#define AG_OBJECT_PRESERVE_DEPS	 0x00010	/* Preserve cnt=0 dependencies */
#define AG_OBJECT_STATIC	 0x00020	/* Don't free() after detach */
#define AG_OBJECT_READONLY	 0x00040	/* Disallow edition (advisory) */
#define AG_OBJECT_WAS_RESIDENT	 0x00080	/* Used internally by ObjectLoad() */
#define AG_OBJECT_REOPEN_ONLOAD	 0x00200	/* Recreate editor UI on ObjectLoad() */
#define AG_OBJECT_REMAIN_DATA	 0x00400	/* Keep user data resident */
#define AG_OBJECT_DEBUG		 0x00800	/* Enable debugging */
#define AG_OBJECT_NAME_ONATTACH	 0x01000	/* Generate name on attach */
#define AG_OBJECT_CHLD_AUTOSAVE	 0x02000	/* Include child obj data in archive */
#define AG_OBJECT_DEBUG_DATA	 0x04000	/* Datafiles contain debug info */
#define AG_OBJECT_INATTACH	 0x08000	/* In AG_ObjectAttach() */
#define AG_OBJECT_INDETACH	 0x10000	/* In AG_ObjectDetach() */
#define AG_OBJECT_SAVED_FLAGS	(AG_OBJECT_FLOATING_VARS|\
 				 AG_OBJECT_INDESTRUCTIBLE|\
				 AG_OBJECT_PRESERVE_DEPS|\
				 AG_OBJECT_READONLY|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA|\
				 AG_OBJECT_DEBUG)
#define AG_OBJECT_DUPED_FLAGS	(AG_OBJECT_SAVED_FLAGS|\
				 AG_OBJECT_NON_PERSISTENT|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA)

	AG_TAILQ_HEAD_(ag_event) events;	/* Event handlers / virtual fns */
	AG_TAILQ_HEAD_(ag_timer) timers;	/* Running timers */
	AG_TAILQ_HEAD_(ag_variable) vars;	/* Named variables / bindings */
	AG_TAILQ_HEAD_(ag_object_dep) deps;	/* Object dependencies */
	struct ag_objectq children;		/* Child objects */
	AG_TAILQ_ENTRY(ag_object) cobjs;	/* Entry in parent */
	AG_TAILQ_ENTRY(ag_object) tobjs;	/* Entry in timer queue */
	void *parent;			/* Parent object (NULL for VFS root) */
	void *root;			/* Pointer to VFS root */
	AG_Event *attachFn;		/* Attach hook */
	AG_Event *detachFn;		/* Detach hook */
	AG_Mutex lock;			/* General object lock */
} AG_Object;

/* Object archive header information. */
typedef struct ag_object_header {
	AG_ObjectClassSpec cs;			/* Class specification */
	Uint32 dataOffs;			/* Dataset offset */
	AG_Version ver;				/* AG_Object version */
	Uint flags;				/* Object flags */
} AG_ObjectHeader;

/* Checksum method for ObjectCopyChecksum(). */
enum ag_object_checksum_alg {
	AG_OBJECT_MD5,
	AG_OBJECT_SHA1,
	AG_OBJECT_RMD160
};

/* Iterate over the direct child objects. */
#define AGOBJECT_FOREACH_CHILD(var, ob, t) \
	for((var) = (struct t *)AG_TAILQ_FIRST(&AGOBJECT(ob)->children); \
	    (var) != (struct t *)AG_TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct t *)AG_TAILQ_NEXT(AGOBJECT(var), cobjs))

/* Return next entry in list of direct child objects. */
#define AGOBJECT_NEXT_CHILD(var,t) \
	((struct t *)AG_TAILQ_NEXT(AGOBJECT(var),cobjs))

/* Return last entry in list of direct child objects. */
#define AGOBJECT_LAST_CHILD(var,t) \
	((struct t *)AG_TAILQ_LAST(&AGOBJECT(var)->children,ag_objectq))
	
/* Iterate over the direct child objects (reverse order). */
#define AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, t) \
	for((var) = (struct t *)AG_TAILQ_LAST(&AGOBJECT(ob)->children, \
	    ag_objectq); \
	    (var) != (struct t *)AG_TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct t *)AG_TAILQ_PREV(AGOBJECT(var), ag_objectq, \
	    cobjs))

/* Iterate over the direct child objects (matching a specified class). */
#define AGOBJECT_FOREACH_CLASS(var, ob, t, subclass) \
	AGOBJECT_FOREACH_CHILD(var,ob,t) \
		if (!AG_OfClass(var,(subclass))) { \
			continue; \
		} else

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_CORE)
# define OBJECT(ob)            AGOBJECT(ob)
# define OBJECT_CLASS(ob)      AGOBJECT_CLASS(ob)
# define CLASS(ob)             AGCLASS(ob)

# define OBJECT_RESIDENT(ob)   (AGOBJECT(ob)->flags & AG_OBJECT_RESIDENT)
# define OBJECT_PERSISTENT(ob) !(AGOBJECT(ob)->flags & AG_OBJECT_NON_PERSISTENT)
# define OBJECT_DEBUG(ob)      (AGOBJECT(ob)->flags & AG_OBJECT_DEBUG)

# define OBJECT_FOREACH_CHILD(var,ob,t)			AGOBJECT_FOREACH_CHILD((var),(ob),t)
# define OBJECT_FOREACH_CHILD_REVERSE(var,ob,t)		AGOBJECT_FOREACH_CHILD_REVERSE((var),(ob),t)
# define OBJECT_FOREACH_CLASS(var,ob,t,subclass)	AGOBJECT_FOREACH_CLASS((var),(ob),t,(subclass))
# define OBJECT_NEXT_CHILD(var,t)			AGOBJECT_NEXT_CHILD((var),t)
# define OBJECT_LAST_CHILD(var,t)			AGOBJECT_LAST_CHILD((var),t)

#endif /* _AGAR_INTERNAL || _USE_AGAR_CORE */

__BEGIN_DECLS
extern AG_ObjectClass   agObjectClass;		/* Generic Object class */

void	*AG_ObjectNew(void *, const char *, AG_ObjectClass *);
void	 AG_ObjectAttach(void *, void *);
int	 AG_ObjectAttachToNamed(void *, const char *, void *);
void	 AG_ObjectDetach(void *);

void	 AG_ObjectInit(void *, void *);
void	 AG_ObjectInitStatic(void *, void *);
void	 AG_ObjectInitNamed(void *, void *, const char *);
void	 AG_ObjectFreeDataset(void *);
void	 AG_ObjectRemain(void *, Uint);
char    *AG_ObjectGetName(void *);
int	 AG_ObjectCopyName(void *, char *, size_t)
	                   BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 AG_ObjectCopyDirname(void *, char *, size_t)
	                      BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 AG_ObjectCopyFilename(void *, char *, size_t)
	                       BOUNDED_ATTRIBUTE(__string__, 2, 3);
size_t	 AG_ObjectCopyChecksum(void *, enum ag_object_checksum_alg, char *);
int	 AG_ObjectCopyDigest(void *, size_t *, char *);
int	 AG_ObjectChanged(void *);
int	 AG_ObjectChangedAll(void *);

#define AG_ObjectRoot(ob) (AGOBJECT(ob)->root)
#define AG_ObjectParent(ob) (AGOBJECT(ob)->parent)

void	*AG_ObjectFindS(void *, const char *);
void	*AG_ObjectFind(void *, const char *, ...)
	                FORMAT_ATTRIBUTE(printf, 2, 3);
void	*AG_ObjectFindParent(void *, const char *, const char *);

int	 AG_ObjectInUse(void *);
void	 AG_ObjectSetName(void *, const char *, ...)
	                  FORMAT_ATTRIBUTE(printf, 2, 3);
void	 AG_ObjectSetNameS(void *, const char *);
void	 AG_ObjectSetArchivePath(void *, const char *);
void	 AG_ObjectGetArchivePath(void *, char *, size_t)
	                         BOUNDED_ATTRIBUTE(__string__, 2, 3);
void	 AG_ObjectSetClass(void *, void *);

void	 AG_ObjectSetAttachFn(void *, void (*fn)(AG_Event *), const char *, ...);
void	 AG_ObjectSetDetachFn(void *, void (*fn)(AG_Event *), const char *, ...);

void	 AG_ObjectMoveUp(void *);
void	 AG_ObjectMoveDown(void *);
void	 AG_ObjectMoveToHead(void *);
void	 AG_ObjectMoveToTail(void *);
void	 AG_ObjectDestroy(void *);
void	 AG_ObjectUnlinkDatafiles(void *);
void	 AG_ObjectSetSavePfx(void *, char *);

void	 AG_ObjectFreeVariables(void *);
void	 AG_ObjectFreeChildren(void *);
void 	 AG_ObjectFreeEvents(AG_Object *);
void	 AG_ObjectFreeDeps(AG_Object *);
void	 AG_ObjectFreeDummyDeps(AG_Object *);

int	 AG_ObjectPageIn(void *);
int	 AG_ObjectPageOut(void *);
int	 AG_ObjectSerialize(void *, AG_DataSource *);
int	 AG_ObjectUnserialize(void *, AG_DataSource *);
int	 AG_ObjectSaveToFile(void *, const char *);
#define	 AG_ObjectSave(p) AG_ObjectSaveToFile((p),NULL)
int	 AG_ObjectSaveAll(void *);
void	 AG_ObjectSaveVariables(void *, AG_DataSource *);
int      AG_ObjectLoadFromDB(void *, struct ag_db *, const struct ag_dbt *);
int	 AG_ObjectSaveToDB(void *, struct ag_db *, const struct ag_dbt *);

#define	 AG_ObjectLoad(p) AG_ObjectLoadFromFile((p),NULL)
#define	 AG_ObjectLoadData(o,f) AG_ObjectLoadDataFromFile((o),(f),NULL)
#define	 AG_ObjectLoadGeneric(p) AG_ObjectLoadGenericFromFile((p),NULL)
int	 AG_ObjectLoadFromFile(void *, const char *);
int	 AG_ObjectLoadGenericFromFile(void *, const char *);
int	 AG_ObjectResolveDeps(void *);
int	 AG_ObjectLoadDataFromFile(void *, int *, const char *);
int	 AG_ObjectReadHeader(AG_DataSource *, AG_ObjectHeader *);
int	 AG_ObjectLoadVariables(void *, AG_DataSource *);

AG_ObjectDep *AG_ObjectAddDep(void *, void *, int);
int           AG_ObjectFindDep(void *, Uint32, void **);
void          AG_ObjectDelDep(void *, const void *);
Uint32        AG_ObjectEncodeName(void *, const void *);
void         *AG_ObjectEdit(void *);
void          AG_ObjectGenName(void *, AG_ObjectClass *, char *, size_t);
void          AG_ObjectGenNamePfx(void *, const char *, char *, size_t);

#define AG_OfClass(obj,cspec) AG_ClassIsNamed(AGOBJECT(obj)->cls,(cspec))

#ifdef AG_THREADS
# define AG_ObjectLock(ob) AG_MutexLock(&AGOBJECT(ob)->lock)
# define AG_ObjectUnlock(ob) AG_MutexUnlock(&AGOBJECT(ob)->lock)
# define AG_LockVFS(ob) AG_ObjectLock(AGOBJECT(ob)->root)
# define AG_UnlockVFS(ob) AG_ObjectUnlock(AGOBJECT(ob)->root)
#else /* !AG_THREADS */
# define AG_ObjectLock(ob)
# define AG_ObjectUnlock(ob)
# define AG_LockVFS(ob)
# define AG_UnlockVFS(ob)
#endif /* AG_THREADS */

/*
 * Detach and destroy an object.
 */
static __inline__ void
AG_ObjectDelete(void *pObj)
{
	AG_Object *obj = AGOBJECT(pObj);

	if (obj->parent != NULL) {
		AG_ObjectDetach(obj);
	}
	AG_ObjectDestroy(obj);
}

/*
 * Return a child object by name.
 * Result is valid as long as parent object's VFS is locked.
 */
static __inline__ void *
AG_ObjectFindChild(void *pParent, const char *name)
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

/* Return the description of the superclass of a given object. */
static __inline__ AG_ObjectClass *
AG_ObjectSuperclass(const void *p)
{
	return AGOBJECT(p)->cls->super;
}

/* Lock/unlock the timer queue and all timers associated with an object. */
static __inline__ void
AG_LockTimers(void *p)
{
#ifdef AG_THREADS
	AG_Object *ob = (p != NULL) ? AGOBJECT(p) : &agTimerMgr;
	AG_ObjectLock(ob);
	AG_LockTiming();
#endif
}
static __inline__ void
AG_UnlockTimers(void *p)
{
#ifdef AG_THREADS
	AG_Object *ob = (p != NULL) ? AGOBJECT(p) : &agTimerMgr;
	AG_UnlockTiming();
	AG_ObjectUnlock(ob);
#endif
}

/* Return the inheritance hierarchy of an object in string form. */
static __inline__ void
AG_ObjectGetInheritHierString(void *obj, char *buf, size_t buf_size)
{
	AG_ObjectLock(obj);
	AG_Strlcpy(buf, AGOBJECT_CLASS(obj)->hier, buf_size);
	AG_ObjectUnlock(obj);
}

/*
 * Evaluate whether the named object variable exists.
 * The object must be locked.
 */
static __inline__ int
AG_Defined(void *pObj, const char *name)
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
static __inline__ AG_Variable *
AG_FetchVariable(void *pObj, const char *name, enum ag_variable_type type)
{
	AG_Object *obj = (AG_Object *)pObj;
	AG_Variable *V;

	AG_TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (V == NULL) {
		V = AG_Malloc(sizeof(AG_Variable));
		AG_InitVariable(V, type);
		AG_Strlcpy(V->name, name, sizeof(V->name));
		AG_TAILQ_INSERT_TAIL(&obj->vars, V, vars);
	}
	return (V);
}

/*
 * Variant of AG_FetchVariable(). If an existing variable of the given name
 * exists, it is freed and reinitialized as the specified type.
 */
static __inline__ AG_Variable *
AG_FetchVariableOfType(void *pObj, const char *name, enum ag_variable_type type)
{
	AG_Variable *V = AG_FetchVariable(pObj, name, type);
	if (V->type != type) {
		AG_FreeVariable(V);
		AG_InitVariable(V, type);
	}
	return (V);
}

/*
 * Lookup an object variable by name and return a locked AG_Variable.
 * The object must be locked.
 */
static __inline__ AG_Variable *
AG_GetVariableLocked(void *pObj, const char *name)
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
		Vtgt = AG_GetVariableLocked(AGOBJECT(V->data.p), V->info.ref.key);
		AG_UnlockVariable(V);
		return (Vtgt);
	}
	return (V);
}

/* Accessor routine for AG_OBJECT_NAMED() macro in AG_Event(3). */
static __inline__ void *
AG_GetNamedObject(AG_Event *event, const char *key, const char *classSpec)
{
	AG_Variable *V = AG_GetNamedEventArg(event, key);

	if (!AG_OfClass((struct ag_object *)V->data.p, classSpec)) {
		AG_FatalError("Argument %s is not a %s", key, classSpec);
	}
	return (V->data.p);
}

#ifdef AG_LEGACY
# define AG_OBJECT_RELOAD_PROPS AG_OBJECT_FLOATING_VARS
# define AG_LockTimeouts(ob) AG_LockTimers(ob)
# define AG_UnlockTimeouts(ob) AG_UnlockTimers(ob)
# define AG_ObjectIsClass(obj,cname) AG_OfClass((obj),(cname))
# define AG_ObjectFreeProps(obj) AG_ObjectFreeVariables(obj)
# define AG_ObjectFindF AG_ObjectFind
# define AG_PropLoad AG_ObjectLoadVariables
# define AG_PropSave AG_ObjectSaveVariables
# define AG_PropDefined AG_Defined
# define AG_GetStringCopy AG_GetString
# define AG_Prop AG_Variable
# define ag_prop ag_variable
# define ag_prop_type ag_variable_type
# define AG_PROP_UINT AG_VARIABLE_UINT
# define AG_PROP_INT AG_VARIABLE_INT
# define AG_PROP_UINT8 AG_VARIABLE_UINT8
# define AG_PROP_SINT8 AG_VARIABLE_SINT8
# define AG_PROP_UINT16	AG_VARIABLE_UINT16
# define AG_PROP_SINT16	AG_VARIABLE_SINT16
# define AG_PROP_UINT32	AG_VARIABLE_UINT32
# define AG_PROP_SINT32	AG_VARIABLE_SINT32
# define AG_PROP_FLOAT AG_VARIABLE_FLOAT
# define AG_PROP_DOUBLE	AG_VARIABLE_DOUBLE
# define AG_PROP_STRING	AG_VARIABLE_STRING
# define AG_PROP_POINTER AG_VARIABLE_POINTER
# define AG_PROP_BOOL AG_VARIABLE_INT
AG_Prop	*AG_SetProp(void *, const char *, enum ag_prop_type, ...) DEPRECATED_ATTRIBUTE;
AG_Prop	*AG_GetProp(void *, const char *, int, void *) DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/core/close.h>

#endif /* _AGAR_CORE_OBJECT_H_ */
