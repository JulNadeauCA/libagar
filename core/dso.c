/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Cross-platform interface to dynamic linking loader.
 */

#include <agar/config/ag_enable_dso.h>
#ifdef AG_ENABLE_DSO
 
#include <agar/config/have_dlopen.h>
#include <agar/config/have_dyld.h>
#include <agar/config/have_dyld_return_on_error.h>
#include <agar/config/have_shl_load.h>
#include <agar/config/have_dlfcn_h.h>
#include <agar/config/have_dl_h.h>
#include <agar/config/have_mach_o_dyld_h.h>

#include <agar/core/core.h>

#if defined(BEOS)
# include <kernel/image.h>
# include <string.h>
#elif defined(NETWARE)
# include <dlfcn.h>
#elif defined(OS390)
# include <dll.h>
#elif defined(_WIN32)
# include <agar/core/queue_close.h>			/* Conflicts */
#ifdef _XBOX
# include <xtl.h>
#else
# include <windows.h>
# if !defined(__CYGWIN__) && defined(__MINGW32__) && !defined(__MINGW64__)
#  include <dlfcn.h>					/* bug #219 */
# endif
#endif
# include <agar/core/queue_close.h>
# include <agar/core/queue.h>
#else
# include <string.h>
# include <errno.h>
# ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
# endif
# ifdef HAVE_DL_H
#  include <dl.h>
# endif
# ifdef HAVE_MACH_O_DYLD_H
#  include <mach-o/dyld.h>
# endif

# ifndef RTLD_NOW
#  define RTLD_NOW 1
# endif
# ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
# endif
#endif

#if defined(BEOS)
typedef struct ag_dso_beos {
	struct ag_dso dso;
	image_id handle;
} AG_DSO_BeOS;
#elif defined(OS2)
typedef struct ag_dso_os2 {
	struct ag_dso dso;
	HMODULE handle;
} AG_DSO_OS2;
#else
typedef struct ag_dso_generic {
	struct ag_dso dso;
	void *handle;
} AG_DSO_Generic;
#endif

struct ag_dsoq agLoadedDSOs = TAILQ_HEAD_INITIALIZER(agLoadedDSOs);
AG_Mutex agDSOLock;

/* Load a DSO using the load_add_on() interface on BeOS. */
#ifdef BEOS
static AG_DSO *_Nullable
LoadDSO_BEOS(const char *_Nonnull path)
{
	AG_DSO_BeOS *d;
	image_id handle;

	if ((handle = load_add_on(path)) < B_NO_ERROR) {
		AG_SetError("%s: %s", path, strerror(handle));
		return (NULL);
	}
	d = Malloc(sizeof(AG_DSO_BeOS));
	d->handle = handle;
	return (AG_DSO *)d;
}
#endif /* BEOS */

/* Load a DSO using the DosLoadModule() interface on OS/2. */
#ifdef OS2
static AG_DSO *_Nullable
LoadDSO_OS2(const char *_Nonnull path)
{
	AG_DSO_OS2 *d;
	char failedMod[256];
	HMODULE handle;
	int rv;

	rv = DosLoadModule(failedMod, sizeof(failedMod), path, &handle);
	if (rv != 0) {
		AG_SetError("%s: DosLoadModule() failed: %s", path, failedMod);
		return (NULL);
	}
	d = Malloc(sizeof(AG_DSO_OS2));
	d->handle = handle;
	return (AG_DSO *)d;
}
#endif /* OS2 */

/* Load a DSO using the dllload() interface on OS/390. */
#ifdef OS390
static AG_DSO *_Nullable
LoadDSO_OS390(const char *_Nonnull path)
{
	AG_DSO_Generic *d;
	
	d = Malloc(sizeof(AG_DSO_Generic));
	if ((d->handle = dllload(path)) == NULL) {
		AG_SetError("%s: dllload() failed", path);
		Free(d);
		return (NULL);
	}
	return (AG_DSO *)d;
}
#endif /* OS390 */

