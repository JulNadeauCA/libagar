/*
 * Copyright (c) 2003-2015 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/config/ag_debug_core.h>

#include <string.h>
#include <ctype.h>

extern AG_ObjectClass agObjectClass;

AG_Tbl          *agClassTbl = NULL;		/* Classes in hash table */
AG_ObjectClass  *agClassTree = NULL;		/* Classes in tree format */
AG_Namespace    *agNamespaceTbl = NULL;		/* Registered namespaces */
int              agNamespaceCount = 0;
char           **agModuleDirs = NULL;		/* Module search directories */
int              agModuleDirCount = 0;
AG_Mutex	 agClassLock;			/* Lock on class table */

static void
InitClass(AG_ObjectClass *cl, const char *hier, const char *libs)
{
	const char *c;

	Strlcpy(cl->hier, hier, sizeof(cl->hier));
	Strlcpy(cl->libs, libs, sizeof(cl->libs));

	if ((c = strrchr(hier, ':')) != NULL && c[1] != '\0') {
		Strlcpy(cl->name, &c[1], sizeof(cl->name));
	} else {
		Strlcpy(cl->name, hier, sizeof(cl->name));
	}

	TAILQ_INIT(&cl->sub);
}

/*
 * Initialize the object class description table.
 * Invoked internally by AG_InitCore().
 */
void
AG_InitClassTbl(void)
{
	AG_Variable V;

	/* Initialize the namespaces */
	agNamespaceTbl = Malloc(sizeof(AG_Namespace));
	agNamespaceCount = 0;
	agModuleDirs = Malloc(sizeof(char *));
	agModuleDirCount = 0;
	AG_RegisterNamespace("Agar", "AG_", "http://libagar.org/");

	/* Initialize the class tree */
	InitClass(&agObjectClass, "AG_Object", "");
	agClassTree = &agObjectClass;

	/* Initialize the class table. */
	agClassTbl = AG_TblNew(256, 0);
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
	int i;

	free(agNamespaceTbl); agNamespaceTbl = NULL;
	agNamespaceCount = 0;

	for (i = 0; i < agModuleDirCount; i++) { free(agModuleDirs[i]); }
	free(agModuleDirs);
	agModuleDirs = NULL;
	agModuleDirCount = 0;

	agClassTree = NULL;
	
	AG_TblDestroy(agClassTbl);
	free(agClassTbl); agClassTbl = NULL;
	
	AG_MutexDestroy(&agClassLock);
}

