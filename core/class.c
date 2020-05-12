/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Functions related to Agar object classes and namespaces.
 */

#include <agar/core/core.h>

#include <string.h>
#include <ctype.h>

/* Debug class registration and module linking */
/* #define AG_DEBUG_CLASSES */

extern AG_ObjectClass agObjectClass;

#ifdef AG_THREADS
AG_Mutex         agClassLock;		/* Lock on class table */
#endif
AG_ObjectClass **agClasses;		/* Array of registered classes */
Uint             agClassCount = 0;
AG_Tbl          *agClassTbl = NULL;	/* Classes in hash table */
#ifdef AG_NAMESPACES
AG_Namespace *agNamespaceTbl = NULL;	/* Registered namespaces */
int           agNamespaceCount = 0;
#endif
#ifdef AG_ENABLE_DSO
char **agModuleDirs = NULL;		/* Module search directories */
int    agModuleDirCount = 0;
#endif

static void
InitClass(AG_ObjectClass *_Nonnull C, const char *_Nonnull hier)
{
	const char *c;
	AG_Size rv;

	if (Strlcpy(C->hier, hier, sizeof(C->hier)) >= sizeof(C->hier))
		goto too_big;

	if ((c = strrchr(hier, ':')) != NULL && c[1] != '\0') {
		rv = Strlcpy(C->name, &c[1], sizeof(C->name));
	} else {
		rv = Strlcpy(C->name, hier, sizeof(C->name));
	}
	if (rv >= sizeof(C->name)) {
		goto too_big;
	}
	TAILQ_INIT(&C->pvt.sub);
	return;
too_big:
	AG_FatalError("Class name overflow");
}

/*
 * Initialize the object class description table.
 * Invoked internally by AG_InitCore().
 */
void
AG_InitClassTbl(void)
{
	AG_Variable V;

#ifdef AG_NAMESPACES
	agNamespaceTbl = Malloc(sizeof(AG_Namespace));
	agNamespaceCount = 0;
	AG_RegisterNamespace("Agar", "AG_", "http://libagar.org/");
#endif
#ifdef AG_ENABLE_DSO
	agModuleDirs = Malloc(sizeof(char *));
	agModuleDirCount = 0;
#endif
	/* Initialize the class tree */
	InitClass(&agObjectClass, "AG_Object");
#ifdef AG_ENABLE_DSO
	agObjectClass.pvt.libs[0] = '\0';
#endif
	/* Initialize the class table. */
	agClassTbl = AG_TblNew(AG_OBJECT_CLASSTBLSIZE, 0);

	/* AG_Object -> agObjectClass */
	AG_InitPointer(&V, &agObjectClass);
	if (AG_TblInsert(agClassTbl, "AG_Object", &V) == -1)
		AG_FatalError(NULL);

	AG_MutexInitRecursive(&agClassLock);
}

/*
 * Release the object class description table.
 * Invoked internally by AG_Destroy().
 */
void
AG_DestroyClassTbl(void)
{
#ifdef AG_NAMESPACES
	free(agNamespaceTbl);
	agNamespaceTbl = NULL;
	agNamespaceCount = 0;
#endif
#ifdef AG_ENABLE_DSO
	{
		int i;

		for (i = 0; i < agModuleDirCount; i++) {
			free(agModuleDirs[i]);
		}
		free(agModuleDirs);
		agModuleDirs = NULL;
		agModuleDirCount = 0;
	}
#endif
	AG_TblDestroy(agClassTbl);
	free(agClassTbl); agClassTbl = NULL;
	
	AG_MutexDestroy(&agClassLock);
}

#ifdef AG_NAMESPACES
/*
 * Parse a class specification string either in the conventional form
 * "AG_Class1:AG_Class2:...[@lib]", or in "Agar(Class1:Class2:...)[@lib]"
 * format if NAMESPACE is supported.
 */