/* Load a DSO using the LoadLibraryExW() interface on Windows. */
#if defined(_WIN32) && !defined (_XBOX)
static AG_DSO *_Nullable
LoadDSO_WIN32(const char *_Nonnull path)
{
	AG_DSO_Generic *d;
	char buf[AG_PATHNAME_MAX], *p;
	UINT em;
	
	d = Malloc(sizeof(AG_DSO_Generic));

	Strlcpy(buf, path, sizeof(buf));
	for (p = buf; *p != '\0'; p++) {
		if (*p == '/') { *p = '\\'; }
	}
	em = SetErrorMode(SEM_FAILCRITICALERRORS);
	d->handle = LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (d->handle == NULL) {
		d->handle = LoadLibraryExA(path, NULL, 0);
		if (d->handle == NULL) {
			AG_SetError("%s: LoadLibraryEx() failed", path);
			Free(d);
			return (NULL);
		}
	}
	SetErrorMode(em);
	return (AG_DSO *)d;
}
#endif /* _WIN32 */

/* Load a DSO using the HP-UX shl_load() interface. */
#ifdef HAVE_SHL_LOAD
static AG_DSO *_Nullable
LoadDSO_SHL(const char *_Nonnull path)
{
	AG_DSO_Generic *d;

	d = Malloc(sizeof(AG_DSO_Generic));
	if ((d->handle = shl_load(path, BIND_IMMEDIATE, 0L)) == NULL) {
		AG_SetError("%s: shl_load() failed", path);
		Free(d);
		return (NULL);
	}
	return (AG_DSO *)d;
}
#endif /* HAVE_SHL_LOAD */

/* Load a DSO using the dyld NSLinkModule() interface. */
#ifdef HAVE_DYLD
#define DYLD_LIBRARY_HANDLE ((void *)-1)
static AG_DSO *_Nullable
LoadDSO_DYLD(const char *_Nonnull path)
{
	AG_DSO_Generic *d;
	NSObjectFileImage image;
	NSObjectFileImageReturnCode rv;

	d = Malloc(sizeof(AG_DSO_Generic));
	d->handle = NULL;
		
	rv = NSCreateObjectFileImageFromFile(path, &image);

	if (rv == NSObjectFileImageSuccess) {
#  if defined(HAVE_DYLD_RETURN_ON_ERROR)
		d->handle = (void *)NSLinkModule(image, path,
		    NSLINKMODULE_OPTION_RETURN_ON_ERROR|
		    NSLINKMODULE_OPTION_NONE);
		if (d->handle == NULL) {
			NSLinkEditErrors errors;
			int errorNumber;
			const char *fileName;
			const char *s = NULL;

			NSLinkEditError(&errors, &errorNumber, &fileName, &s);
			AG_SetError("%s", s);
			goto fail;
		}
#  else /* !HAVE_DYLD_RETURN_ON_ERROR */
		d->handle = (void *)NSLinkModule(image, path, FALSE);
		if (d->handle == NULL) {
			AG_SetError("%s: NSLinkModule() failed", path);
			goto fail;
		}
#  endif /* HAVE_DYLD_RETURN_ON_ERROR */

		NSDestroyObjectFileImage(image);
	} else if ((rv == NSObjectFileImageFormat ||
	            rv == NSObjectFileImageInappropriateFile) &&
		    NSAddLibrary(path) == TRUE) {
		d->handle = (void *)( (NSModule)DYLD_LIBRARY_HANDLE );
		if (d->handle == NULL) {
			AG_SetError("%s: NSAddLibrary() failed", path);
			goto fail;
		}
	}
	return (AG_DSO *)d;
fail:
	if (rv == NSObjectFileImageSuccess) {
		NSDestroyObjectFileImage(image);
	}
	Free(d);
	return (NULL);
}
#endif /* HAVE_DYLD */

/* Load a DSO using the standard dlopen() interface. */
#ifdef HAVE_DLOPEN
static AG_DSO *_Nullable
LoadDSO_DLOPEN(const char *_Nonnull path)
{
	AG_DSO_Generic *d;
	int flags = RTLD_NOW|RTLD_GLOBAL;
	
	d = Malloc(sizeof(AG_DSO_Generic));
# if defined(_AIX)
	/* Special archive.a(dso.so) syntax requires RTLD_MEMBER flag */
	if (strchr(&path[1], '(') && path[strlen(path)-1] == ')')
		flags |= RTLD_MEMBER;
# endif
      	if ((d->handle = dlopen(path, flags)) == NULL) {
		AG_SetError("%s: %s", path, dlerror());
		Free(d);
		return (NULL);
	}
	return (AG_DSO *)d;
}
#endif /* HAVE_DLOPEN */

