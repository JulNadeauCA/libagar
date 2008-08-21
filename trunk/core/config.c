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
#include <config/ttfdir.h>
#include <config/have_getpwuid.h>
#include <config/have_getuid.h>
#include <config/have_freetype.h>

#include <core/core.h>
#include <core/config.h>
#include <core/rcs.h>
#include <core/dir.h>
#include <core/file.h>

#include <gui/view.h>
#include <gui/text.h>
#include <gui/window.h>

#include <string.h>
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif

int agKbdDelay = 250;			/* Key repeat delay */
int agKbdRepeat = 35;			/* Key repeat interval */
int agMouseDblclickDelay = 250;		/* Mouse double-click delay */
int agMouseSpinDelay = 250;		/* Spinbutton repeat delay */
int agMouseSpinIval = 50;		/* Spinbutton repeat interval */

int agTextComposition = 1;		/* Built-in input composition */
int agTextBidi = 0;			/* Bidirectionnal text display */
int agTextAntialiasing = 1;		/* Use font antialiasing */
int agTextCache = 0;			/* Dynamic text cache */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 250;		/* Cursor blink rate (ms) */
int agTextSymbols = 1;			/* Process special symbols in text */
int agPageIncrement = 4;		/* Pgup/Pgdn scrolling increment */

int agIdleThresh = 20;			/* Idling threshold */
int agScreenshotQuality = 100;		/* JPEG quality in % */
int agWindowAnySize = 0;		/* Allow any window size */
int agMsgDelay = 500;			/* Display duration of infoboxes (ms) */

void
AG_ConfigInit(AG_Config *cfg)
{
	char udatadir[AG_PATHNAME_MAX];
	char tmpdir[AG_PATHNAME_MAX];
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	struct passwd *pwd = getpwuid(getuid());
#endif

	AG_ObjectInit(cfg, &agConfigClass);
	AG_ObjectSetName(cfg, "config");
	OBJECT(cfg)->flags |= AG_OBJECT_RELOAD_PROPS|AG_OBJECT_RESIDENT;
	OBJECT(cfg)->save_pfx = NULL;

	AG_SetBool(cfg, "initial-run", 1);
	AG_SetBool(cfg, "view.full-screen", 0);
	AG_SetBool(cfg, "view.async-blits", 0);
	AG_SetBool(cfg, "view.opengl", 0);
	AG_SetUint16(cfg, "view.w", 800);
	AG_SetUint16(cfg, "view.h", 600);
	AG_SetUint16(cfg, "view.min-w", 16);
	AG_SetUint16(cfg, "view.min-h", 16);
	AG_SetUint8(cfg, "view.depth", 32);
	AG_SetUint(cfg, "view.nominal-fps", 40);
	AG_SetBool(cfg, "input.joysticks", 1);
	AG_SetBool(cfg, "input.unicode", 1);
	AG_SetBool(cfg, "input.composition", 1);

	/* Set the save directory path and create it as needed. */
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	Strlcpy(udatadir, pwd->pw_dir, sizeof(udatadir));
	Strlcat(udatadir, AG_PATHSEP, sizeof(udatadir));
	Strlcat(udatadir, ".", sizeof(udatadir));
	Strlcat(udatadir, agProgName, sizeof(udatadir));
#else
	udatadir[0] = '.';
	Strlcpy(&udatadir[1], agProgName, sizeof(udatadir)-1);
#endif
	Strlcpy(tmpdir, udatadir, sizeof(tmpdir));
	Strlcat(tmpdir, "/tmp", sizeof(tmpdir));
	
	if (AG_FileExists(udatadir) == 0 && AG_MkDir(udatadir) != 0)
		AG_FatalError("%s: %s", udatadir, AG_GetError());
	if (AG_FileExists(tmpdir) == 0 && AG_MkDir(tmpdir) != 0)
		AG_FatalError("%s: %s", tmpdir, AG_GetError());
	
	AG_SetString(cfg, "save-path", "%s", udatadir);
	AG_SetString(cfg, "tmp-path", "%s", tmpdir);

#if defined(_WIN32)
	AG_SetString(cfg, "den-path", ".");
	AG_SetString(cfg, "load-path", "%s:.", udatadir);
#else
	AG_SetString(cfg, "den-path", "%s", SHAREDIR);
	AG_SetString(cfg, "load-path", "%s:%s", udatadir, SHAREDIR);
#endif
	
#if defined(__APPLE__)
# if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	AG_SetString(cfg, "font-path", "%s/fonts:%s:%s/Library/Fonts:"
	                                "/Library/Fonts:"
					"/System/Library/Fonts",
					udatadir, TTFDIR, pwd->pw_dir);
# else
	AG_SetString(cfg, "font-path", "%s/fonts:%s:"
	                                "/Library/Fonts:"
					"/System/Library/Fonts",
					udatadir, TTFDIR);
# endif
#elif defined(_WIN32)
	AG_SetString(cfg, "font-path", "fonts:.");
#else
	AG_SetString(cfg, "font-path", "%s/fonts:%s", udatadir, TTFDIR);
#endif

#ifdef HAVE_FREETYPE
	AG_SetBool(cfg, "font.freetype", 1);
#else
	AG_SetBool(cfg, "font.freetype", 0);
#endif
	AG_SetString(cfg, "font.face", "?");
	AG_SetInt(cfg, "font.size", -1);
	AG_SetUint(cfg, "font.flags", 0);
}

static int
Load(void *p, AG_DataSource *ds, const AG_Version *ver)
{
#ifdef DEBUG
	agDebugLvl = AG_ReadUint8(ds);
#else
	(void)AG_ReadUint8(ds);
#endif
	if (ver->minor < 2) { (void)AG_ReadUint8(ds); } /* agServerMode */
	agIdleThresh = (int)AG_ReadUint8(ds);
	if (ver->minor >= 3) { agWindowAnySize = (int)AG_ReadUint8(ds); }
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
	if (ver->minor >= 1) { agTextAntialiasing = AG_ReadUint8(ds); }

	agRcsMode = (int)AG_ReadUint8(ds);
	AG_CopyString(agRcsHostname, ds, sizeof(agRcsHostname));
	agRcsPort = (Uint)AG_ReadUint16(ds);
	AG_CopyString(agRcsUsername, ds, sizeof(agRcsUsername));
	AG_CopyString(agRcsPassword, ds, sizeof(agRcsPassword));
	return (0);
}

static int
Save(void *p, AG_DataSource *ds)
{
#ifdef DEBUG
	AG_WriteUint8(ds, (Uint8)agDebugLvl);
#else
	AG_WriteUint8(ds, 0);
#endif
	AG_WriteUint8(ds, (Uint8)agIdleThresh);
	AG_WriteUint8(ds, (Uint8)agWindowAnySize);
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
	AG_WriteUint8(ds, (Uint8)agTextAntialiasing);

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

	AG_GetStringCopy(agConfig, path_key, path, path_len);

	for (dir = Strsep(&pathp, ":");
	     dir != NULL;
	     dir = Strsep(&pathp, ":")) {
		Strlcpy(file, dir, sizeof(file));

		if (name[0] != AG_PATHSEPC) {
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
	AG_GetStringCopy(agConfig, path_key, path, path_len);
	AG_SetError(_("Cannot find %s.%s (in <%s>:%s)."), name,
	    (ext != NULL) ? ext : "", path_key, path);
	return (-1);
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
