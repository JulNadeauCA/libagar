/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
#define _AGAR_CORE_OBJECT_H_

#ifndef AG_OBJECT_NAME_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_NAME_MAX 16
# else
#  define AG_OBJECT_NAME_MAX 36
# endif
#endif
#ifndef AG_OBJECT_TYPE_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_TYPE_MAX 24
# else
#  define AG_OBJECT_TYPE_MAX 48
# endif
#endif
#ifndef AG_OBJECT_HIER_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_HIER_MAX 48
# elif AG_MODEL == AG_MEDIUM
#  define AG_OBJECT_HIER_MAX 64
# elif AG_MODEL == AG_LARGE
#  define AG_OBJECT_HIER_MAX 96
# endif
#endif
#ifndef AG_OBJECT_PATH_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_PATH_MAX 64
# else
#  define AG_OBJECT_PATH_MAX 128
# endif
#endif
#ifndef AG_OBJECT_LIBS_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_LIBS_MAX 16
# else
#  define AG_OBJECT_LIBS_MAX 32
# endif
#endif
#ifndef AG_OBJECT_CLASSTBLSIZE
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_CLASSTBLSIZE 8
# elif AG_MODEL == AG_MEDIUM
#  define AG_OBJECT_CLASSTBLSIZE 64
# else
#  define AG_OBJECT_CLASSTBLSIZE 128
# endif
#endif

#ifndef AG_OBJECT_MAX_VARIABLES
# define AG_OBJECT_MAX_VARIABLES 0xffff
#endif
#ifndef AG_OBJECT_TYPE_TAG
# define AG_OBJECT_TYPE_TAG "AgarObj"
#endif
#define AG_OBJECT_TYPE_TAG_LEN 8

#include <agar/core/begin.h>

struct ag_object;
struct ag_tbl;
struct ag_db;
struct ag_dbt;

#include <agar/core/variable.h>
#include <agar/core/event.h>
#include <agar/core/agtime.h>

/* Object class specification (generated) */
typedef struct ag_object_class_spec {
	char hier[AG_OBJECT_HIER_MAX];	/* Inheritance hierarchy (normalized) */
	char spec[AG_OBJECT_HIER_MAX];	/* Full class string + optional @libs */
	char name[AG_OBJECT_NAME_MAX];	/* Name of the last class in hierarchy */
#ifdef AG_ENABLE_DSO
	char libs[AG_OBJECT_LIBS_MAX];	/* Optional comma-separated libs */
#endif
} AG_ObjectClassSpec;

/* Registered name space (for NAMESPACES) */
typedef struct ag_namespace {
	const char *_Nonnull name;	/* Name string */
	const char *_Nonnull pfx;	/* Prefix string */
	const char *_Nonnull url;	/* URL of package */
} AG_Namespace;

/* Convenient typedefs for base AG_Object methods */
typedef void (*AG_ObjectInitFn)(void *_Nonnull);
typedef void (*AG_ObjectResetFn)(void *_Nonnull);
typedef void (*AG_ObjectDestroyFn)(void *_Nonnull);
#ifdef AG_SERIALIZATION
typedef int (*AG_ObjectLoadFn)(void *_Nonnull, AG_DataSource *_Nonnull,
                               const AG_Version *_Nonnull);
typedef int (*AG_ObjectSaveFn)(void *_Nonnull, AG_DataSource *_Nonnull);
#else
typedef int (*AG_ObjectLoadFn)(void *_Nonnull, void *_Nonnull,
                               const AG_Version *_Nonnull);
typedef int (*AG_ObjectSaveFn)(void *_Nonnull, void *_Nonnull);
#endif
typedef void *_Nullable (*AG_ObjectEditFn) (void *_Nonnull);

/* Object class description (private, generated section) */
typedef struct ag_object_class_pvt {
	char libs[AG_OBJECT_LIBS_MAX];              /* List of required modules */
	AG_TAILQ_HEAD_(ag_object_class) sub;        /* Direct subclasses */
	AG_TAILQ_ENTRY(ag_object_class) subclasses; /* Subclass entry */
} AG_ObjectClassPvt;