/* Lookup a loaded DSO by name */
AG_DSO *
AG_LookupDSO(const char *name)
{
	AG_DSO *dso;

	AG_LockDSO();
	AG_TAILQ_FOREACH(dso, &agLoadedDSOs, dsos) {
		if (strcmp(dso->name, name) == 0)
			break;
	}
	AG_UnlockDSO();
	return (dso);
}

/*
 * Load the specified module into the process address space. If it already
 * exists, return the existing structure incrementing its reference count.
 */
AG_DSO *
AG_LoadDSO(const char *name, Uint flags)
{
	char path[AG_PATHNAME_MAX];
	AG_DSO *dso;
	int i;

	AG_MutexLock(&agDSOLock);
	
	/* See if module has already been loaded. */
	TAILQ_FOREACH(dso, &agLoadedDSOs, dsos) {
		if (strcmp(dso->name, name) == 0)
			break;
	}
	if (dso != NULL) {
		dso->refCount++;
		goto out;
	}

	/* Scan the module directories. */
	path[0] = '\0';
	for (i = 0; i < agModuleDirCount; i++) {
		/* Look for an exact file match. */
		Strlcpy(path, agModuleDirs[i], sizeof(path));
		Strlcat(path, AG_PATHSEP, sizeof(path));
#if defined(__AMIGAOS4__)
		Strlcat(path, name, sizeof(path));
		Strlcat(path, ".ixlibrary", sizeof(path));
#elif defined(HPUX)
		Strlcat(path, name, sizeof(path));
		Strlcat(path, ".sl", sizeof(path));
#elif defined(_WIN32) || defined(OS2)
		Strlcat(path, name, sizeof(path));
		Strlcat(path, ".dll", sizeof(path));
#else
		Strlcat(path, "lib", sizeof(path));
		Strlcat(path, name, sizeof(path));
		Strlcat(path, ".so", sizeof(path));
#endif
		if (AG_FileExists(path))
			break;

#if !defined(__AMIGAOS4__) && !defined(HPUX) && \
    !defined(_WIN32) && !defined(OS2)
		/*
		 * Look for a versioned library file.
		 */
    		{
			AG_Dir *dir;

			if ((dir = AG_OpenDir(agModuleDirs[i])) != NULL) {
				char latestFile[AG_FILENAME_MAX];
				int latestVer, j;
	
				latestFile[0] = '\0';
				latestVer = 0;
	
				for (j = 0; j < dir->nents; j++) {
					char *file = dir->ents[j];
					char pat[AG_FILENAME_MAX];
					int noffs;
					int verMin, verMaj;

					Strlcpy(pat, "lib", sizeof(pat));
					Strlcat(pat, name, sizeof(pat));
					Strlcat(pat, ".so.", sizeof(pat));
					noffs = (int)strlen(pat);
					if (strncmp(file, pat, noffs) != 0) {
						continue;
					}
					if (sscanf(&file[noffs], "%d.%d",
					    &verMin, &verMaj) != 2) {
						continue;
					}
					if ((verMaj*10000 + verMin) > latestVer) {
						latestVer = verMaj*10000 + verMin;
						Strlcpy(latestFile, file, sizeof(latestFile));
					}
				}
				AG_CloseDir(dir);
				if (latestFile[0] != '\0') {
					Strlcpy(path, agModuleDirs[i], sizeof(path));
					Strlcat(path, AG_PATHSEP, sizeof(path));
					Strlcat(path, latestFile, sizeof(path));
					break;
				}
			}
		}
#endif /* UNIX */
	}
	if (i == agModuleDirCount) {
		AG_SetError("Cannot find \"%s\" module (last checked in: %s)",
		    name, path);
		goto fail;
	}

	/* Load the module in memory. */
	Verbose("Loading: %s (%s)\n", name, path);
#if defined(BEOS)
	dso = LoadDSO_BEOS(path);
#elif defined(OS2)
	dso = LoadDSO_OS2(path);
#elif defined(OS390)
	dso = LoadDSO_OS390(path);
#elif defined(_WIN32) && !defined(_XBOX)
	dso = LoadDSO_WIN32(path);
#elif defined(HAVE_SHL_LOAD)
	dso = LoadDSO_SHL(path);
#elif defined(HAVE_DLOPEN)
	dso = LoadDSO_DLOPEN(path);
#elif defined(HAVE_DYLD)
	dso = LoadDSO_DYLD(path);
#else
	AG_SetError("Dynamic linking is not supported on this platform");
	dso = NULL;
#endif
	if (dso == NULL)
		goto fail;

	Strlcpy(dso->name, name, sizeof(dso->name));
	Strlcpy(dso->path, path, sizeof(dso->path));
	dso->flags = 0;
	dso->refCount = 1;
	TAILQ_INIT(&dso->syms);
	TAILQ_INSERT_TAIL(&agLoadedDSOs, dso, dsos);
out:
	AG_MutexUnlock(&agDSOLock);
	return (dso);
fail:
	AG_MutexUnlock(&agDSOLock);
	return (NULL);
}

