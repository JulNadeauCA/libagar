/*	Public domain	*/

#include "begin_code.h"

struct ag_object;

/* Agar namespace description. */
typedef struct ag_namespace {
	const char *name;			/* Name string */
	const char *pfx;			/* Prefix string */
	const char *url;			/* URL of package */
} AG_Namespace;

/* Agar object class description. */
typedef struct ag_object_class {
	char name[AG_OBJECT_TYPE_MAX];		/* Expanded class name */
	size_t size;				/* Structure size */
	AG_Version ver;				/* Version numbers */
	void (*init)(void *);
	void (*reinit)(void *);
	void (*destroy)(void *);
	int (*load)(void *, AG_DataSource *, const AG_Version *); 
	int (*save)(void *, AG_DataSource *);
	void *(*edit)(void *);
	/*
	 * Private
	 */
	char libs[AG_OBJECT_TYPE_MAX];	/* Comma-separated module list */
} AG_ObjectClass;

__BEGIN_DECLS
extern AG_ObjectClass **agClassTbl;		/* Object classes */
extern int              agClassCount;
extern AG_Namespace    *agNamespaceTbl;		/* Object class namespaces */
extern int              agNamespaceCount;
extern char           **agModuleDirs;		/* Module search directories */
extern int              agModuleDirCount;

void            AG_InitClassTbl(void);
void            AG_DestroyClassTbl(void);
AG_ObjectClass *AG_LookupClass(const char *);
AG_ObjectClass *AG_LoadClass(const char *);
void            AG_UnloadClass(AG_ObjectClass *);
AG_Namespace   *AG_RegisterNamespace(const char *, const char *, const char *);
void            AG_UnregisterNamespace(const char *);

void AG_RegisterModuleDirectory(const char *);
void AG_UnregisterModuleDirectory(const char *);
void AG_RegisterClass(void *);
void AG_UnregisterClass(void *);
int  AG_ParseClassSpec(char *, size_t, char *, size_t, const char *)
                       BOUNDED_ATTRIBUTE(__string__, 1, 2)
                       BOUNDED_ATTRIBUTE(__string__, 3, 4);
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

/* Compare the specified object class against a given pattern. */
static __inline__ int
AG_ClassIsNamed(void *pClass, const char *pat)
{
	AG_ObjectClass *cls = pClass;
	const char *c;
	int nwild = 0;

	for (c = &pat[0]; *c != '\0'; c++) {
		if (*c == '*')
			nwild++;
	}
	if (nwild == 0) {
		return (strncmp(cls->name, pat, c - &pat[0]) == 0);
	} else if (nwild == 1) {
		if (pat[0] == '*') {
			return (1);
		}
		for (c = &pat[0]; *c != '\0'; c++) {
			if (c[0] == ':' && c[1] == '*' && c[2] == '\0') {
				if (c == &pat[0] ||
				    strncmp(cls->name, pat, c - &pat[0]) == 0)
					return (1);
			}
		}
		return (0);
	}
	return AG_ClassIsNamedGeneral(cls, pat);	/* General case */
}
__END_DECLS

#include "close_code.h"