/* Object class description */
typedef struct ag_object_class {              /* --- Required fields --- */
	char hier[AG_OBJECT_HIER_MAX];        /* Inheritance hierarchy */
	AG_Size size;                         /* Instance structure size (bytes) */
	AG_Version ver;                       /* Serialized version (or 0.0) */
	_Nullable AG_ObjectInitFn init;       /* Initialization */
	_Nullable AG_ObjectResetFn reset;     /* Pre-serialization code */
	_Nullable AG_ObjectDestroyFn destroy; /* Finalization */
	_Nullable AG_ObjectLoadFn load;       /* Deserialization (reading) */
	_Nullable AG_ObjectSaveFn save;       /* Serialization (writing) */
	_Nullable AG_ObjectEditFn edit;       /* User-defined editor callback */

	                                         /* --- Generated fields --- */
	char name[AG_OBJECT_TYPE_MAX];		 /* Short name of this class */
	struct ag_object_class *_Nullable super; /* Direct superclass */
	AG_ObjectClassPvt pvt;			 /* Private data */
} AG_ObjectClass;

AG_TAILQ_HEAD(ag_objectq, ag_object);

/* Object instance */
typedef struct ag_object {
#ifdef AG_TYPE_SAFETY
	char tag[AG_OBJECT_TYPE_TAG_LEN]; /* For runtime type-safety check */
#endif
	char name[AG_OBJECT_NAME_MAX];    /* Object ID (unique in parent) */
	Uint flags;
#define AG_OBJECT_FLOATING_VARS	 0x00001  /* Clear variables before load */
#define AG_OBJECT_NON_PERSISTENT 0x00002  /* Never include in saves */
#define AG_OBJECT_INDESTRUCTIBLE 0x00004  /* Not destructible (advisory) */
#define AG_OBJECT_RESIDENT	 0x00008  /* Data part is resident */
#define AG_OBJECT_STATIC	 0x00020  /* Don't free() after detach */
#define AG_OBJECT_READONLY	 0x00040  /* Disallow edition (advisory) */
#define AG_OBJECT_WAS_RESIDENT	 0x00080  /* Used internally by ObjectLoad() */
#define AG_OBJECT_REOPEN_ONLOAD	 0x00200  /* Recreate editor UI on ObjectLoad() */
#define AG_OBJECT_REMAIN_DATA	 0x00400  /* Keep user data resident */
#define AG_OBJECT_DEBUG		 0x00800  /* Enable debugging */
#define AG_OBJECT_NAME_ONATTACH	 0x01000  /* Generate name on attach */
#define AG_OBJECT_CHLD_AUTOSAVE	 0x02000  /* Include child obj data in archive */
#define AG_OBJECT_DEBUG_DATA	 0x04000  /* Datafiles contain debug info */
#define AG_OBJECT_INATTACH	 0x08000  /* In AG_ObjectAttach() */
#define AG_OBJECT_INDETACH	 0x10000  /* In AG_ObjectDetach() */
#define AG_OBJECT_BOUND_EVENTS	 0x20000  /* Generate "bound" events whenever
					     AG_Bind*() is invoked */
#define AG_OBJECT_SAVED_FLAGS	(AG_OBJECT_FLOATING_VARS|\
 				 AG_OBJECT_INDESTRUCTIBLE|\
				 AG_OBJECT_READONLY|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA|\
				 AG_OBJECT_DEBUG|\
				 AG_OBJECT_BOUND_EVENTS)

	AG_ObjectClass *_Nonnull cls;     /* Class description */

	AG_TAILQ_HEAD_(ag_event) events;  /* Event handlers */
#ifdef AG_TIMERS
	AG_TAILQ_HEAD_(ag_timer) timers;  /* Running timers */
#endif
	AG_TAILQ_HEAD_(ag_variable) vars; /* Named variables / bindings */
	struct ag_objectq children;       /* Child objects */
	AG_TAILQ_ENTRY(ag_object) cobjs;  /* Entry in parent */
	void *_Nullable parent;           /* Parent in VFS (NULL = is root) */
	void *_Nonnull root;              /* VFS root (possibly self) */
