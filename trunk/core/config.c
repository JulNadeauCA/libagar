/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Global configuration object. Used for Agar-related settings, but
 * applications are free to insert additional properties into the
 * object.
 */

#include <config/sharedir.h>
#include <config/have_getpwuid.h>
#include <config/have_getuid.h>
#include <config/have_freetype.h>

#include <core/core.h>
#include <core/config.h>
#include <core/rcs.h>

#ifdef _XBOX
#include <core/xbox.h>
#endif

#include <string.h>
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif

/* XXX XXX move to agar-gui */
int agKbdDelay = 250;			/* Key repeat delay */
int agKbdRepeat = 35;			/* Key repeat interval */
int agMouseDblclickDelay = 250;		/* Mouse double-click delay */
int agMouseSpinDelay = 250;		/* Spinbutton repeat delay */
int agMouseSpinIval = 50;		/* Spinbutton repeat interval */

int agTextComposition = 1;		/* Built-in input composition */
int agTextBidi = 0;			/* Bidirectionnal text display */
int agTextCache = 0;			/* Dynamic text caching */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 500;		/* Cursor blink rate (ms) */
int agTextSymbols = 1;			/* Process special symbols in text */
int agPageIncrement = 4;		/* Pgup/Pgdn scrolling increment */

int agIdleThresh = 20;			/* Idling threshold */
int agScreenshotQuality = 100;		/* JPEG quality in % */
int agMsgDelay = 500;			/* Display duration of infoboxes (ms) */