int
AG_ParseClassSpec(AG_ObjectClassSpec *cs, const char *spec)
{
	char buf[AG_OBJECT_HIER_MAX], *pBuf, *pTok;
	char nsName[AG_OBJECT_HIER_MAX], *pNsName = nsName;
	const char *s, *p, *pOpen = NULL;
	char *c;
	AG_Namespace *ns;
	AG_Size rv;
	int iTok, i=0, len=0;

	cs->hier[0] = '\0';
	cs->name[0] = '\0';
# ifdef AG_ENABLE_DSO
	cs->libs[0] = '\0';
# endif
	*pNsName = '\0';
	for (s = &spec[0]; *s != '\0'; s++) {
		if (++len >= AG_OBJECT_HIER_MAX) {
			AG_SetErrorV("E22", _("Class is too long"));
			return (-1);
		}
		if (s[0] == '(' && s[1] != '\0') {
			if (pOpen || nsName[0] == '\0') {
				AG_SetErrorV("E23", _("Class syntax error"));
				return (-1);
			}
			pOpen = &s[1];
			continue;
		}
		if (pOpen == NULL) {
			if (*s != ':') {
				*pNsName = *s;
				pNsName++;
			}
# ifdef AG_ENABLE_DSO
			if (s[0] == '@' && s[1] != '\0')
				if (Strlcpy(cs->libs, &s[1], sizeof(cs->libs))
				    >= sizeof(cs->libs))
					AG_FatalError("DSO name overflow");
# endif
		}
		if (*s == ')') {
			if ((s - pOpen) == 0) {
				pOpen = NULL;
				continue;
			}
			*pNsName = '\0';
			pNsName = &nsName[0];
			if ((ns = AG_GetNamespace(nsName)) == NULL) {
				AG_SetErrorV("E24", _("No such namespace"));
				return (-1);
			}
			for (p = pOpen, iTok = 0;
			     (p < s) && (iTok < sizeof(buf)-1);
			     p++, iTok++) {
				buf[iTok] = *p;
			}
			buf[iTok] = '\0';
			for (pBuf = buf;
			     (pTok = Strsep(&pBuf, ":")) != NULL; ) {
				i += Strlcpy(&cs->hier[i], ns->pfx, sizeof(cs->hier)-i);
				i += Strlcpy(&cs->hier[i], pTok, sizeof(cs->hier)-i);
				i += Strlcpy(&cs->hier[i], ":", sizeof(cs->hier)-i);
			}
			pOpen = NULL;
			continue;
		}
	}

	/* Fill in the "hier" field. */
	if (i > 0 && cs->hier[i-1] == ':') {
		cs->hier[i-1] = '\0';		/* Strip last ':' */
	}
	if (i == 0) {				/* Flat format */
#ifdef AG_DEBUG
		if (Strlcpy(cs->hier, spec, sizeof(cs->hier)) >= sizeof(cs->hier))
			AG_FatalError("Class hierarchy overflow");
#else
		Strlcpy(cs->hier, spec, sizeof(cs->hier));
#endif
	} else {
		cs->hier[i] = '\0';
	}
	if ((c = strrchr(cs->hier, '@')) != NULL)
		*c = '\0';

	/* Fill in the "name" field. */
	if ((c = strrchr(cs->hier, ':')) != NULL && c[1] != '\0') {
		rv = Strlcpy(cs->name, &c[1], sizeof(cs->name));
	} else {
		rv = Strlcpy(cs->name, cs->hier, sizeof(cs->name));
	}
	if (rv >= sizeof(cs->name))
		AG_FatalError("Class name overflow");

	if ((c = strrchr(cs->name, '@')) != NULL)
		*c = '\0';
	
	/* Fill in the "spec" field. */
	Strlcpy(cs->spec, cs->hier, sizeof(cs->spec));
# ifdef AG_ENABLE_DSO
	if (cs->libs[0] != '\0')
		if (Strlcat(cs->spec, cs->libs, sizeof(cs->spec)) >=
		    sizeof(cs->spec))
			AG_FatalError("DSO name overflow");
# endif
	return (0);
}
#else /* !AG_NAMESPACES */
/*
 * Parse a class specification string only in the conventional format
 * "AG_Class1:AG_Class2:...[@lib]" (no namespace support).
 */