/* Convert a class specification in "Namespace1(Class1:Class2)[@lib]" format. */
int
AG_ParseClassSpec(AG_ObjectClassSpec *cs, const char *spec)
{
	char buf[AG_OBJECT_HIER_MAX], *pBuf, *pTok;
	char nsName[AG_OBJECT_HIER_MAX], *pNsName = nsName;
	const char *s, *p, *pOpen = NULL;
	char *c;
	int iTok, i = 0;
	AG_Namespace *ns;

	if (strlen(spec) >= AG_OBJECT_HIER_MAX) {
		AG_SetError("Hierarchy string too long");
		return (-1);
	}

	/* Parse the inheritance hierarchy and library list. */
	cs->hier[0] = '\0';
	cs->libs[0] = '\0';
	cs->name[0] = '\0';
	*pNsName = '\0';
	for (s = &spec[0]; *s != '\0'; s++) {
		if (s[0] == '(' && s[1] != '\0') {
			if (pOpen || nsName[0] == '\0') {
				AG_SetError("Syntax error");
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
			if (s[0] == '@' && s[1] != '\0')
				Strlcpy(cs->libs, &s[1], sizeof(cs->libs));
		}
		if (*s == ')') {
			if ((s - pOpen) == 0) {
				pOpen = NULL;
				continue;
			}
			*pNsName = '\0';
			pNsName = &nsName[0];
			if ((ns = AG_GetNamespace(nsName)) == NULL) {
				AG_SetError("No such namespace: \"%s\"",
				    nsName);
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
				i += Strlcpy(&cs->hier[i], ns->pfx,
				    sizeof(cs->hier)-i);
				i += Strlcpy(&cs->hier[i], pTok,
				    sizeof(cs->hier)-i);
				i += Strlcpy(&cs->hier[i], ":",
				    sizeof(cs->hier)-i);
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
		Strlcpy(cs->hier, spec, sizeof(cs->hier));
	} else {
		cs->hier[i] = '\0';
	}
	if ((c = strrchr(cs->hier, '@')) != NULL)
		*c = '\0';

	/* Fill in the "name" field. */
	if ((c = strrchr(cs->hier, ':')) != NULL && c[1] != '\0') {
		Strlcpy(cs->name, &c[1], sizeof(cs->name));
	} else {
		Strlcpy(cs->name, cs->hier, sizeof(cs->name));
	}
	if ((c = strrchr(cs->name, '@')) != NULL)
		*c = '\0';
	
	/* Fill in the "spec" field. */
	Strlcpy(cs->spec, cs->hier, sizeof(cs->spec));
	if (cs->libs[0] != '\0') {
		Strlcat(cs->spec, cs->libs, sizeof(cs->spec));
	}
	return (0);
}

/* Register object class as described by the given AG_ObjectClass structure. */
void
AG_RegisterClass(void *p)
{
	AG_ObjectClass *cl = p;
	AG_ObjectClassSpec cs;
	AG_Variable V;
	char *s;
	
	/* Parse the class specification. */
	if (AG_ParseClassSpec(&cs, cl->hier) == -1) {
		AG_FatalError(NULL);
	}
	InitClass(cl, cs.hier, cs.libs);
	
#ifdef AG_DEBUG_CORE
	Debug(NULL, "Registered class: %s: %s (%s)\n", cs.name, cs.hier,
	    cs.libs);
#endif

	AG_MutexLock(&agClassLock);

	/* Insert into the class tree. */
	if ((s = strrchr(cs.hier, ':')) != NULL) {
		*s = '\0';
		if ((cl->super = AG_LookupClass(cs.hier)) == NULL)
			AG_FatalError(NULL);
	} else {
		cl->super = agClassTree;			/* Root */
	}
	TAILQ_INSERT_TAIL(&cl->super->sub, cl, subclasses);

	/* Insert into the class table. */
	AG_InitPointer(&V, cl);
	if (AG_TblInsert(agClassTbl, cl->hier, &V) == -1)
		AG_FatalError(NULL);

	AG_MutexUnlock(&agClassLock);
}

/* Unregister an object class. */
void
AG_UnregisterClass(void *p)
{
	AG_ObjectClass *cl = p;
	AG_ObjectClass *clSuper = cl->super;
	Uint h = AG_TblHash(agClassTbl, cl->hier);

	AG_MutexLock(&agClassLock);
	if (AG_TblExistsHash(agClassTbl, h, cl->hier)) {
#ifdef AG_DEBUG_CORE
		Debug(NULL, "Unregistering class: %s\n", cl->name);
#endif
		/* Remove from the class tree. */
		TAILQ_REMOVE(&clSuper->sub, cl, subclasses);
		cl->super = NULL;

		/* Remove from the class table. */
		AG_TblDeleteHash(agClassTbl, h, cl->hier);
	}
	AG_MutexUnlock(&agClassLock);
}

/*
 * Return information about the currently registered class matching the
 * given specification.
 */
AG_ObjectClass *
AG_LookupClass(const char *inSpec)
{
	AG_ObjectClassSpec cs;
	AG_Variable *V;

	if (inSpec[0] == '\0' ||
	    strcmp(inSpec, "Agar(Object)") == 0 ||
	    strcmp(inSpec, "AG_Object") == 0)
		return (agClassTree);				/* Root */

	if (AG_ParseClassSpec(&cs, inSpec) == -1)
		return (NULL);

	/* Look up the class table. */
	AG_MutexLock(&agClassLock);
	if ((V = AG_TblLookup(agClassTbl, cs.hier)) != NULL) {
		AG_MutexUnlock(&agClassLock);
		return ((AG_ObjectClass *)V->data.p);
	}
	AG_MutexUnlock(&agClassLock);

	AG_SetError("No such class: %s", inSpec);
	return (NULL);
}

/* Convert "PFX_Foo" to "pfxFooClass". */
static int
GetClassSymbol(char *sym, size_t len, const AG_ObjectClassSpec *cs)
{
	char *d;
	const char *c;
	int inPfx = 1;
	size_t l = 0;

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
	AG_SetError("Symbol too long");
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
	AG_ObjectClass *cl;
	char *s, *lib;
	char sym[AG_OBJECT_HIER_MAX];
	AG_DSO *dso;
	void *pClass = NULL;
	int i;

	if (AG_ParseClassSpec(&cs, classSpec) == -1) {
		return (NULL);
	}
	
	AG_MutexLock(&agClassLock);

	if ((cl = AG_LookupClass(cs.hier)) != NULL) {
		AG_MutexUnlock(&agClassLock);
		return (cl);
	}
	if (cs.libs[0] == '\0') {
		AG_SetError("Class %s not found (and no modules specified)",
		    cs.hier);
		goto fail;
	}

	/* Load all libraries specified in the string. */
	for (i = 0, s = cs.libs;
	    (lib = Strsep(&s, ", ")) != NULL;
	    i++) {
#ifdef AG_DEBUG_CORE
		Debug(NULL, "<%s>: Linking %s...", classSpec, lib);
#endif
		if ((dso = AG_LoadDSO(lib, 0)) == NULL) {
			AG_SetError("Loading <%s>: %s", classSpec, AG_GetError());
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
#ifdef AG_DEBUG_CORE
		Debug(NULL, "OK\n");
#endif
	}
	if (pClass == NULL) {
		AG_SetError("Loading <%s>: No library specified", classSpec);
		goto fail;
	}
	AG_RegisterClass(pClass);

	AG_MutexUnlock(&agClassLock);
	return (pClass);
fail:
#ifdef AG_DEBUG_CORE
	Debug(NULL, "%s\n", AG_GetError());
#endif
	AG_MutexUnlock(&agClassLock);
	return (pClass);
}

/*
 * Unregister the given class and decrement the reference count / unload
 * related dynamically-linked libraries.
 */
void
AG_UnloadClass(AG_ObjectClass *cl)
{
	char *s, *lib;
	AG_DSO *dso;
	
	AG_UnregisterClass(cl);

	for (s = cl->libs; (lib = Strsep(&s, ", ")) != NULL; ) {
		if ((dso = AG_LookupDSO(lib)) != NULL)
			AG_UnloadDSO(dso);
	}
}

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

/* General case fallback for AG_ClassIsNamed() */
int
AG_ClassIsNamedGeneral(const AG_ObjectClass *cl, const char *cn)
{
	char cname[AG_OBJECT_HIER_MAX], *cp, *c;
	char nname[AG_OBJECT_HIER_MAX], *np, *s;

	Strlcpy(cname, cn, sizeof(cname));
	Strlcpy(nname, cl->hier, sizeof(nname));
	cp = cname;
	np = nname;
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
 * Return an array of class structures describing the inheritance
 * hierarchy of an object.
 * XXX
 */
int
AG_ObjectGetInheritHier(void *obj, AG_ObjectClass ***hier, int *nHier)
{
	char cname[AG_OBJECT_HIER_MAX], *c;
	AG_ObjectClass *cl;
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
	*hier = Malloc((*nHier)*sizeof(AG_ObjectClass *));
	i = 0;
	for (c = &cname[0];; c++) {
		if (*c != ':' && *c != '\0') {
			continue;
		}
		if (*c == '\0') {
			stop++;
		} else {
			*c = '\0';
		}
		if ((cl = AG_LookupClass(cname)) == NULL) {
			Free(*hier);
			return (-1);
		}
		*c = ':';
		(*hier)[i++] = cl;
		
		if (stop)
			break;
	}
	return (0);
}