#ifdef AG_TIMERS
	AG_TAILQ_ENTRY(ag_object) tobjs;  /* Entry in agTimerObjQ */
#endif
	_Nonnull_Mutex AG_Mutex lock;     /* General object lock */
} AG_Object;

/* Object archive header information. */
typedef struct ag_object_header {
	AG_ObjectClassSpec cs;            /* Class specification */
	Uint32 dataOffs;                  /* Dataset offset */
	AG_Version ver;                   /* AG_Object version */
	Uint flags;                       /* Object flags */
} AG_ObjectHeader;

#define AGOBJECT(ob) ((struct ag_object *)(ob))
#define AGCLASS(cls) ((struct ag_object_class *)(cls))
#define AGOBJECT_CLASS(obj) ((struct ag_object_class *)(AGOBJECT(obj)->cls))

/* Argument Accessors */
#ifdef AG_TYPE_SAFETY
# define AG_OBJECT_VALID(p) \
   (strncmp(AGOBJECT(p)->tag, AG_OBJECT_TYPE_TAG, AG_OBJECT_TYPE_TAG_LEN) == 0)
# define AG_OBJECT(v,hier) \
   ((v <= event->argc && event->argv[v].type == AG_VARIABLE_POINTER && \
     !(event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY) && \
     AG_OBJECT_VALID(event->argv[v].data.p) && \
     AG_OfClass(event->argv[v].data.p,(hier))) ? event->argv[v].data.p : \
                                                 AG_ObjectMismatch())
# define AG_OBJECT_PTR(v) \
  ((v <= event->argc && event->argv[v].type == AG_VARIABLE_POINTER && \
    !(event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY) && \
    AG_OBJECT_VALID(event->argv[v].data.p)) ? event->argv[v].data.p : \
                                              AG_ObjectMismatch())
# define AG_CONST_OBJECT(v,hier) \
   ((v <= event->argc && event->argv[v].type == AG_VARIABLE_POINTER && \
     (event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY) && \
     AG_OBJECT_VALID(event->argv[v].data.p) && \
     AG_OfClass(event->argv[v].data.p,(hier))) ? (const void *)event->argv[v].data.p : \
                                                 (const void *)AG_ObjectMismatch())
# define AG_CONST_OBJECT_PTR(v) \
  ((v <= event->argc && event->argv[v].type == AG_VARIABLE_POINTER && \
    (event->argv[v].info.pFlags & AG_VARIABLE_P_READONLY) && \
    AG_OBJECT_VALID(event->argv[v].data.p)) ? (const void *)event->argv[v].data.p : \
                                              (const void *)AG_ObjectMismatch())
# define AG_OBJECT_ISA(obj,class) { \
	if (!AG_OBJECT_VALID(obj)) { \
		AG_FatalErrorF("%p is not a valid AG_Object", (obj)); \
	} \
	if (!AG_OfClass((obj),(class))) { \
		AG_FatalErrorF("%s is not a %s", AGOBJECT(obj)->name, class); \
	} \
 }

#else /* !AG_TYPE_SAFETY */

# define AG_OBJECT_VALID(p)       (1)
# define AG_OBJECT(v,hier)        (event->argv[v].data.p)
# define AG_OBJECT_PTR(v)         (event->argv[v].data.p)
# define AG_CONST_OBJECT(v,hier)  (event->argv[v].data.p)
# define AG_CONST_OBJECT_PTR(v)  ((const void *)event->argv[v].data.p)
# define AG_OBJECT_ISA(obj,class)

#endif /* AG_TYPE_SAFETY */

#define AG_OBJECT_SELF()         AG_OBJECT_PTR(0)
#define AG_OBJECT_NAMED(n)       AG_PTR_NAMED(n)
#define AG_CONST_OBJECT_SELF()   AG_CONST_OBJECT_PTR(0)
#define AG_CONST_OBJECT_NAMED(n) AG_CONST_PTR_NAMED(n)

/* Iterate over child objects */
#define AGOBJECT_FOREACH_CHILD(var, ob, t) \
	for((var) = (struct t *)AG_TAILQ_FIRST(&AGOBJECT(ob)->children); \
	    (var) != (struct t *)AG_TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct t *)AG_TAILQ_NEXT(AGOBJECT(var), cobjs))