int
AG_ParseClassSpec(AG_ObjectClassSpec *cs, const char *spec)
{
	char *c;

	Strlcpy(cs->hier, spec, sizeof(cs->hier));
	Strlcpy(cs->spec, spec, sizeof(cs->spec));
	
	if ((c = strchr(cs->hier, '@')) != NULL) {
# ifdef AG_ENABLE_DSO
		Strlcpy(cs->libs, &c[1], sizeof(cs->libs));
# endif
		*c = '\0';
	}
# ifdef AG_ENABLE_DSO
	else {
		cs->libs[0] = '\0';
	}
# endif
	if ((c = strrchr(spec, ':')) != NULL && c[1] != '\0') {
		Strlcpy(cs->name, &c[1], sizeof(cs->name));
	} else {
		Strlcpy(cs->name, spec, sizeof(cs->name));
	}
	return (0);
}
#endif /* !AG_NAMESPACES */

/* Register object class as described by the given AG_ObjectClass structure. */
void
AG_RegisterClass(void *p)
{
	AG_ObjectClass *C = p;
	AG_ObjectClassSpec cs;
	AG_Variable V;
	char *s;
	
	if (AG_ParseClassSpec(&cs, C->hier) == -1) {
		AG_FatalError(NULL);
	}
	InitClass(C, cs.hier);
#ifdef AG_ENABLE_DSO
	Strlcpy(C->pvt.libs, cs.libs, sizeof(C->pvt.libs));
#endif
#ifdef AG_DEBUG_CLASSES
	Debug(NULL, "[ Register %s (%s) ]\n", cs.name, cs.hier);
#endif
	AG_MutexLock(&agClassLock);

	/* Insert into the class tree. */
	if ((s = strrchr(cs.hier, ':')) != NULL) {
		*s = '\0';
		if ((C->super = AG_LookupClass(cs.hier)) == NULL)
			AG_FatalError(NULL);
	} else {
		C->super = &agObjectClass;	/* Base AG_Object class */
	}
	TAILQ_INSERT_TAIL(&C->super->pvt.sub, C, pvt.subclasses);

	/* Insert into the class table. */
	AG_InitPointer(&V, C);
	if (AG_TblInsert(agClassTbl, C->hier, &V) == -1)
		AG_FatalError(NULL);

	AG_MutexUnlock(&agClassLock);
}

/* Unregister an object class. */
void
AG_UnregisterClass(void *p)
{
	AG_ObjectClass *C = p;
	AG_ObjectClass *Csuper = C->super;
	Uint h;

	AG_MutexLock(&agClassLock);
	h = AG_TblHash(agClassTbl, C->hier);
	if (AG_TblExistsHash(agClassTbl, h, C->hier)) {
#ifdef AG_DEBUG_CLASSES
		Debug(NULL, "[ Unregister %s ]\n", C->name);
#endif
		/* Remove from the class tree. */
		TAILQ_REMOVE(&Csuper->pvt.sub, C, pvt.subclasses);
		C->super = NULL;

		/* Remove from the class table. */
		AG_TblDeleteHash(agClassTbl, h, C->hier);
	}
	AG_MutexUnlock(&agClassLock);
}

#if AG_MODEL != AG_SMALL
/*
 * Allocate, initialize and zero an AG_ObjectClass (or derivative thereof).
 *
 * This gives an alternative to passing a statically-initialized AG_ObjectClass
 * to AG_RegisterClass(). Here we auto-allocate it instead,
 * and the methods can be set using AG_ClassSet{Init,Reset,Destroy,...}().
 */
