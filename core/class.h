/*	Public domain	*/

#include <agar/core/begin.h>

struct ag_tbl;
struct ag_object;

/* Object class specification */
typedef struct ag_object_class_spec {
	char hier[AG_OBJECT_HIER_MAX];		/* Inheritance hierarchy */
	char libs[AG_OBJECT_LIBS_MAX];		/* Library list */
	char spec[AG_OBJECT_HIER_MAX];		/* Full class specification */
	char name[AG_OBJECT_NAME_MAX];		/* Short name */
} AG_ObjectClassSpec;

/* Global name space registration */
typedef struct ag_namespace {
	const char *_Nonnull name;		/* Name string */
	const char *_Nonnull pfx;		/* Prefix string */
	const char *_Nonnull url;		/* URL of package */
} AG_Namespace;

typedef void  (*AG_ObjectInitFn)    (void *_Nonnull);
typedef void  (*AG_ObjectResetFn)   (void *_Nonnull);
typedef void  (*AG_ObjectDestroyFn) (void *_Nonnull);

typedef int   (*AG_ObjectLoadFn) (void             *_Nonnull,
                                  AG_DataSource    *_Nonnull,
                                  const AG_Version *_Nonnull);

typedef int   (*AG_ObjectSaveFn) (void          *_Nonnull,
                                  AG_DataSource *_Nonnull);

typedef void *_Nullable (*AG_ObjectEditFn) (void *_Nonnull);

/* Object class description (generated part) */
typedef struct ag_object_class_pvt {
	char libs[AG_OBJECT_LIBS_MAX];              /* List of required modules */
	AG_TAILQ_HEAD_(ag_object_class) sub;        /* Direct subclasses */
	AG_TAILQ_ENTRY(ag_object_class) subclasses; /* Subclass entry */
} AG_ObjectClassPvt;

/* Object class description. */
typedef struct ag_object_class {
	char       hier[AG_OBJECT_HIER_MAX];	/* Inheritance hierarchy */
	AG_Size    size;			/* Structure size */
	AG_Version ver;				/* Data version */

	_Nullable AG_ObjectInitFn    init;	/* Initialization routine */
	_Nullable AG_ObjectResetFn   reset;	/* Reset state for load */
	_Nullable AG_ObjectDestroyFn destroy;	/* Release data */
	_Nullable AG_ObjectLoadFn    load;	/* Deserialize */
	_Nullable AG_ObjectSaveFn    save;	/* Serialize */
	_Nullable AG_ObjectEditFn    edit;	/* User-defined edit callback */

	char name[AG_OBJECT_TYPE_MAX];		 /* Short name of this class */
	struct ag_object_class *_Nullable super; /* Direct superclass */
	AG_ObjectClassPvt pvt;			 /* Private data */
} AG_ObjectClass;

#ifdef AG_DEBUG
# define AG_ASSERT_CLASS(obj,class) \
	if (!AG_OfClass((obj),(class))) { \
		AG_SetError("%s is not a %s", AGOBJECT(obj)->name, class); \
		AG_FatalError(NULL); \
	}
#else
# define AG_ASSERT_CLASS(obj,class)
#endif

__BEGIN_DECLS
extern struct ag_tbl  *_Nullable agClassTbl;	/* Classes in hash table */
extern AG_ObjectClass *_Nullable agClassTree;	/* Classes in tree format */

extern AG_Namespace *_Nullable agNamespaceTbl;   /* Registered namespaces */
extern int                     agNamespaceCount;

extern char *_Nullable *_Nonnull agModuleDirs;      /* Module search dirs */
extern int                       agModuleDirCount;

extern _Nonnull_Mutex AG_Mutex agClassLock;           /* Lock on class table */

void AG_InitClassTbl(void);
void AG_DestroyClassTbl(void);

AG_ObjectClass *_Nullable AG_LoadClass(const char *_Nonnull);
AG_ObjectClass *_Nullable AG_LookupClass(const char *_Nonnull);
void                      AG_UnloadClass(AG_ObjectClass *_Nonnull);

AG_Namespace *_Nonnull AG_RegisterNamespace(const char *_Nonnull,
                                            const char *_Nonnull,
                                            const char *_Nonnull);
void                   AG_UnregisterNamespace(const char *_Nonnull);

void AG_RegisterModuleDirectory(const char *_Nonnull);
void AG_UnregisterModuleDirectory(const char *_Nonnull);

void AG_RegisterClass(void *_Nonnull);
void AG_UnregisterClass(void *_Nonnull);

void *_Nullable AG_CreateClass(const char *_Nonnull, AG_Size, AG_Size, Uint, Uint);
void            AG_DestroyClass(void *_Nonnull);

_Nullable AG_ObjectInitFn AG_ClassSetInit(void *_Nonnull, _Nullable AG_ObjectInitFn);
_Nullable AG_ObjectResetFn AG_ClassSetReset(void *_Nonnull, _Nullable AG_ObjectResetFn);
_Nullable AG_ObjectDestroyFn AG_ClassSetDestroy(void *_Nonnull, _Nullable AG_ObjectDestroyFn);
_Nullable AG_ObjectLoadFn AG_ClassSetLoad(void *_Nonnull, _Nullable AG_ObjectLoadFn);
_Nullable AG_ObjectSaveFn AG_ClassSetSave(void *_Nonnull, _Nullable AG_ObjectSaveFn);
_Nullable AG_ObjectEditFn AG_ClassSetEdit(void *_Nonnull, _Nullable AG_ObjectEditFn);

int  AG_ParseClassSpec(AG_ObjectClassSpec *_Nonnull, const char *_Nonnull);
int  AG_ClassIsNamedGeneral(const AG_ObjectClass *_Nonnull, const char *_Nonnull);
int  AG_ObjectGetInheritHier(void *_Nonnull,
                             AG_ObjectClass *_Nonnull *_Nonnull *_Nullable,
			     int *_Nonnull);
#ifdef AG_INLINE_OBJECT
# define AG_INLINE_HEADER
# include <agar/core/inline_class.h>
#else
AG_Namespace *_Nullable ag_get_namespace(const char *_Nonnull)
                                        _Warn_Unused_Result;
int ag_class_is_named(void *_Nonnull, const char *_Nonnull)
                     _Warn_Unused_Result;

# define AG_GetNamespace(s)   ag_get_namespace(s)
# define AG_ClassIsNamed(C,s) ag_class_is_named((C),(s))
#endif
__END_DECLS

#include <agar/core/close.h>