#define AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, t) \
	for((var) = (struct t *)AG_TAILQ_LAST(&AGOBJECT(ob)->children, \
	    ag_objectq); \
	    (var) != (struct t *)AG_TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct t *)AG_TAILQ_PREV(AGOBJECT(var), ag_objectq, \
	    cobjs))
#define AGOBJECT_NEXT_CHILD(var,t) \
	((struct t *)AG_TAILQ_NEXT(AGOBJECT(var),cobjs))
#define AGOBJECT_LAST_CHILD(var,t) \
	((struct t *)AG_TAILQ_LAST(&AGOBJECT(var)->children,ag_objectq))

/* Iterate over child objects of a given class. */
# define AGOBJECT_FOREACH_CLASS(var, ob, t, subclass) \
	AGOBJECT_FOREACH_CHILD(var,ob,t) \
		if (!AG_OBJECT_VALID(var) || !AG_OfClass(var,(subclass))) { \
			continue; \
		} else

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_CORE)
/*
 * Shorthands
 */
# define OBJECT(ob)              AGOBJECT(ob)
# define OBJECT_CLASS(ob)        AGOBJECT_CLASS(ob)
# define CLASS(ob)               AGCLASS(ob)

# define OBJECT_RESIDENT(ob)    (AGOBJECT(ob)->flags & AG_OBJECT_RESIDENT)
# define OBJECT_PERSISTENT(ob) !(AGOBJECT(ob)->flags & AG_OBJECT_NON_PERSISTENT)
# define OBJECT_DEBUG(ob)       (AGOBJECT(ob)->flags & AG_OBJECT_DEBUG)

# define OBJECT_FOREACH_CHILD(var,ob,t)          AGOBJECT_FOREACH_CHILD((var),(ob),t)
# define OBJECT_FOREACH_CHILD_REVERSE(var,ob,t)  AGOBJECT_FOREACH_CHILD_REVERSE((var),(ob),t)
# define OBJECT_FOREACH_CLASS(var,ob,t,subclass) AGOBJECT_FOREACH_CLASS((var),(ob),t,(subclass))
# define OBJECT_NEXT_CHILD(var,t)                AGOBJECT_NEXT_CHILD((var),t)
# define OBJECT_LAST_CHILD(var,t)                AGOBJECT_LAST_CHILD((var),t)
#endif /* _AGAR_INTERNAL or _USE_AGAR_CORE */

__BEGIN_DECLS
extern AG_ObjectClass agObjectClass;              /* Base Object class */

#ifdef AG_THREADS
extern _Nonnull_Mutex AG_Mutex agClassLock;       /* Lock on class table */
#endif
extern struct ag_tbl  *_Nullable agClassTbl;      /* Classes in hash table */
#ifdef AG_NAMESPACES
extern AG_Namespace *_Nullable agNamespaceTbl;    /* Registered namespaces */
extern int                     agNamespaceCount;
#endif
#ifdef AG_ENABLE_DSO
extern char *_Nullable *_Nonnull agModuleDirs;    /* Module search dirs */
extern int                       agModuleDirCount;
#endif

void AG_InitClassTbl(void);
void AG_DestroyClassTbl(void);

AG_ObjectClass *_Nullable AG_LookupClass(const char *_Nonnull);
#ifdef AG_ENABLE_DSO
AG_ObjectClass *_Nullable AG_LoadClass(const char *_Nonnull);
void                      AG_UnloadClass(AG_ObjectClass *_Nonnull);
void                      AG_RegisterModuleDirectory(const char *_Nonnull);
void                      AG_UnregisterModuleDirectory(const char *_Nonnull);
#endif

#ifdef AG_NAMESPACES
AG_Namespace *_Nonnull AG_RegisterNamespace(const char *_Nonnull,
                                            const char *_Nonnull,
                                            const char *_Nonnull);
void                   AG_UnregisterNamespace(const char *_Nonnull);
#endif