void *
AG_CreateClass(const char *hier, AG_Size objectSize, AG_Size classSize,
    Uint major, Uint minor)
{
	AG_ObjectClass *C;

	if ((C = TryMalloc(classSize)) == NULL) {
		return (NULL);
	}
	memset(C, 0, classSize);
	if (Strlcpy(C->hier, hier, sizeof(C->hier)) >= sizeof(C->hier)) {
		AG_FatalError("Class hierarchy overflow");
	}
	C->size = objectSize;
	C->ver.major = major;
	C->ver.minor = minor;
	AG_RegisterClass(C);
	return (C);
}

/* Set Object class operations procedurally. */
#define AG_CLASS_SET_FN_BODY(fnName, fnType, op) \
fnType fnName (void *Cp, fnType fn) { \
	AG_ObjectClass *C = (AG_ObjectClass *)Cp; \
	fnType fnOrig = C->op; \
	C->op = fn; \
	return (fnOrig); \
}
AG_CLASS_SET_FN_BODY(AG_ClassSetInit,    AG_ObjectInitFn,    init);
AG_CLASS_SET_FN_BODY(AG_ClassSetReset,   AG_ObjectResetFn,   reset);
AG_CLASS_SET_FN_BODY(AG_ClassSetDestroy, AG_ObjectDestroyFn, destroy);
AG_CLASS_SET_FN_BODY(AG_ClassSetLoad,    AG_ObjectLoadFn,    load);
AG_CLASS_SET_FN_BODY(AG_ClassSetSave,    AG_ObjectSaveFn,    save);
AG_CLASS_SET_FN_BODY(AG_ClassSetEdit,    AG_ObjectEditFn,    edit);
#undef AG_CLASS_SET_FN_BODY

/* Unregister and free an auto-allocated AG_ObjectClass (or derivative thereof) */
void
AG_DestroyClass(void *C)
{
	AG_UnregisterClass(C);
	free(C);
}
#endif /* !AG_SMALL */

/*
 * Lookup information about a registered object class.
 * Return a normalized class description (or NULL if no such class exists).
 */
AG_ObjectClass *
AG_LookupClass(const char *inSpec)
{
	AG_ObjectClassSpec cs;
	AG_Variable *V;

	if (inSpec[0] == '\0' ||
#ifdef AG_NAMESPACES
	    strcmp(inSpec, "Agar(Object)") == 0 ||
#endif
	    strcmp(inSpec, "AG_Object") == 0)
		return (&agObjectClass);

	if (AG_ParseClassSpec(&cs, inSpec) == -1)
		return (NULL);

	/* Look up the class table. */
	AG_MutexLock(&agClassLock);
	if ((V = AG_TblLookup(agClassTbl, cs.hier)) != NULL) {
		AG_MutexUnlock(&agClassLock);
		return ((AG_ObjectClass *)V->data.p);
	}
	AG_MutexUnlock(&agClassLock);
#ifdef AG_VERBOSITY
	AG_SetError(_("No such class \"%s\". "
	              "Did you forget AG_RegisterClass()?"), inSpec);
#else
	AG_SetErrorV("E25", _("No such class"));
#endif
	return (NULL);
}

#ifdef AG_ENABLE_DSO
/*
 * Transform "PFX_Foo" string to "pfxFooClass".
 */
static int
GetClassSymbol(char *_Nonnull sym, AG_Size len,
    const AG_ObjectClassSpec *_Nonnull cs)
{
	char *d;
	const char *c;
	int inPfx = 1;
	AG_Size l = 0;

	for (c = &cs->name[0], d = &sym[0];
	     *c != '\0';
	     c++) {
		if (*c == '_') {
			inPfx = 0;
			continue;
		}
		if ((l+2) >= len) {
			goto toolong;
		}
		*d = inPfx ? (char) tolower((int) *c) : *c;
		d++;
		l++;
	}
	*d = '\0';
	if (Strlcat(sym, "Class", len) >= len) {
		goto toolong;
	}
	return (0);
toolong:
	AG_SetErrorS(_("Symbol is too long"));
	return (-1);
}