int
AG_CreateDataDir(void)
{
	char dataDir[AG_PATHNAME_MAX];
	char tmpDir[AG_PATHNAME_MAX];

	AG_CopyCfgString("save-path", dataDir, sizeof(dataDir));
	AG_CopyCfgString("tmp-path", tmpDir, sizeof(tmpDir));

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
	char udatadir[AG_PATHNAME_MAX];
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	struct passwd *pwd = getpwuid(getuid());
#endif

	AG_ObjectInit(cfg, &agConfigClass);
	AG_ObjectSetName(cfg, "config");
	OBJECT(cfg)->flags |= AG_OBJECT_RESIDENT;
	OBJECT(cfg)->save_pfx = NULL;

	AG_SetInt(cfg, "initial-run", 1);
	AG_SetInt(cfg, "no-confirm-quit", 0);

	/* XXX XXX move to agar-gui */
	AG_SetInt(cfg, "view.full-screen", 0);

	/* Set the save directory path and create it as needed. */
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	Strlcpy(udatadir, pwd->pw_dir, sizeof(udatadir));
	Strlcat(udatadir, AG_PATHSEP, sizeof(udatadir));
	Strlcat(udatadir, ".", sizeof(udatadir));
	Strlcat(udatadir, agProgName, sizeof(udatadir));
	AG_SetString(cfg, "home", pwd->pw_dir);
#elif defined(_XBOX)
	/* If the persistent data drive is mounted use it */
	if(AG_XBOX_DriveIsMounted('T')) {
		Strlcpy(udatadir, "T:\\", sizeof(udatadir));
	} else {
		Strlcpy(udatadir, "D:\\.", sizeof(udatadir));
	}
	Strlcat(udatadir, agProgName, sizeof(udatadir)-1);
	AG_SetString(cfg, "home", "D:\\");
#else
	udatadir[0] = '.';
	Strlcpy(&udatadir[1], agProgName, sizeof(udatadir)-1);
	AG_SetString(cfg, "home", ".");
#endif
	AG_SetString(cfg, "save-path", udatadir);
	AG_PrtString(cfg, "tmp-path", "%s%stmp", udatadir, AG_PATHSEP);

#if defined(_XBOX)
	if(AG_XBOX_DriveIsMounted('T')) {
		AG_SetString(cfg, "den-path", "T:\\");
	} else {
		AG_SetString(cfg, "den-path", "D:\\");
	}
	AG_PrtString(cfg, "load-path", "%s;D:\\", udatadir);
#elif defined(_WIN32)
	AG_SetString(cfg, "den-path", ".");
	AG_PrtString(cfg, "load-path", "%s:.", udatadir);
#else
	AG_SetString(cfg, "den-path", SHAREDIR);
	AG_PrtString(cfg, "load-path", "%s:%s", udatadir, SHAREDIR);
#endif /* _WIN32 */
	
	if (flags & AG_CREATE_DATADIR) {
		if (AG_CreateDataDir() == -1)
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
	if (ver->minor < 2) { (void)AG_ReadUint8(ds); } /* agServerMode */
	agIdleThresh = (int)AG_ReadUint8(ds);
	if (ver->minor >= 3) { (void)AG_ReadUint8(ds); } /* agWindowAnySize */
	if (ver->minor >= 4) { agMsgDelay = (int)AG_ReadUint32(ds); }
	agTextComposition = AG_ReadUint8(ds);
	agTextBidi = AG_ReadUint8(ds);
	(void)AG_ReadUint8(ds);				/* agKbdUnicode */
	agKbdDelay = (int)AG_ReadUint32(ds);
	agKbdRepeat = (int)AG_ReadUint32(ds);
	agMouseDblclickDelay = (int)AG_ReadUint32(ds);
	agMouseSpinDelay = (int)AG_ReadUint16(ds);
	agMouseSpinIval = (int)AG_ReadUint16(ds);
	agScreenshotQuality = (int)AG_ReadUint8(ds);
	agTextTabWidth = (int)AG_ReadUint16(ds);
	if (ver->minor >= 1) { (void)AG_ReadUint8(ds); } /* agTextAntialiasing */

	agRcsMode = (int)AG_ReadUint8(ds);
	AG_CopyString(agRcsHostname, ds, sizeof(agRcsHostname));
	agRcsPort = (Uint)AG_ReadUint16(ds);
	AG_CopyString(agRcsUsername, ds, sizeof(agRcsUsername));
	AG_CopyString(agRcsPassword, ds, sizeof(agRcsPassword));
	return (0);
}

static int
Save(void *obj, AG_DataSource *ds)
{
	AG_Config *cfg = obj;

	AG_SetInt(cfg, "initial-run", 0);

#ifdef AG_DEBUG
	AG_WriteUint8(ds, (Uint8)agDebugLvl);
#else
	AG_WriteUint8(ds, 0);
#endif
	AG_WriteUint8(ds, (Uint8)agIdleThresh);
	AG_WriteUint8(ds, 0);				/* agWindowAnySize */
	AG_WriteUint32(ds, (Uint32)agMsgDelay);
	AG_WriteUint8(ds, (Uint8)agTextComposition);
	AG_WriteUint8(ds, (Uint8)agTextBidi);
	AG_WriteUint8(ds, 0);				/* agKbdUnicode */
	AG_WriteUint32(ds, (Uint32)agKbdDelay);
	AG_WriteUint32(ds, (Uint32)agKbdRepeat);
	AG_WriteUint32(ds, (Uint32)agMouseDblclickDelay);
	AG_WriteUint16(ds, (Uint16)agMouseSpinDelay);
	AG_WriteUint16(ds, (Uint16)agMouseSpinIval);
	AG_WriteUint8(ds, (Uint8)agScreenshotQuality);
	AG_WriteUint16(ds, (Uint16)agTextTabWidth);
	AG_WriteUint8(ds, 0);				/* agTextAntialiasing */

	AG_WriteUint8(ds, (Uint8)agRcsMode);
	AG_WriteString(ds, agRcsHostname);
	AG_WriteUint16(ds, (Uint16)agRcsPort);
	AG_WriteString(ds, agRcsUsername);
	AG_WriteString(ds, agRcsPassword);
	return (0);
}

/* Copy the full pathname of a data file to a sized buffer. */
int
AG_ConfigFile(const char *path_key, const char *name, const char *ext,
    char *path, size_t path_len)
{
	char file[AG_PATHNAME_MAX];
	char *dir, *pathp = path;
	int rv;

	AG_GetString(agConfig, path_key, path, path_len);

	for (dir = Strsep(&pathp, ":");
	     dir != NULL;
	     dir = Strsep(&pathp, ":")) {
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

void
AG_SetCfgString(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	AG_SetString(agConfig, key, s);
	Free(s);
}

AG_ObjectClass agConfigClass = {
	"Agar(Config)",
	sizeof(AG_Config),
	{ 9, 4 },
	NULL,
	NULL,
	NULL,
	Load,
	Save,
	NULL
};