void AG_RegisterClass(void *_Nonnull);
void AG_UnregisterClass(void *_Nonnull);

#if AG_MODEL != AG_SMALL
void *_Nullable AG_CreateClass(const char *_Nonnull, AG_Size, AG_Size, Uint, Uint);
void            AG_DestroyClass(void *_Nonnull);

_Nullable AG_ObjectInitFn    AG_ClassSetInit(void *_Nonnull, _Nullable AG_ObjectInitFn);
_Nullable AG_ObjectResetFn   AG_ClassSetReset(void *_Nonnull, _Nullable AG_ObjectResetFn);
_Nullable AG_ObjectDestroyFn AG_ClassSetDestroy(void *_Nonnull, _Nullable AG_ObjectDestroyFn);
_Nullable AG_ObjectLoadFn    AG_ClassSetLoad(void *_Nonnull, _Nullable AG_ObjectLoadFn);
_Nullable AG_ObjectSaveFn    AG_ClassSetSave(void *_Nonnull, _Nullable AG_ObjectSaveFn);
_Nullable AG_ObjectEditFn    AG_ClassSetEdit(void *_Nonnull, _Nullable AG_ObjectEditFn);
#endif /* !AG_SMALL */

int AG_ParseClassSpec(AG_ObjectClassSpec *_Nonnull, const char *_Nonnull);
int AG_ClassIsNamedGeneral(const AG_ObjectClass *_Nonnull, const char *_Nonnull);

int AG_ObjectGetInheritHier(void *_Nonnull,
                            AG_ObjectClass *_Nonnull *_Nonnull *_Nullable,
                            int *_Nonnull);

void *_Nullable AG_ObjectNew(void *_Nullable, const char *_Nullable,
                             AG_ObjectClass *_Nonnull);

void AG_ObjectAttach(void *_Nullable _Restrict, void *_Nonnull _Restrict);

void AG_ObjectInit(void *_Nonnull _Restrict, void *_Nullable _Restrict);
void AG_ObjectInitStatic(void *_Nonnull, void *_Nullable);
void AG_ObjectDetach(void *_Nonnull);
void AG_ObjectReset(void *_Nonnull);

#if AG_MODEL != AG_SMALL
char *_Nonnull  AG_ObjectGetClassName(const void *, int);
char *_Nullable AG_ObjectGetName(void *_Nonnull) _Warn_Unused_Result;
void            AG_ObjectInitNamed(void *_Nonnull, void *_Nonnull,
                                   const char *_Nullable);
int             AG_ObjectCopyName(void *_Nonnull, char *_Nonnull, AG_Size);
#endif

void *_Nullable AG_ObjectFindS(void *_Nonnull, const char *_Nonnull)
                              _Pure_Attribute_If_Unthreaded
			      _Warn_Unused_Result;

void *_Nullable AG_ObjectFind(void *_Nonnull, const char *_Nonnull, ...)
                             FORMAT_ATTRIBUTE(printf,2,3)
			     _Pure_Attribute_If_Unthreaded
			     _Warn_Unused_Result;

void *_Nullable AG_ObjectFindParent(void *_Nonnull, const char *_Nonnull,
				    const char *_Nonnull)
				   _Warn_Unused_Result;

void AG_ObjectSetNameS(void *_Nonnull, const char *_Nullable);
void AG_ObjectSetName(void *_Nonnull, const char *_Nullable, ...)
                     FORMAT_ATTRIBUTE(printf,2,3);

void AG_SetFn(void *_Nonnull, const char *_Nonnull,
              _Nullable AG_EventFn, const char *_Nullable, ...);

#if AG_MODEL != AG_SMALL
void AG_ObjectMoveUp(void *_Nonnull);
void AG_ObjectMoveDown(void *_Nonnull);
void AG_ObjectMoveToHead(void *_Nonnull);
#endif
void AG_ObjectMoveToTail(void *_Nonnull);

void AG_ObjectDestroy(void *_Nonnull);
void AG_ObjectFreeVariables(void *_Nonnull);
void AG_ObjectFreeChildren(void *_Nonnull);
void AG_ObjectFreeEvents(AG_Object *_Nonnull);