/*
 * Look for a "@libs" string in the class specification and scan module
 * directories for the required libraries. If they are found, bring them
 * into the current process's address space. If successful, look up the
 * "pfxFooClass" symbol and register the class.
 *
 * Multiple libraries can be specified with commas. The "pfxFooClass"
 * symbol is assumed to be defined in the first library in the list.
 */
AG_ObjectClass *
AG_LoadClass(const char *classSpec)
{
	AG_ObjectClassSpec cs;
	AG_ObjectClass *C;
	char *s, *lib;
	char sym[AG_OBJECT_HIER_MAX];
	AG_DSO *dso;
	void *pClass = NULL;
	int i;

	if (AG_ParseClassSpec(&cs, classSpec) == -1)
		return (NULL);
	
	AG_MutexLock(&agClassLock);

	if ((C = AG_LookupClass(cs.hier)) != NULL) {
		AG_MutexUnlock(&agClassLock);
		return (C);
	}
	if (cs.libs[0] == '\0') {
		AG_SetError(_("Class %s not found (and no modules specified)"),
		    cs.hier);
		goto fail;
	}

	/* Load all libraries specified in the string. */
	for (i = 0, s = cs.libs;
	    (lib = Strsep(&s, ", ")) != NULL;
	    i++) {
# ifdef AG_DEBUG_CLASSES
		Debug(NULL, "<%s>: Linking %s...", classSpec, lib);
# endif
		if ((dso = AG_LoadDSO(lib, 0)) == NULL) {
			AG_SetError("DSO(%s): %s", classSpec, AG_GetError());
			goto fail;
		}
		/* Look up "pfxFooClass" in the first library. */
		if (i == 0) {
			if (GetClassSymbol(sym, sizeof(sym), &cs) == -1) {
				goto fail;
			}
			if (AG_SymDSO(dso, sym, &pClass) == -1) {
				AG_UnloadDSO(dso);
				/* XXX TODO undo other DSOs we just loaded */
				goto fail;
			}
		}
# ifdef AG_DEBUG_CLASSES
		Debug(NULL, "OK\n");
# endif
	}
	if (pClass == NULL) {
		AG_SetError(_("<%s>: No library specified"), classSpec);
		goto fail;
	}
	AG_RegisterClass(pClass);

	AG_MutexUnlock(&agClassLock);
	return (pClass);
fail:
# ifdef AG_DEBUG_CLASSES
	Debug(NULL, "%s\n", AG_GetError());
# endif
	AG_MutexUnlock(&agClassLock);
	return (pClass);
}

/*
 * Unregister the given class and decrement the reference count / unload
 * related dynamically-linked libraries.
 */
void
AG_UnloadClass(AG_ObjectClass *C)
{
	char *s, *lib;
	AG_DSO *dso;
	
	AG_UnregisterClass(C);

	for (s = C->pvt.libs; (lib = Strsep(&s, ", ")) != NULL; ) {
		if ((dso = AG_LookupDSO(lib)) != NULL)
			AG_UnloadDSO(dso);
	}
}
#endif /* AG_ENABLE_DSO */

#ifdef AG_NAMESPACES
/* Register a new namespace. */
AG_Namespace *
AG_RegisterNamespace(const char *name, const char *pfx, const char *url)
{
	AG_Namespace *ns;

	agNamespaceTbl = Realloc(agNamespaceTbl,
	    (agNamespaceCount+1)*sizeof(AG_Namespace));
	ns = &agNamespaceTbl[agNamespaceCount++];
	ns->name = name;
	ns->pfx = pfx;
	ns->url = url;
	return (ns);
}

