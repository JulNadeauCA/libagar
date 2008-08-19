/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
void AG_InitClassTbl(void);
void AG_DestroyClassTbl(void);

AG_Namespace *AG_RegisterNamespace(const char *, const char *, const char *);
void          AG_UnregisterNamespace(const char *);

void AG_RegisterClass(void *);
void AG_UnregisterClass(void *);
int  AG_ParseClassSpec(char *, size_t, const char *)
                       BOUNDED_ATTRIBUTE(__string__, 1, 2);

/* Return description for the given object class name. */
static __inline__ AG_ObjectClass *
AG_FindClass(const char *name)
{
	int i;

	for (i = 0; i < agClassCount; i++) {
		if (strcmp(agClassTbl[i]->name, name) == 0)
			return (agClassTbl[i]);
	}
	AG_SetError("No such class: %s", name);
	return (NULL);
}

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
__END_DECLS

#include "close_code.h"
