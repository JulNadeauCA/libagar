/*
 * Copyright (c) 2002-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * settings, but applications are free to define new properties
 * (note: the "ag_" prefix is reserved for Agar settings).
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <string.h>

#include <agar/config/datadir.h>

/* Create the "save-path" / "tmp-path" directories, if they don't exist. */
int
AG_CreateDataDir(void)
{
	char dataDir[AG_PATHNAME_MAX];
	char tmpDir[AG_PATHNAME_MAX];

	AG_GetString(agConfig, "save-path", dataDir, sizeof(dataDir));
	AG_GetString(agConfig, "tmp-path", tmpDir, sizeof(tmpDir));

	if (AG_FileExists(dataDir) == 0 && AG_MkDir(dataDir) != 0)
		return (-1);
	if (AG_FileExists(tmpDir) == 0 && AG_MkDir(tmpDir) != 0)
		return (-1);
	
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
	return AG_ObjectLoad(agConfig);
}

int
AG_ConfigInit(AG_Config *cfg, Uint flags)
{
	char path[AG_FILENAME_MAX];
	AG_User *sysUser;
	int i;

	AG_ObjectInit(cfg, &agConfigClass);
	AG_ObjectSetName(cfg, "config");
	OBJECT(cfg)->save_pfx = NULL;

	for (i = 0; i < AG_CONFIG_PATH_LAST; i++)
		SLIST_INIT(&cfg->paths[i]);

	AG_SetInt(cfg, "initial-run", 1);
	AG_SetInt(cfg, "no-confirm-quit", 0);

	if (agProgName != NULL &&
	    (sysUser = AG_GetRealUser()) != NULL) {
		AG_SetString(cfg, "home", sysUser->home);
		AG_SetString(cfg, "tmp-path", sysUser->tmp);

		/* Add ~/.progname/ to load path. */
		Strlcpy(path, sysUser->home, sizeof(path));
		Strlcat(path, AG_PATHSEP, sizeof(path));
#ifndef _WIN32
		Strlcat(path, ".", sizeof(path));
#endif
		Strlcat(path, agProgName, sizeof(path));
		AG_ConfigAddPathS(AG_CONFIG_PATH_DATA, path);

		/* Also use ~/.progname as the default save path. */
		AG_SetString(cfg, "save-path", path);
	
		/*
		 * Add static DATADIR to load path if one is defined
		 * (./configure --datadir setting).
		 */
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
		AG_SetString(cfg, "save-path", ".");
		AG_SetString(cfg, "tmp-path", "tmp");
	}

	if ((flags & AG_CREATE_DATADIR) &&
	    AG_CreateDataDir() == -1) {
		return (-1);
	}
	return (0);
}

static int
Load(void *_Nonnull p, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
#ifdef AG_DEBUG
	agDebugLvl = AG_ReadUint8(ds);
#else
	(void)AG_ReadUint8(ds);
#endif
	/* For backward compatibility with <9.5 (pre-1.4.2) saves. */
	if (ver->minor < 2) { AG_ReadUint8(ds); }
	(void)AG_ReadUint8(ds);
	if (ver->minor >= 3) { AG_ReadUint8(ds); }
	if (ver->minor >= 4) { AG_ReadUint32(ds); }
	AG_Seek(ds, 22, AG_SEEK_CUR);
	if (ver->minor >= 1) { AG_ReadUint8(ds); }
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

/*
 * Search the load paths for the named file. If one is found and is accessible,
 * copy its absolute path name to the fixed-size buffer path and return 0.
 * If the file cannot be found, return -1.
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

#ifdef AG_DEBUG
	if (path_len < AG_FILENAME_MAX) { AG_FatalError("path_len too small"); }
#endif
	SLIST_FOREACH(loadPath, pathGroup, paths) {
		Strlcpy(file, loadPath->s, sizeof(file));
		Strlcat(file, AG_PATHSEP, sizeof(file));
		Strlcat(file, filename, sizeof(file));
		if ((rv = AG_FileExists(file)) == 1) {
			Strlcpy(path, file, path_len);
			if (loadPath != SLIST_FIRST(pathGroup)) {
				/*
				 * Sort paths by the most recently
				 * successfully accessed first.
				 */
				SLIST_REMOVE(pathGroup, loadPath, ag_config_path, paths);
				SLIST_INSERT_HEAD(pathGroup, loadPath, paths);
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

/* Return a pointer to the global agConfig object. */
AG_Config *
AG_ConfigObject(void)
{
	return (agConfig);
}

/* Add a directory for to the AG_ConfigFind() search path. */
void
AG_ConfigAddPathS(AG_ConfigPathGroup group, const char *_Nonnull s)
{
	AG_ConfigPath *loadPath = Malloc(sizeof(AG_ConfigPath));

	loadPath->s = Strdup(s);
#ifdef AG_DEBUG
	if (group >= AG_CONFIG_PATH_LAST) { AG_FatalError("Bad group"); }
#endif
	SLIST_INSERT_HEAD(&agConfig->paths[group], loadPath, paths);
}

void
AG_ConfigAddPath(AG_ConfigPathGroup group, const char *_Nonnull fmt, ...)
{
	AG_ConfigPath *loadPath = Malloc(sizeof(AG_ConfigPath));
	va_list ap;
	
	va_start(ap, fmt);
	Vasprintf(&loadPath->s, fmt, ap);
	va_end(ap);
#ifdef AG_DEBUG
	if (group >= AG_CONFIG_PATH_LAST) { AG_FatalError("Bad group"); }
#endif
	SLIST_INSERT_HEAD(&agConfig->paths[group], loadPath, paths);
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
	SLIST_FOREACH(loadPath, pathGroup, paths) {
#ifdef _WIN32
		if (Strcasecmp(loadPath->s, s) == 0)
			break;
#else
		if (strcmp(loadPath->s, s) == 0)
			break;
#endif
	}
	if (loadPath != NULL)
		SLIST_REMOVE(pathGroup, loadPath, ag_config_path, paths);
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
		AG_SetError("Bad path-key");
		return (-1);
	}
}
#endif /* AG_LEGACY */

AG_ObjectClass agConfigClass = {
	"Agar(Config)",
	sizeof(AG_Config),
	{ 9, 5 },
	NULL,
	NULL,
	NULL,
	Load,
	Save,
	NULL
};
