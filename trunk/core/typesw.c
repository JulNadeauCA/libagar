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
 * Functions related to Agar namespacses and classes. The class table is
 * useful for debugging and enables dynamic loading of archived objects.
 */

#include <core/core.h>

#include <string.h>

extern AG_ObjectClass agObjectClass;

AG_ObjectClass **agClassTbl = NULL;
int            agClassCount = 0;
AG_Namespace *agNamespaceTbl = NULL;
int           agNamespaceCount = 0;

/* Initialize the Agar class table. */
void
AG_InitClassTbl(void)
{
	agClassTbl = Malloc(sizeof(AG_ObjectClass *));
	agClassCount = 0;
	agNamespaceTbl = Malloc(sizeof(AG_Namespace));
	agNamespaceCount = 0;

	AG_RegisterNamespace("Agar", "AG_", "http://libagar.org/");
	AG_RegisterClass(&agObjectClass);
}

/* Free the Agar class table. */
void
AG_DestroyClassTbl(void)
{
	Free(agClassTbl);
	agClassTbl = NULL;
	agClassCount = 0;
	Free(agNamespaceTbl);
	agNamespaceTbl = NULL;
	agNamespaceCount = 0;
}

/* 
 * Convert a class specification in "Namespace1(Class1:Class2)" format
 * to the flat "PFX_Class1:PFX_Class2" format.
 */
int
AG_ParseClassSpec(char *dst, size_t dstLen, const char *name)
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
			dst[i--] = '\0';		/* Strip last ':' */
			pOpen = NULL;
			continue;
		}
	}
	if (i == 0) {					/* Flat format */
		Strlcpy(dst, name, dstLen);
	} else {
		dst[i] = '\0';
	}
	return (0);
}

/* Register the class as described by the given AG_ObjectClass structure. */
void
AG_RegisterClass(void *pClass)
{
	AG_ObjectClass *cls = pClass;
	char s[AG_OBJECT_TYPE_MAX];

	agClassTbl = Realloc(agClassTbl,
	    (agClassCount+1)*sizeof(AG_ObjectClass *));
	agClassTbl[agClassCount++] = cls;

	/* Parse "Namespace(Class1:Class2)" specifications. */
	if (AG_ParseClassSpec(s, sizeof(s), cls->name) == -1) {
		AG_FatalError("%s: %s", cls->name, AG_GetError());
	}
	Strlcpy(cls->name, s, sizeof(cls->name));
}

/* Unregister the specified class. */
void
AG_UnregisterClass(void *p)
{
	AG_ObjectClass *cls = p;
	int i;

	for (i = 0; i < agClassCount; i++) {
		if (agClassTbl[i] == cls)
			break;
	}
	if (i == agClassCount) {
		return;
	}
	if (i < agClassCount-1) {
		memmove(&agClassTbl[i], &agClassTbl[i+1],
		    (agClassCount-1)*sizeof(AG_ObjectClass *));
	}
	agClassCount--;
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

/* Unregister a previously registered namespace. */
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
