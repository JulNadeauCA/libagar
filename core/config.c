/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Global configuration object (agObject). Used to store Agar-related
 * (ag_*) settings, but application-defined settings can also be created.
 */

#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <string.h>

#include <agar/config/datadir.h>

const char *agConfigPathGroupNames[] = {
	N_("Application Data"),
	N_("Fonts"),
	N_("Temporary Files")
};

/* Initialize the AG_Config object. */
int
AG_ConfigInit(AG_Config *cfg, Uint flags)
{
	AG_ObjectInit(cfg, &agConfigClass);
	AG_ObjectSetName(cfg, "agConfig");

	if ((flags & AG_CREATE_DATADIR) &&
	    AG_CreateDataDir() == -1) {
		return (-1);
	}
	return (0);
}

/* Create the save and tmp directories if they don't exist. */
int
AG_CreateDataDir(void)
{
	char path[AG_PATHNAME_MAX];

	if (AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, path, sizeof(path)) < sizeof(path)) {
		if (AG_FileExists(path) == 0 && AG_MkDir(path) != 0)
			return (-1);
	}
	if (AG_ConfigGetPath(AG_CONFIG_PATH_TEMP, 0, path, sizeof(path)) < sizeof(path)) {
		if (AG_FileExists(path) == 0 && AG_MkDir(path) != 0)
			return (-1);
	}
	return (0);
}

int
AG_ConfigSave(void)
{
	if (AG_CreateDataDir() == -1 ||
	    AG_ObjectSave(agConfig) == -1) {
		return (-1);
	}
	return (0);
}

int
AG_ConfigLoad(void)
{
#ifdef AG_DEBUG
	int debugLvl;
#endif
	int rv;

	Debug_Mute(debugLvl);
	rv = AG_ObjectLoad(agConfig);
	Debug_Unmute(debugLvl);
	return (rv);
}

static void
Init(void *_Nonnull obj)
{
	char path[AG_FILENAME_MAX];
	AG_Config *cfg = obj;
	AG_User *sysUser;
#ifdef AG_DEBUG
	int debugLvlSave;
#endif
	int i;
	
	for (i = 0; i < AG_CONFIG_PATH_LAST; i++)
		TAILQ_INIT(&cfg->paths[i]);
#ifdef AG_DEBUG
	debugLvlSave = agDebugLvl;
	agDebugLvl = 0;
#endif
	AG_SetInt(cfg, "initial-run", 1);
	AG_SetInt(cfg, "no-confirm-quit", 0);

	if (agProgName != NULL &&
	    (sysUser = AG_GetRealUser()) != NULL) {
		/* Set the default temp directory. */
		AG_SetString(cfg, "home", sysUser->home);
		AG_ConfigSetPathS(AG_CONFIG_PATH_TEMP, 0, sysUser->tmp);

		/*
		 * Add $HOME/.progname to data path. Since it is the first
		 * entry, it is also the default location where object files
		 * will be saved (formerly known as "save-path" before 1.6.0).
		 */
		Strlcpy(path, sysUser->home, sizeof(path));
		Strlcat(path, AG_PATHSEP, sizeof(path));
#ifndef _WIN32
		Strlcat(path, ".", sizeof(path));
#endif
		Strlcat(path, agProgName, sizeof(path));
		AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, path);

		/* Add ./configure --datadir setting to data path. */
		if (strcmp(DATADIR, "NONE") != 0) {
			AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, DATADIR);
		}
		AG_UserFree(sysUser);
	} else {
		AG_SetString(cfg, "home", "");
		
		if (strcmp(DATADIR, "NONE") != 0) {
			AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, DATADIR);
		} else {
			AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, ".");
		}
		AG_ConfigSetPathS(AG_CONFIG_PATH_TEMP, 0, "tmp");
	}
#ifdef AG_DEBUG
	agDebugLvl = debugLvlSave;
#endif
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
#ifdef AG_DEBUG
	agDebugLvl = AG_ReadUint8(ds);
#else
	(void)AG_ReadUint8(ds);
#endif
#ifdef AG_LEGACY
	if (ver->minor < 2) { AG_ReadUint8(ds); } /* r<9.5 (pre-1.4.2) compat */
#endif
	(void)AG_ReadUint8(ds);
#ifdef AG_LEGACY
	if (ver->minor >= 3) { AG_ReadUint8(ds); }
	if (ver->minor >= 4) { AG_ReadUint32(ds); }
#endif
	AG_Seek(ds, 22, AG_SEEK_CUR);
