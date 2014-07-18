/*
 * Copyright (c) 2002-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
	char path[AG_PATHNAME_MAX], *s;
	AG_User *sysUser;

	AG_ObjectInit(cfg, &agConfigClass);
	AG_ObjectSetName(cfg, "config");
	OBJECT(cfg)->save_pfx = NULL;

	AG_SetInt(cfg, "initial-run", 1);
	AG_SetInt(cfg, "no-confirm-quit", 0);

	if (agProgName != NULL &&
	    (sysUser = AG_GetRealUser()) != NULL) {
		AG_SetString(cfg, "home", sysUser->home);
		AG_SetString(cfg, "tmp-path", sysUser->tmp);

		Strlcpy(path, sysUser->home, sizeof(path));
		Strlcat(path, AG_PATHSEP, sizeof(path));
#ifndef _WIN32
		Strlcat(path, ".", sizeof(path));
#endif
		Strlcat(path, agProgName, sizeof(path));
		AG_SetString(cfg, "save-path", path);

#ifndef _WIN32
		if (strcmp(DATADIR, "NONE") != 0) {
			AG_PrtString(cfg, "load-path", "%s%s%s",
			    path, AG_PATHSEPMULTI, DATADIR);
		} else
#endif
		{
			AG_SetString(cfg, "load-path", path);
		}


		AG_UserFree(sysUser);
	} else {
		AG_SetString(cfg, "home", "");
		s = (strcmp(DATADIR,"NONE") != 0) ? DATADIR : ".";
		AG_SetString(cfg, "load-path", s);
		AG_SetString(cfg, "save-path", s);
		AG_SetString(cfg, "tmp-path", "tmp");
	}

	if ((flags & AG_CREATE_DATADIR) &&
	    AG_CreateDataDir() == -1) {
		return (-1);
	}
	return (0);
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
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
Save(void *obj, AG_DataSource *ds)
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
 * Copy the full pathname of a data file to a fixed-size buffer.
 * Return 0 if the file exists, or -1 if an error occured.
 */
int
AG_ConfigFile(const char *path_key, const char *name, const char *ext,
    char *path, size_t path_len)
{
	char file[AG_PATHNAME_MAX];
	char *dir, *pathp = path;
	int rv;

	AG_GetString(agConfig, path_key, path, path_len);

	for (dir = Strsep(&pathp, AG_PATHSEPMULTI);
	     dir != NULL;
	     dir = Strsep(&pathp, AG_PATHSEPMULTI)) {
		Strlcpy(file, dir, sizeof(file));

		if (name[0] != AG_PATHSEPCHAR) {
			Strlcat(file, AG_PATHSEP, sizeof(file));
		}
		Strlcat(file, name, sizeof(file));
		if (ext != NULL) {
			Strlcat(file, ".", sizeof(file));
			Strlcat(file, ext, sizeof(file));
		}
		if ((rv = AG_FileExists(file)) == 1) {
			if (Strlcpy(path, file, path_len) >= path_len) {
				AG_SetError(_("The search path is too big."));
				return (-1);
			}
			return (0);
		} else if (rv == -1) {
			AG_SetError("%s: %s", file, AG_GetError());
			return (-1);
		}
	}
	AG_GetString(agConfig, path_key, path, path_len);
	AG_SetError(_("Cannot find %s.%s (in <%s>:%s)."), name,
	    (ext != NULL) ? ext : "", path_key, path);
	return (-1);
}

/* Return a pointer to the global agConfig object. */
AG_Config *
AG_ConfigObject(void)
{
	return (agConfig);
}

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