/*
 * Decrement the reference count of the specified DSO object. If it reaches
 * zero, unload the module from the process's address space.
 */
int
AG_UnloadDSO(AG_DSO *dso)
{
	AG_DSOSym *cSym, *cSymNext;
	int rv = -1;

	AG_MutexLock(&agDSOLock);

	if (--dso->refCount > 0) {
		rv = 0;
		goto out;
	}

#if defined(BEOS)
	{
		AG_DSO_BeOS *d = (AG_DSO_BeOS *)dso;
		if (unload_add_on(d->handle) < B_NO_ERROR) {
			AG_SetError("%s: unload_add_on() failed", dso->name);
			goto out;
		}
	}
#elif defined(OS2)
	{
		AG_DSO_OS2 *d = (AG_DSO_OS2 *)dso;
		if (DosFreeModule(d->handle) != 0) {
			AG_SetError("%s: DosFreeModule() failed", dso->name);
			goto out;
		}
	}
#elif defined(OS390)
	{
		AG_DSO_Generic *d = (AG_DSO_Generic *)dso;
		if (dllfree(d->handle) != 0) {
			AG_SetError("%s: dllfree() failed", dso->name);
			goto out;
		}
	}
#elif defined(_WIN32) && !defined(_XBOX)
	{
		AG_DSO_Generic *d = (AG_DSO_Generic *)dso;
		if (!FreeLibrary(d->handle)) {
			AG_SetError("%s: FreeLibrary() failed", dso->name);
			goto out;
		}
	}
#elif defined(HAVE_SHL_LOAD)
	{
		AG_DSO_Generic *d = (AG_DSO_Generic *)dso;
		shl_unload((shl_t)d->handle);
	}
#elif defined(HAVE_DYLD)
	{
		AG_DSO_Generic *d = (AG_DSO_Generic *)dso;
		if (d->handle != DYLD_LIBRARY_HANDLE)
			NSUnLinkModule(d->handle, FALSE);
	}
#elif defined(HAVE_DLOPEN)
	{
		AG_DSO_Generic *d = (AG_DSO_Generic *)dso;
# ifdef NETWARE
		void *NLMHandle = getnlmhandle();
		TAILQ_FOREACH(cSym, &dso->syms, syms)
			UnImportPublicObject(NLMHandle, cSym->sym);
# endif
		if (dlclose(d->handle) != 0) {
			AG_SetError("%s: dlclose: %s", dso->name,
			    strerror(errno));
			goto out;
		}
	}
#else
	AG_SetError("Dynamic linking is not supported on this platform");
	goto out;
#endif /* DSO_USE_FOO */

	for (cSym = TAILQ_FIRST(&dso->syms);
	     cSym != TAILQ_END(&dso->syms);
	     cSym = cSymNext) {
		cSymNext = TAILQ_NEXT(cSym, syms);
		Free(cSym->sym);
		Free(cSym);
	}
	TAILQ_REMOVE(&agLoadedDSOs, dso, dsos);
	Free(dso);
	rv = 0;
out:
	AG_MutexUnlock(&agDSOLock);
	return (rv);
}