#ifdef AG_SERIALIZATION
int AG_ObjectCopyFilename(void *_Nonnull, char *_Nonnull, AG_Size);
int AG_ObjectCopyDirname(void *_Nonnull, char *_Nonnull, AG_Size);
int AG_ObjectChanged(void *_Nonnull);
int AG_ObjectChangedAll(void *_Nonnull);

int AG_ObjectInUse(void *_Nonnull)
                  _Pure_Attribute_If_Unthreaded
		  _Warn_Unused_Result;

void AG_ObjectUnlinkDatafiles(void *_Nonnull);

int AG_ObjectPageIn(void *_Nonnull);
int AG_ObjectPageOut(void *_Nonnull);

int AG_ObjectSerialize(void *_Nonnull, AG_DataSource *_Nonnull);
int AG_ObjectUnserialize(void *_Nonnull, AG_DataSource *_Nonnull);

int  AG_ObjectSave(void *_Nonnull);
int  AG_ObjectSaveToFile(void *_Nonnull, const char *_Nullable);
int  AG_ObjectSaveToDB(void *_Nonnull, struct ag_db *_Nonnull,
                       const struct ag_dbt *_Nonnull);
int  AG_ObjectSaveAll(void *_Nonnull);
void AG_ObjectSaveVariables(void *_Nonnull, AG_DataSource *_Nonnull);

int AG_ObjectLoad(void *_Nonnull);
int AG_ObjectLoadFromFile(void *_Nonnull, const char *_Nullable);
int AG_ObjectLoadFromDB(void *_Nonnull, struct ag_db *_Nonnull,
                        const struct ag_dbt *_Nonnull);
int AG_ObjectLoadData(void *_Nonnull, int *_Nonnull);
int AG_ObjectLoadDataFromFile(void *_Nonnull, int *_Nonnull, const char *_Nullable);
int AG_ObjectLoadGeneric(void *_Nonnull);
int AG_ObjectLoadGenericFromFile(void *_Nonnull, const char *_Nullable);
int AG_ObjectReadHeader(AG_DataSource *_Nonnull, AG_ObjectHeader *_Nonnull);
int AG_ObjectLoadVariables(void *_Nonnull, AG_DataSource *_Nonnull);
#endif /* AG_SERIALIZATION */

void AG_ObjectGenName(void *_Nonnull, AG_ObjectClass *_Nonnull, char *_Nonnull,
                      AG_Size);
#if AG_MODEL != AG_SMALL
void AG_ObjectGenNamePfx(void *_Nonnull, const char *_Nonnull, char *_Nonnull,
                         AG_Size);
#endif

/*
 * Inlinables
 */
int ag_of_class(const void *_Nonnull, const char *_Nonnull)
               _Warn_Unused_Result;

AG_Object *_Nonnull ag_object_root(const void *_Nonnull)
                                  _Pure_Attribute
                                  _Warn_Unused_Result;

AG_Object *_Nullable ag_object_parent(const void *_Nonnull)
                                     _Pure_Attribute
                                     _Warn_Unused_Result;

#ifdef AG_NAMESPACES
AG_Namespace *_Nullable ag_get_namespace(const char *_Nonnull)
                                        _Warn_Unused_Result;
#endif

int ag_class_is_named(const void *_Nonnull, const char *_Nonnull)
                     _Warn_Unused_Result;

void *_Nullable ag_object_find_child(void *_Nonnull, const char *_Nonnull)
                                    _Pure_Attribute_If_Unthreaded
				    _Warn_Unused_Result;

AG_ObjectClass *_Nullable ag_object_superclass(const void *_Nonnull)
                                              _Pure_Attribute
					      _Warn_Unused_Result;

void ag_object_delete(void *_Nonnull);

int ag_defined(void *_Nonnull, const char *_Nonnull)
              _Pure_Attribute
              _Warn_Unused_Result;

AG_Variable *_Nonnull ag_fetch_variable(void *_Nonnull, const char *_Nonnull,
                                        enum ag_variable_type)
                                       _Warn_Unused_Result;

