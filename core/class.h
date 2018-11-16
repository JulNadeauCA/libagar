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

extern _Nonnull AG_Mutex agClassLock;	           /* Lock on class table */

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

/* Return description for the given namespace. */
static __inline__ AG_Namespace *_Nullable
AG_GetNamespace(const char *_Nonnull ns)
{
	int i;

	for (i = 0; i < agNamespaceCount; i++) {
		if (strcmp(agNamespaceTbl[i].name, ns) == 0)
			return (&agNamespaceTbl[i]);
	}
	AG_SetError("No such namespace: %s", ns);
	return (NULL);
}

/* Compare the inheritance hierarchy of a class against a given pattern. */
static __inline__ int
AG_ClassIsNamed(void *_Nonnull pClass, const char *_Nonnull pat)
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
__END_DECLS

#include <agar/core/close.h>
