/*
 * Copyright (c) 2003-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include <string.h>

extern AG_ObjectClass agObjectClass;

AG_ObjectClass *agClassTree = NULL;		/* Root */
AG_Namespace    *agNamespaceTbl = NULL;
int              agNamespaceCount = 0;
char           **agModuleDirs = NULL;
int              agModuleDirCount = 0;
AG_Mutex	 agClassLock;

static void
InitClass(AG_ObjectClass *cls, const char *name, const char *libs)
{
	Strlcpy(cls->name, name, sizeof(cls->name));
	Strlcpy(cls->libs, libs, sizeof(cls->libs));
	TAILQ_INIT(&cls->sub);
}

/* Initialize the object class description table. */
void
AG_InitClassTbl(void)
{
	agNamespaceTbl = Malloc(sizeof(AG_Namespace));
	agNamespaceCount = 0;
	agModuleDirs = Malloc(sizeof(char *));
	agModuleDirCount = 0;

	AG_RegisterNamespace("Agar", "AG_", "http://libagar.org/");
	
	agClassTree = Malloc(sizeof(AG_ObjectClass));
	memcpy(agClassTree, &agObjectClass, sizeof(AG_ObjectClass));
	InitClass(agClassTree, "AG_Object", "");
	
	AG_MutexInitRecursive(&agClassLock);
}

/* Release the object class description table. */
void
AG_DestroyClassTbl(void)
{
	Free(agClassTree);
	agClassTree = NULL;

	Free(agNamespaceTbl);
	agNamespaceTbl = NULL;
	agNamespaceCount = 0;
	
	AG_MutexDestroy(&agClassLock);
}

/* 
 * Convert a class specification in "Namespace1(Class1:Class2)[@lib]" format
 * to the flat "PFX_Class1:PFX_Class2" format. dstLib may be NULL.
 */
int
AG_ParseClassSpec(char *dst, size_t dstLen, char *dstLibs, size_t dstLibsLen,
    const char *name)
{
	char buf[AG_OBJECT_TYPE_MAX], *pBuf, *pTok;
	char nsName[AG_OBJECT_TYPE_MAX], *pNsName = nsName;
	const char *s, *p, *pOpen = NULL;
	int iTok, i = 0;
	AG_Namespace *ns;

	if (strlen(name) >= AG_OBJECT_TYPE_MAX) {
		AG_SetError("Class string too large");
		return (-1);
	}

	if (dstLibs != NULL) { *dstLibs = '\0'; }
	*pNsName = '\0';
	for (s = &name[0]; *s != '\0'; s++) {
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
			if (dstLibs != NULL && s[0] == '@' && s[1] != '\0')
				Strlcpy(dstLibs, &s[1], dstLibsLen);
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
				i += Strlcpy(&dst[i], ns->pfx, dstLen-i);
				i += Strlcpy(&dst[i], pTok, dstLen-i);
				i += Strlcpy(&dst[i], ":", dstLen-i);
			}
			pOpen = NULL;
			continue;
		}
	}
	if (i > 0 && dst[i-1] == ':') {
		dst[i-1] = '\0';		/* Strip last ':' */
	}
	if (i == 0) {				/* Flat format */
		Strlcpy(dst, name, dstLen);
	} else {
		dst[i] = '\0';
	}
	return (0);
}

/* Register object class as described by the given AG_ObjectClass structure. */
void
AG_RegisterClass(void *p)
{
	AG_ObjectClass *cls = p;
	char name[AG_OBJECT_TYPE_MAX];
	char libs[AG_OBJECT_TYPE_MAX];
	char *s;

	/* Parse the inheritance hierarchy string. */
	if (AG_ParseClassSpec(name, sizeof(name), libs, sizeof(libs),
	    cls->name) == -1) {
		AG_FatalError("%s: %s", cls->name, AG_GetError());
	}
	InitClass(cls, name, libs);
	Debug(NULL, "Registered class: %s (%s)\n", name, libs);

	/* Lookup the superclass. */
	AG_MutexLock(&agClassLock);
	if ((s = strrchr(name, ':')) != NULL) {
		*s = '\0';
		if ((cls->super = AG_LookupClass(name)) == NULL)
			AG_FatalError("%s: No such superclass", name);
	} else {
		cls->super = agClassTree;			/* Root */
	}
	TAILQ_INSERT_TAIL(&cls->super->sub, cls, subclasses);
	AG_MutexUnlock(&agClassLock);
}

/* Unregister an object class. */
void
AG_UnregisterClass(void *p)
{
	AG_ObjectClass *cls = p;
	AG_ObjectClass *clsSuper = cls->super;

	AG_MutexLock(&agClassLock);
	Debug(NULL, "Unregistering class: %s\n", cls->name);
	TAILQ_REMOVE(&clsSuper->sub, cls, subclasses);
	cls->super = NULL;
	AG_MutexUnlock(&agClassLock);
}

/* XXX ridiculous */
static AG_ObjectClass *
FindClassByName(AG_ObjectClass *cls, const char *spec)
{
	AG_ObjectClass *subcls, *rv;
	
	TAILQ_FOREACH(subcls, &cls->sub, subclasses) {
		if (strcmp(subcls->name, spec) == 0) {
			return (subcls);
		}
		if ((rv = FindClassByName(subcls, spec)) != NULL)
			return (rv);
	}
	return (NULL);
}

