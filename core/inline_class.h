/*	Public domain	*/

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

/*
 * Compare the inheritance hierarchy of a class against a given pattern.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int
AG_ClassIsNamed(void *_Nonnull pClass, const char *_Nonnull pat)
#else
int
ag_class_is_named(void *pClass, const char *pat)
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