#ifdef AG_LEGACY
	if (ver->minor >= 1) { AG_ReadUint8(ds); }
#endif
	(void)AG_ReadUint8(ds);				/* agRcsMode */
	AG_SkipString(ds);				/* agRcsHostname */
	(void)AG_ReadUint16(ds);			/* agRcsPort */
	AG_SkipString(ds);				/* agRcsUsername */
	AG_SkipString(ds);				/* agRcsPassword */
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	AG_Config *cfg = obj;
	char buf[30];

	AG_SetInt(cfg, "initial-run", 0);
#ifdef AG_DEBUG
	AG_WriteUint8(ds, (Uint8)agDebugLvl);
#else
	AG_WriteUint8(ds, 0);
#endif
	/* For backward compatibility with <9.5 (pre-1.4.2) saves. */
	memset(buf, 0, sizeof(buf));
	AG_Write(ds, buf, sizeof(buf));
	AG_WriteString(ds, "");			/* agRcsHostname */
	AG_WriteUint16(ds, 0);			/* agRcsPort */
	AG_WriteString(ds, "");			/* agRcsUsername */
	AG_WriteString(ds, "");			/* agRcsPassword */
	return (0);
}

void
AG_ConfigClearPaths(AG_Config *cfg)
{
	AG_ConfigPath *cp, *cpNext;
	int i;

	for (i = 0; i < AG_CONFIG_PATH_LAST; i++) {
		AG_ConfigPathQ *pathq = &cfg->paths[i];

		for (cp = TAILQ_FIRST(pathq);
		     cp != TAILQ_END(pathq);
		     cp = cpNext) {
			cpNext = TAILQ_NEXT(cp, paths);
			Free(cp->s);
			free(cp);
		}
		TAILQ_INIT(pathq);
	}
}

/* Return a pointer to the global agConfig object. */
AG_Config *
AG_ConfigObject(void)
{
	return (agConfig);
}

/*
 * Copy the path at index idx to a fixed-size buffer.
 * If no match (or no such group), return 0 and NUL-terminate the buffer.
 * Otherwise, return the length of the string that would have been created
 * were bufSize unlimited.
 */
AG_Size
AG_ConfigGetPath(AG_ConfigPathGroup group, int idx, char *buf, AG_Size bufSize)
{
	const AG_ConfigPathQ *pathGroup = &agConfig->paths[group];
	const AG_ConfigPath *path;
	int i;

	if (group >= AG_CONFIG_PATH_LAST || idx < 0) {
		goto no_match;
	}
	i = 0;
	TAILQ_FOREACH(path, pathGroup, paths) {
		if (i++ == idx) {
			return Strlcpy(buf, path->s, bufSize);
		} else if (i > idx) {
			break;
		}
	}
no_match:
	if (bufSize > 0) { buf[0] = '\0'; }
	return (0);
}

/*
 * Set the path at index idx.
 * If the entry at idx does not exist, create it.
 * If creating a new entry, idx must be (last)-1.
 */
int
AG_ConfigSetPath(AG_ConfigPathGroup group, int idx, const char *fmt, ...)
{
	va_list ap;
	char *s;
	
	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	return AG_ConfigSetPathS(group, idx, s);
}
int
AG_ConfigSetPathS(AG_ConfigPathGroup group, int idx, const char *buf)
{
	const AG_ConfigPathQ *pathGroup = &agConfig->paths[group];
	AG_ConfigPath *path;
	int i;

	if (group >= AG_CONFIG_PATH_LAST || idx < 0) {
		AG_SetErrorS("Bad path group/index");
		return (-1);
	}
	i = 0;
	TAILQ_FOREACH(path, pathGroup, paths) {
		if (i++ == idx) {
			Free(path->s);
			path->s = Strdup(buf);
			return (0);
		} else if (i > idx) {
			AG_SetErrorS("Bad index");
			return (-1);
		}
	}
	AG_ConfigAddPathS(group, buf);
	return (0);
}

/*
 * Search the specified path group for the given filename, test for its
 * existence and if the file exists, copy its fullpath to a fixed-size
 * buffer and return 0. If the file does not exist, return -1.
 *
 * Designated paths groups (such as FONTS) may have their entries
 * reordered based on the most recent successful access.
 */