#ifdef BEOS
/* Look up a symbol using the BeOS get_image_symbol() interface. */
static int
SymDSO_BeOS(AG_DSO_BeOS *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	AG_DSO_BeOS *d = (AG_DSO_BeOS *)dso;
	int rv;

	rv = get_image_symbol(d->handle, sym, B_SYMBOL_TYPE_ANY, p);
	if (rv != B_OK) {
		AG_SetError("Symbol not found: \"%s\"", sym);
		return (-1);
	}
	return (0);
}
#endif /* BEOS */

#ifdef OS2
/* Look up a symbol using the OS/2 DosQueryProcAddr() interface. */
static int
SymDSO_OS2(AG_DSO_OS2 *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	PFN fn;

	rv = DosQueryProcAddr(d->handle, 0, sym, &fn);
	if (rv != 0) {
		AG_SetError("Symbol not found: \"%s\"", sym);
		return (-1);
	}
	*p = (void *)fn;
	return (0);
}
#endif /* OS2 */

#ifdef OS390
/* Look up a symbol using the OS/390 dllqueryvar() interface. */
static int
SymDSO_OS390(AG_DSO_Generic *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	if ((*p = dllqueryfn(d->handle, sym)) != NULL ||
	    (*p = dllqueryvar(d->handle, sym)) != NULL) {
		return (0);
	}
	AG_SetError("Symbol not found: \"%s\"", sym);
	return (-1);
}
#endif /* OS390 */

#if defined(_WIN32) && !defined(_XBOX)
/* Look up a symbol using the Windows GetProcAddress() interface. */
static int
SymDSO_WIN32(AG_DSO_Generic *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	*p = (void *)GetProcAddress(d->handle, sym);
	/* XXX no way to determine error */
	return (0);
}
#endif /* _WIN32 */

#ifdef HAVE_SHL_LOAD
/* Look up a symbol using the shl_findsym() interface. */
static int
SymDSO_SHL(AG_DSO_Generic *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	int rv;

	*p = NULL;
	errno = 0;
	if ((rv = shl_findsym((shl_t *)&d->handle, sym, TYPE_PROCEDURE, p))
	    == -1) {
		if (errno != 0) {
			goto notfound;
		}
		if ((rv = shl_findsym((shl_t *)&d->handle, sym, TYPE_DATA, p))
		    == -1) {
			goto notfound;
		}
	}
	return (0);
notfound:
	AG_SetError("%s: Symbol not found: \"%s\"", AGDSO(d)->name, sym);
	return (-1);
}
#endif /* HAVE_SHL_LOAD */

#ifdef HAVE_DYLD
/* Look up a symbol using the dyld interface. */
static int
SymDSO_DYLD(AG_DSO_Generic *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	NSSymbol symbol;
	size_t symLen = strlen(sym);
	char *symUnder = Malloc(symLen+2);

	symUnder[0] = '_';
	Strlcpy(&symUnder[1], sym, (symLen+2)-1);
	*p = NULL;
# ifdef NSLINKMODULE_OPTION_PRIVATE
	if (d->handle == DYLD_LIBRARY_HANDLE) {
		symbol = NSLookupAndBindSymbol(symUnder);
	} else {
		symbol = NSLookupSymbolInModule((NSModule)d->handle, symUnder);
	}
# else
	symbol = NSLookupAndBindSymbol(symUnder);
# endif
	Free(symUnder);

	if (symbol == NULL) {
		AG_SetError("%s: Undefined symbol: %s", AGDSO(d)->name, sym);
		return (-1);
	}
	*p = NSAddressOfSymbol(symbol);
	return (0);
}
#endif /* HAVE_DYLD */

#ifdef HAVE_DLOPEN
/* Look up a symbol using the standard dlopen() interface. */
static int
SymDSO_DLOPEN(AG_DSO_Generic *_Nonnull d, const char *_Nonnull sym,
    void *_Nonnull *_Nullable p)
{
	char *error;

	*p = NULL;

# if !defined(__ELF__) && (defined(__NetBSD__) || defined(__OpenBSD__) || \
     defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FabBSD__))
	{
		size_t symLen = strlen(sym);
		char *symUnder = Malloc(symLen+2);

		symUnder[0] = '_';
		Strlcpy(&symUnder[1], sym, (symLen+2)-1);
		*p = dlsym(d->handle, symUnder);
		Free(symUnder);
	}