/* Unregister a namespace. */
void
AG_UnregisterNamespace(const char *name)
{
	int i;

	for (i = 0; i < agNamespaceCount; i++) {
		if (strcmp(agNamespaceTbl[i].name, name) == 0)
			break;
	}
	if (i < agNamespaceCount) {
		if (i < agNamespaceCount-1) {
			memmove(&agNamespaceTbl[i], &agNamespaceTbl[i+1],
			    (agNamespaceCount-i-1)*sizeof(AG_Namespace));
		}
		agNamespaceCount--;
	}
}
#endif /* AG_NAMESPACES */

#ifdef AG_ENABLE_DSO
/* Register a new module directory path. */
void
AG_RegisterModuleDirectory(const char *path)
{
	char *s, *p;

	agModuleDirs = Realloc(agModuleDirs,
	    (agModuleDirCount+1)*sizeof(char *));
	agModuleDirs[agModuleDirCount++] = s = Strdup(path);
	if (*(p = &s[strlen(s)-1]) == AG_PATHSEPCHAR)
		*p = '\0';
}

/* Unregister a module directory path. */
void
AG_UnregisterModuleDirectory(const char *path)
{
	int i;

	for (i = 0; i < agModuleDirCount; i++) {
		if (strcmp(agModuleDirs[i], path) == 0)
			break;
	}
	if (i < agModuleDirCount) {
		free(agModuleDirs[i]);
		if (i < agModuleDirCount-1) {
			memmove(&agModuleDirs[i], &agModuleDirs[i+1],
			    (agModuleDirCount-i-1)*sizeof(char *));
		}
		agModuleDirCount--;
	}
}
#endif /* AG_ENABLE_DSO */

/* General case fallback for AG_ClassIsNamed() */
int
AG_ClassIsNamedGeneral(const AG_ObjectClass *C, const char *cn)
{
	char cname[AG_OBJECT_HIER_MAX], *cp, *c;
	char nname[AG_OBJECT_HIER_MAX], *np, *s;

	Strlcpy(cname, cn, sizeof(cname));
	Strlcpy(nname, C->hier, sizeof(nname));
	cp = cname;
	np = nname;
	/*
	 * TODO: eliminating this strsep usage would allow AG_OfClass()
	 * to be made _Pure_Attribute_If_Unthreaded.
	 */
	while ((c = Strsep(&cp, ":")) != NULL &&
	       (s = Strsep(&np, ":")) != NULL) {
		if (c[0] == '*' && c[1] == '\0')
			continue;
		if (strcmp(c, s) != 0)
			return (0);
	}
	return (1);
}

/*
 * Return an array of class description pointers ("AG_ObjectClass *") for
 * each class in the inheritance hierarchy of obj. For example:
 *
 *   "AG_Widget:AG_Box:AG_Titlebar" -> { &agWidgetClass,
 *                                       &agBoxClass,
 *                                       &agTitlebarClass }
 */
int
AG_ObjectGetInheritHier(void *obj, AG_ObjectClass ***hier, int *nHier)
{
	char cname[AG_OBJECT_HIER_MAX], *c;
	AG_ObjectClass *C, **pHier;
	int i, stop = 0;

	if (AGOBJECT(obj)->cls->hier[0] == '\0') {
		(*nHier) = 0;
		return (0);
	}
	(*nHier) = 1;
	Strlcpy(cname, AGOBJECT(obj)->cls->hier, sizeof(cname));
	for (c = &cname[0]; *c != '\0'; c++) {
		if (*c == ':')
			(*nHier)++;
	}
	pHier = (*hier) = Malloc((*nHier)*sizeof(AG_ObjectClass *));
	for (c = &cname[0], i = 0; ; c++) {
		if (*c != ':' && *c != '\0') {
			continue;
		}
		if (*c == '\0') {
			stop++;
		} else {
			*c = '\0';
		}
		if ((C = AG_LookupClass(cname)) == NULL) {
			free(pHier);
			return (-1);
		}
		*c = ':';
		pHier[i++] = C;
		
		if (stop)
			break;
	}
	return (0);
}
