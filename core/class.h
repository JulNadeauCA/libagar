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
	const char *name;			/* Name string */
	const char *pfx;			/* Prefix string */
	const char *url;			/* URL of package */
} AG_Namespace;

typedef void  (*AG_ObjectInitFn)(void *);
typedef void  (*AG_ObjectResetFn)(void *);
typedef void  (*AG_ObjectDestroyFn)(void *);
typedef int   (*AG_ObjectLoadFn)(void *, AG_DataSource *, const AG_Version *);
typedef int   (*AG_ObjectSaveFn)(void *, AG_DataSource *);
typedef void *(*AG_ObjectEditFn)(void *);

/* Object class description (generated part) */
typedef struct ag_object_class_pvt {
	char libs[AG_OBJECT_LIBS_MAX];              /* List of required modules */
	AG_TAILQ_HEAD_(ag_object_class) sub;        /* Direct subclasses */
	AG_TAILQ_ENTRY(ag_object_class) subclasses; /* Subclass entry */
} AG_ObjectClassPvt;

/* Object class description. */
typedef struct ag_object_class {
	char       hier[AG_OBJECT_HIER_MAX];	/* Inheritance hierarchy */
	size_t     size;			/* Structure size */
	AG_Version ver;				/* Data version */

	AG_ObjectInitFn    init;		/* Initialization routine */
	AG_ObjectResetFn   reset;		/* Reset state for load */
	AG_ObjectDestroyFn destroy;		/* Release data */
	AG_ObjectLoadFn    load;		/* Deserialize */
	AG_ObjectSaveFn    save;		/* Serialize */
	AG_ObjectEditFn    edit;		/* User-defined edit callback */

	char name[AG_OBJECT_TYPE_MAX];		/* Short name of this class */
	struct ag_object_class *super;		/* Direct superclass */
	AG_ObjectClassPvt pvt;			/* Private data */
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
extern struct ag_tbl  *agClassTbl;		/* Classes in hash table */
extern AG_ObjectClass *agClassTree;		/* Classes in tree format */
extern AG_Namespace   *agNamespaceTbl;		/* Registered namespaces */
extern int             agNamespaceCount;
extern char           **agModuleDirs;		/* Module search directories */
extern int              agModuleDirCount;
extern AG_Mutex	        agClassLock;		/* Lock on class table */

void            AG_InitClassTbl(void);
void            AG_DestroyClassTbl(void);

AG_ObjectClass *AG_LoadClass(const char *);
AG_ObjectClass *AG_LookupClass(const char *);
void            AG_UnloadClass(AG_ObjectClass *);

AG_Namespace *AG_RegisterNamespace(const char *, const char *, const char *);
void          AG_UnregisterNamespace(const char *);
void          AG_RegisterModuleDirectory(const char *);
void          AG_UnregisterModuleDirectory(const char *);
void          AG_RegisterClass(void *);
void          AG_UnregisterClass(void *);

void              *AG_CreateClass(const char *, size_t, size_t, Uint, Uint);
AG_ObjectInitFn    AG_ClassSetInit(void *, AG_ObjectInitFn);
AG_ObjectResetFn   AG_ClassSetReset(void *, AG_ObjectResetFn);
AG_ObjectDestroyFn AG_ClassSetDestroy(void *, AG_ObjectDestroyFn);
AG_ObjectLoadFn    AG_ClassSetLoad(void *, AG_ObjectLoadFn);
AG_ObjectSaveFn    AG_ClassSetSave(void *, AG_ObjectSaveFn);
AG_ObjectEditFn    AG_ClassSetEdit(void *, AG_ObjectEditFn);
void               AG_DestroyClass(void *);

int  AG_ParseClassSpec(AG_ObjectClassSpec *, const char *);
int  AG_OfClassGeneral(const struct ag_object *, const char *);
int  AG_ClassIsNamedGeneral(const AG_ObjectClass *, const char *);
int  AG_ObjectGetInheritHier(void *, AG_ObjectClass ***, int *);

/* Return description for the given namespace. */
static __inline__ AG_Namespace *
AG_GetNamespace(const char *ns)
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
AG_ClassIsNamed(void *pClass, const char *pat)
{
	AG_ObjectClass *cls = (AG_ObjectClass *)pClass;
	const char *c;
	int nwild = 0;
	size_t patSize;

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