# else /* __ELF__ */
	*p = dlsym(d->handle, sym);
# endif /* !__ELF__ */

	if (*p == NULL &&
	    (error = dlerror()) != NULL) {
		AG_SetError("%s: %s", AGDSO(d)->name, error);
		return (-1);
	}
	return (0);
}
#endif /* HAVE_DLOPEN */

/* Resolve a symbol in the specified DSO. */
int
AG_SymDSO(AG_DSO *dso, const char *sym, void **p)
{
	AG_DSOSym *cSym;
	int rv = 0;
	
	AG_MutexLock(&agDSOLock);
#if defined(BEOS)
	rv = SymDSO_BeOS((AG_DSO_BeOS *)dso, sym, p);
#elif defined(OS2)
	rv = SymDSO_OS2((AG_DSO_OS2 *)dso, sym, p);
#elif defined(OS390)
	rv = SymDSO_OS390((AG_DSO_Generic *)dso, sym, p);
#elif defined(_WIN32) && !defined(_XBOX)
	rv = SymDSO_WIN32((AG_DSO_Generic *)dso, sym, p);
#elif defined(HAVE_SHL_LOAD)
	rv = SymDSO_SHL((AG_DSO_Generic *)dso, sym, p);
#elif defined(HAVE_DYLD)
	rv = SymDSO_DYLD((AG_DSO_Generic *)dso, sym, p);
#elif defined(HAVE_DLOPEN)
	rv = SymDSO_DLOPEN((AG_DSO_Generic *)dso, sym, p);
#else
	AG_SetError("Dynamic linking is not supported on this platform");
	rv = -1;
#endif
	if (rv == 0) {
		TAILQ_FOREACH(cSym, &dso->syms, syms) {
			if (strcmp(cSym->sym, sym) == 0)
				break;
		}
		if (cSym == NULL) {
			cSym = Malloc(sizeof(AG_DSOSym));
			cSym->sym = Strdup(sym);
			cSym->p = p;
			TAILQ_INSERT_TAIL(&dso->syms, cSym, syms);
		}
	}
	AG_MutexUnlock(&agDSOLock);
	return (rv);
}

/* Return the list of available modules in the registered directories. */
char **
AG_GetDSOList(Uint *count)
{
	char **list;
	AG_Dir *dir;
	int i, j;
	
	list = Malloc(sizeof(char *));
	*count = 0;
	
	for (i = 0; i < agModuleDirCount; i++) {
		if ((dir = AG_OpenDir(agModuleDirs[i])) == NULL) {
			continue;
		}
		for (j = 0; j < dir->nents; j++) {
			char file[AG_FILENAME_MAX];
			char *pStart, *s;

			Strlcpy(file, dir->ents[j], sizeof(file));
			if (file[0] == '.')
				continue;
#if defined(__AMIGAOS4__)
			if ((s = (char *)Strcasestr(file, ".ixlibrary")) == NULL ||
			    s[10] != '\0') {
				continue;
			}
			pStart = s;
#elif defined(HPUX)
			if ((s = (char *)Strcasestr(file, ".sl")) == NULL ||
			    s[3] != '\0') {
				continue;
			}
			pStart = s;
#elif defined(_WIN32) || defined(OS2)
			if ((s = (char *)Strcasestr(file, ".dll")) == NULL ||
			    s[4] != '\0') {
				continue;
			}
			pStart = s;
#else
			if (strncmp(file, "lib", 3) != 0 ||
			   (s = (char *)Strcasestr(file, ".so")) == NULL ||
			   s[3] != '\0') {
				continue;
			}
			pStart = &file[3];
#endif
			*s = '\0';

			list = Realloc(list, ((*count)+1)*sizeof(char *));
			list[(*count)++] = Strdup(pStart);
		}
		AG_CloseDir(dir);
	}
	list[(*count)] = NULL;				/* NULL-terminated */
	return (list);
}

void
AG_FreeDSOList(char **list, Uint count)
{
	Uint i;

	for (i = 0; i < count; i++) {
		Free(list[i]);
	}
	free(list);
}

#endif /* AG_ENABLE_DSO */