AG_Variable *_Nonnull ag_fetch_variable_of_type(void *_Nonnull,
                                                const char *_Nonnull,
                                                enum ag_variable_type)
                                               _Warn_Unused_Result;

AG_Variable *_Nullable ag_access_variable(void *_Nonnull, const char *_Nonnull)
                                         _Pure_Attribute_If_Unthreaded
                                         _Warn_Unused_Result;

#ifdef AG_THREADS
void ag_object_lock(void *_Nonnull);
void ag_object_unlock(void *_Nonnull);
void ag_lock_vfs(void *_Nonnull);
void ag_unlock_vfs(void *_Nonnull);
void ag_lock_timers(void *_Nullable);
void ag_unlock_timers(void *_Nullable);
#endif

#ifdef AG_INLINE_OBJECT
# define AG_INLINE_HEADER
# include <agar/core/inline_object.h>
#else
# define AG_GetNamespace(s)            ag_get_namespace(s)
# define AG_ClassIsNamed(C,s)          ag_class_is_named((C),(s))
# define AG_OfClass(o,s)               ag_of_class((o),(s))
# define AG_ObjectRoot(o)              ag_object_root(o)
# define AG_ObjectParent(o)            ag_object_parent(o)
# define AG_ObjectDelete(o)            ag_object_delete(o)
# define AG_ObjectFindChild(o,n)       ag_object_find_child((o),(n))
# define AG_ObjectSuperclass(o)        ag_object_superclass(o)
# define AG_Defined(o,n)               ag_defined((o),(n))
# define AG_FetchVariable(o,n,t)       ag_fetch_variable((o),(n),(t))
# define AG_FetchVariableOfType(o,n,t) ag_fetch_variable_of_type((o),(n),(t))
# define AG_AccessVariable(o,n)        ag_access_variable((o),(n))
# ifdef AG_THREADS
#  define AG_ObjectLock(o)    ag_object_lock(o)
#  define AG_ObjectUnlock(o)  ag_object_unlock(o)
#  define AG_LockVFS(o)       ag_lock_vfs(o)
#  define AG_UnlockVFS(o)     ag_unlock_vfs(o)
#  ifdef AG_TIMERS
#   define AG_LockTimers(o)   ag_lock_timers(o)
#   define AG_UnlockTimers(o) ag_unlock_timers(o)
#  else
#   define AG_LockTimers(o)
#   define AG_UnlockTimers(o)
#  endif
# else
#  define AG_ObjectLock(o)
#  define AG_ObjectUnlock(o)
#  define AG_LockVFS(o)
#  define AG_UnlockVFS(o)
#  define AG_LockTimers(o)
#  define AG_UnlockTimers(o)
# endif
#endif /* !AG_INLINE_OBJECT */

#ifdef AG_LEGACY
/* <1.6 calls renamed */
# define AG_ObjectFreeDataset(o)   AG_ObjectReset(o)
# define AG_ObjectIsClass(o,c)     AG_OfClass((o),(c))
# define AG_GetStringCopy(o,n,b,s) AG_GetString((o),(n),(b),(s))
# define AG_PrtString              AG_SetStringF
/* <1.6 redundant pointer types removed from AG_Variable */
# define AG_VARIABLE_CONST_STRING    AG_VARIABLE_STRING
# define AG_VARIABLE_P_CONST_STRING  AG_VARIABLE_P_STRING
# define AG_VARIABLE_CONST_POINTER   AG_VARIABLE_POINTER
# define AG_VARIABLE_P_CONST_POINTER AG_VARIABLE_P_POINTER
# define AG_SetConstString(o,n,v)    AG_SetString((o),(n),(char *)(v))
# define AG_BindConstString          AG_BindString
# define AG_BindConstStringMp        AG_BindStringMp
# define AG_BindConstStringFn        AG_BindStringFn
# define AG_BindConstPointer         AG_BindPointer
# define AG_BindConstPointerFn       AG_BindPointerFn
# define AG_BindConstPointerMp       AG_BindPointerMp
/* <1.6 replaced by AG_SetString("archive-path", ...) */
void AG_ObjectSetArchivePath(void *_Nonnull, const char *_Nonnull) DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_OBJECT_H_ */