int
AG_ConfigFind(AG_ConfigPathGroup group, const char *filename, char *path,
    AG_Size path_len)
{
	char file[AG_PATHNAME_MAX];
	AG_Config *cfg = agConfig;
	AG_ConfigPathQ *pathGroup = &cfg->paths[group];
	AG_ConfigPath *loadPath;
	int rv;

	TAILQ_FOREACH(loadPath, pathGroup, paths) {
		Strlcpy(file, loadPath->s, sizeof(file));
		Strlcat(file, AG_PATHSEP, sizeof(file));
		Strlcat(file, filename, sizeof(file));
		if ((rv = AG_FileExists(file)) == 1) {
			if (Strlcpy(path, file, path_len) >= path_len) {
				AG_SetErrorS(_("Path overflow"));
				return (-1);
			}
			if (group == AG_CONFIG_PATH_FONTS &&
			    loadPath != TAILQ_FIRST(pathGroup)) {
				/*
				 * Sort the font paths based on most recent
				 * successful access.
				 */
				TAILQ_REMOVE(pathGroup, loadPath, paths);
				TAILQ_INSERT_HEAD(pathGroup, loadPath, paths);
			}
			return (0);
		} else if (rv == -1) {
			AG_SetError("%s: %s", file, AG_GetError());
			return (-1);
		}
	}
	AG_SetError(_("Cannot find %s in load-path."), filename);
	return (-1);
}

/* Add a directory for to the AG_ConfigFind() search path. */
void
AG_ConfigAddPathS(AG_ConfigPathGroup group, const char *_Nonnull s)
{
	AG_ConfigPath *loadPath;

	loadPath = Malloc(sizeof(AG_ConfigPath));
	loadPath->s = Strdup(s);
#ifdef AG_DEBUG
	if (group >= AG_CONFIG_PATH_LAST) { AG_FatalError("Bad group"); }
#endif
	TAILQ_INSERT_TAIL(&agConfig->paths[group], loadPath, paths);
}

void
AG_ConfigAddPath(AG_ConfigPathGroup group, const char *_Nonnull fmt, ...)
{
	AG_ConfigPath *loadPath;
	va_list ap;
	
	loadPath = Malloc(sizeof(AG_ConfigPath));
	va_start(ap, fmt);
	Vasprintf(&loadPath->s, fmt, ap);
	va_end(ap);
#ifdef AG_DEBUG
	if (group >= AG_CONFIG_PATH_LAST) { AG_FatalError("Bad group"); }
#endif
	TAILQ_INSERT_TAIL(&agConfig->paths[group], loadPath, paths);
}

/* Remove a directory from the AG_ConfigFind() search path. */
void
AG_ConfigDelPathS(AG_ConfigPathGroup group, const char *_Nonnull s)
{
	AG_ConfigPathQ *pathGroup;
	AG_ConfigPath *loadPath;

#ifdef AG_DEBUG
	if (group >= AG_CONFIG_PATH_LAST) { AG_FatalError("Bad group"); }
#endif
	pathGroup = &agConfig->paths[group];
	TAILQ_FOREACH(loadPath, pathGroup, paths) {
#ifdef _WIN32
		if (Strcasecmp(loadPath->s, s) == 0)
			break;
#else
		if (strcmp(loadPath->s, s) == 0)
			break;
#endif
	}
	if (loadPath != NULL)
		TAILQ_REMOVE(pathGroup, loadPath, paths);
}

void
AG_ConfigDelPath(AG_ConfigPathGroup group, const char *_Nonnull fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	AG_ConfigDelPathS(group, s);
	free(s);
}

#ifdef AG_LEGACY
int
AG_ConfigFile(const char *path_key, const char *name, const char *ext,
    char *path, AG_Size path_len)
{
	char filename[AG_FILENAME_MAX];

	Strlcpy(filename, name, sizeof(filename));
	Strlcat(filename, ".", sizeof(filename));
	Strlcat(filename, ext, sizeof(filename));

	if (strcmp(path_key, "load-path") == 0) {
		return AG_ConfigFind(AG_CONFIG_PATH_DATA, filename,
		    path, path_len);
	} else if (strcmp(path_key, "font-path") == 0) {
		return AG_ConfigFind(AG_CONFIG_PATH_FONTS, filename,
		    path, path_len);
	} else {
		AG_SetError("path_key != {load,font}-path");
		return (-1);
	}
}
#endif /* AG_LEGACY */

AG_ObjectClass agConfigClass = {
	"AG_Config",
	sizeof(AG_Config),
	{ 10, 0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	Load,
	Save,
	NULL		/* edit */
};

#endif /* AG_SERIALIZATION */