/*
 * Return information about the currently registered class matching the
 * given specification.
 */
AG_ObjectClass *
AG_LookupClass(const char *inSpec)
{
	char spec[AG_OBJECT_TYPE_MAX];
	AG_ObjectClass *cls;

	if (inSpec[0] == '\0' ||
	    strcmp(inSpec, "Agar(Object)") == 0 ||
	    strcmp(inSpec, "AG_Object") == 0)
		return (agClassTree);				/* Root */

	/* Prepend the implied "AG_Object:" */
	AG_ParseClassSpec(spec, sizeof(spec), NULL, 0, inSpec);

	/* Recursively search the class structure. */
	AG_MutexLock(&agClassLock);
	if ((cls = FindClassByName(agClassTree, spec)) != NULL) {
		AG_MutexUnlock(&agClassLock);
		return (cls);
	}
	AG_MutexUnlock(&agClassLock);
	AG_SetError("No such class: %s", inSpec);
	return (NULL);
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
	char className[AG_OBJECT_TYPE_MAX];
	char libs[AG_OBJECT_TYPE_MAX], *s, *lib;
	char sym[AG_OBJECT_TYPE_MAX];
	AG_DSO *dso;
	void *pClass = NULL;
	int i;

	AG_ParseClassSpec(className, sizeof(className), libs, sizeof(libs),
	    classSpec);

	if (libs[0] == '\0')
		return AG_LookupClass(className);

	AG_MutexLock(&agClassLock);

	/* Load all libraries specified in the string. */
	for (i = 0, s = libs;
	    (lib = Strsep(&s, ", ")) != NULL;
	    i++) {
		if ((dso = AG_LoadDSO(lib, 0)) == NULL) {
			AG_SetError("Loading <%s>: %s", classSpec,
			    AG_GetError());
			goto fail;
		}
		/* Look up "pfxFooClass" in the first library. */
		if (i == 0) {
			if ((s = strrchr(className, ':')) != NULL &&
			    s[1] != '\0') {
				Strlcpy(sym, &s[1], sizeof(sym));
			} else {
				Strlcpy(sym, s, sizeof(sym));
			}
			Strlcpy(sym, className, sizeof(sym));
			Strlcat(sym, "Class", sizeof(sym));

			if (AG_SymDSO(dso, sym, &pClass) == -1) {
				AG_SetError("<%s>: %s", lib, AG_GetError());
				AG_UnloadDSO(dso);
				/* XXX TODO undo other DSOs we just loaded */
				goto fail;
			}
		}
	}
	if (pClass == NULL) {
		AG_SetError("Loading <%s>: No library specified", classSpec);
		goto fail;
	}
	AG_RegisterClass(pClass);

	AG_MutexUnlock(&agClassLock);
	return (pClass);
fail:
	AG_MutexUnlock(&agClassLock);
	return (pClass);
}

/*
 * Unregister the given class and decrement the reference count / unload
 * related dynamically-linked libraries.
 */
void
AG_UnloadClass(AG_ObjectClass *cls)
{
	char *s, *lib;
	AG_DSO *dso;
	
	AG_UnregisterClass(cls);

	for (s = cls->libs; (lib = Strsep(&s, ", ")) != NULL; ) {
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
	if (i == agNamespaceCount) {
		return;
	}
	if (i < agNamespaceCount-1) {
		memmove(&agNamespaceTbl[i], &agNamespaceTbl[i+1],
		    (agNamespaceCount-1)*sizeof(AG_Namespace));
	}
	agNamespaceCount--;
}

/* Register a new module directory path. */
void
AG_RegisterModuleDirectory(const char *path)
{
	char *s, *p;

	agModuleDirs = Realloc(agModuleDirs,
	    (agModuleDirCount+1)*sizeof(char *));
	agModuleDirs[agModuleDirCount++] = s = Strdup(path);
	if (*(p = &s[strlen(s)-1]) == AG_PATHSEPC)
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
	if (i == agModuleDirCount) {
		return;
	}
	free(agModuleDirs[i]);
	if (i < agModuleDirCount-1) {
		memmove(&agModuleDirs[i], &agModuleDirs[i+1],
		    (agModuleDirCount-1)*sizeof(char *));
	}
	agModuleDirCount--;
}

/* General case fallback for AG_ClassIsNamed() */
int
AG_ClassIsNamedGeneral(const AG_ObjectClass *cls, const char *cn)
{
	char cname[AG_OBJECT_TYPE_MAX], *cp, *c;
	char nname[AG_OBJECT_TYPE_MAX], *np, *s;

	Strlcpy(cname, cn, sizeof(cname));
	Strlcpy(nname, cls->name, sizeof(nname));
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
	char cname[AG_OBJECT_TYPE_MAX], *c;
	AG_ObjectClass *cl;
	int i, stop = 0;

	if (AGOBJECT(obj)->cls->name[0] == '\0') {
		(*nHier) = 0;
		return (0);
	}
	(*nHier) = 1;
	Strlcpy(cname, AGOBJECT(obj)->cls->name, sizeof(cname));
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
