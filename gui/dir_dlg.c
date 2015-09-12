/*
 * Copyright (c) 2010-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#ifdef __NetBSD__
#define _NETBSD_SOURCE
#endif

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/dir_dlg.h>
#include <agar/gui/hbox.h>
#include <agar/gui/numerical.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>
#include <agar/gui/icons.h>

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#ifdef _XBOX
# include <agar/core/xbox.h>
#elif _WIN32
# include <agar/core/win32.h>
#else
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
#endif

#include <agar/gui/file_dlg_common.h>

AG_DirDlg *
AG_DirDlgNew(void *parent, Uint flags)
{
	AG_DirDlg *dd;

	dd = Malloc(sizeof(AG_DirDlg));
	AG_ObjectInit(dd, &agDirDlgClass);
	dd->flags |= flags;
	if (flags & AG_DIRDLG_HFILL) { AG_ExpandHoriz(dd); }
	if (flags & AG_DIRDLG_VFILL) { AG_ExpandVert(dd); }
	if (flags & AG_DIRDLG_MULTI) { dd->tlDirs->flags |= AG_TLIST_MULTI; }
	
	if (flags & AG_DIRDLG_NOBUTTONS) {
		AG_ObjectDetach(dd->btnOk);
		AG_ObjectDetach(dd->btnCancel);
		dd->btnOk = NULL;
		dd->btnCancel = NULL;
	}

	AG_ObjectAttach(parent, dd);
	return (dd);
}

AG_DirDlg *
AG_DirDlgNewMRU(void *parent, const char *mruKey, Uint flags)
{
	char savePath[AG_PATHNAME_MAX];
	AG_DirDlg *dd;

	dd = AG_DirDlgNew(parent, flags);
	AG_GetString(AG_ConfigObject(), "save-path", savePath, sizeof(savePath));
	AG_DirDlgSetDirectoryMRU(dd, mruKey, savePath);
	return (dd);
}

/* Update the directory listing */
static void
RefreshListing(AG_DirDlg *dd)
{
	AG_TlistItem *it;
	AG_FileInfo info;
	AG_Dir *dir;
	char **dirs;
	size_t i, ndirs = 0;

	if ((dir = AG_OpenDir(dd->cwd)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", dd->cwd, AG_GetError());
		return;
	}
	
	dirs = Malloc(sizeof(char *));
	
	AG_ObjectLock(dd->tlDirs);

	for (i = 0; i < dir->nents; i++) {
		char path[AG_FILENAME_MAX];
		
		Strlcpy(path, dd->cwd, sizeof(path));
		if(path[strlen(path) - 1] != AG_PATHSEPCHAR) {
			Strlcat(path, AG_PATHSEP, sizeof(path));
		}
		Strlcat(path, dir->ents[i], sizeof(path));

		if (AG_PathIsFilesystemRoot(dd->cwd) &&
		    strcmp(dir->ents[i], "..")==0) {
			continue;
		}
		if (AG_GetFileInfo(path, &info) == -1) {
			continue;
		}
		if (info.type != AG_FILE_DIRECTORY) {
			continue;
		}
		/* XXX TODO: check for symlinks to directories */
		dirs = Realloc(dirs, (ndirs + 1) * sizeof(char *));
		dirs[ndirs++] = Strdup(dir->ents[i]);
	}
	qsort(dirs, ndirs, sizeof(char *), AG_FilenameCompare);

	AG_TlistClear(dd->tlDirs);
	for (i = 0; i < ndirs; i++) {
		it = AG_TlistAddS(dd->tlDirs, agIconDirectory.s, dirs[i]);
		it->cat = "dir";
		it->p1 = it;
		Free(dirs[i]);
	}
	Free(dirs);
	AG_TlistRestore(dd->tlDirs);
	
	AG_ObjectUnlock(dd->tlDirs);
	AG_CloseDir(dir);
}

/* Update the shortcuts. */
static void
RefreshShortcuts(AG_DirDlg *dd, int init)
{
	AG_Tlist *tl = dd->comLoc->list;

	AG_TlistClear(tl);
#ifdef _WIN32
	{
		char path[4];
		int drive;
#ifdef _XBOX
		DWORD d = AG_XBOX_GetLogicalDrives();
#else
		DWORD d = GetLogicalDrives();
#endif

		/* Add the Windows drives */
		for (drive = 0; drive < 26; drive++) {
			AG_TlistItem *ti;

			if (!(d & (1 << drive))) {
				continue;
			}
			path[0] = 'A'+drive;
			path[1] = ':';
			path[2] = '\\';
			path[3] = '\0';
			ti = AG_TlistAddS(tl, agIconDirectory.s, path);

			if (init &&
			    toupper(dd->cwd[0]) == path[0] &&
			    dd->cwd[1] == ':') {
				AG_ComboSelect(dd->comLoc, ti);
			}
#if 0
			/* TODO icons, etc */
			switch (GetDriveType(path)) {
			case DRIVE_UNKNOWN:
			case DRIVE_NO_ROOT_DIR:
			case DRIVE_REMOVABLE:
			case DRIVE_FIXED:
			case DRIVE_REMOTE:
			case DRIVE_CDROM:
			case DRIVE_RAMDISK:
				break;
			}
#endif
		}
	}
#else /* !_WIN32 */
	{
		char path[AG_PATHNAME_MAX], *pPath = &path[0], *p;
		AG_User *sysUser;
	
		/* Add the filesystem root, home and cwd. */
		AG_TlistAddS(tl, agIconDirectory.s, "/");
		if ((sysUser = AG_GetRealUser()) != NULL) {
			AG_TlistAddS(tl, agIconDirectory.s, sysUser->home);
			AG_UserFree(sysUser);
		}
		if (AG_GetCWD(path, sizeof(path)) == 0)
			AG_TlistAddS(tl, agIconDirectory.s, path);
		
		/* Add the Agar save-path or load-path */
		AG_GetString(AG_ConfigObject(),
		    (dd->flags & AG_DIRDLG_SAVE) ? "save-path" : "load-path",
		    path, sizeof(path));
		while ((p = AG_Strsep(&pPath, AG_PATHSEPMULTI)) != NULL) {
			if (!AG_FileExists(p)) {
				continue;
			}
			AG_TlistAddS(tl, agIconDirectory.s, path);
		}
		AG_ComboSelectText(dd->comLoc, dd->cwd);
	}
	AG_TlistUniq(tl);
#endif /* _WIN32 */

	AG_TlistRestore(tl);
}

static void
DirSelected(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_DirDlg *dd = AG_PTR(1);
	AG_TlistItem *ti;

	AG_ObjectLock(dd);
	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (AG_DirDlgSetDirectoryS(dd, ti->text) == -1) {
			/* AG_TextMsgFromError() */
		} else {
			AG_PostEvent(NULL, dd, "dir-selected", "%s", dd->cwd);
			RefreshListing(dd);
		}
	}
	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(dd);
}

static void
LocSelected(AG_Event *event)
{
	AG_DirDlg *dd = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	if (ti == NULL) {
		return;
	}
	if (AG_DirDlgSetDirectoryS(dd, ti->text) == -1) {
		/* AG_TextMsgFromError() */
	} else {
		AG_PostEvent(NULL, dd, "dir-selected", "%s", dd->cwd);
		RefreshListing(dd);
	}
}

int
AG_DirDlgCheckReadAccess(AG_DirDlg *dd)
{
	AG_FileInfo info;

	AG_ObjectLock(dd);
	if (AG_GetFileInfo(dd->cwd, &info) == -1) {
		goto fail;
	}
	if ((info.perms & AG_FILE_READABLE) == 0) {
		AG_SetError(_("%s: Read permission denied"), dd->cwd);
		goto fail;
	}
	AG_ObjectUnlock(dd);
	return (0);
fail:
	AG_ObjectUnlock(dd);
	return (-1);
}

int
AG_DirDlgCheckWriteAccess(AG_DirDlg *dd)
{
	AG_FileInfo info;
	
	AG_ObjectLock(dd);
	if (AG_GetFileInfo(dd->cwd, &info) == -1) {
		goto fail;
	}
	if ((info.perms & AG_FILE_WRITEABLE) == 0) {
		AG_SetError(_("%s: Write permission denied"), dd->cwd);
		goto fail;
	}
	AG_ObjectUnlock(dd);
	return (0);
fail:
	AG_ObjectUnlock(dd);
	return (-1);
}

static void
ChooseDir(AG_DirDlg *dd, AG_Window *pwin)
{
	AG_ObjectLock(dd);
	AG_PostEvent(NULL, dd, "dir-chosen", "%s", dd->cwd);
	if (dd->flags & AG_DIRDLG_CLOSEWIN) {
/*		AG_PostEvent(NULL, pwin, "window-close", NULL); */
		AG_ObjectDetach(pwin);
	}
	AG_ObjectUnlock(dd);
}

static void
CheckAccessAndChoose(AG_DirDlg *dd)
{
	AG_Window *pwin = AG_ParentWindow(dd);
	char *s;

	for (s = &dd->cwd[0]; *s != '\0'; s++) {
		if (!isspace((int) *s))
			break;
	}
	if (*s == '\0')
		return;

	if (dd->flags & AG_DIRDLG_LOAD) {
		if (AG_DirDlgCheckReadAccess(dd) == -1) {
			AG_TextMsgFromError();
			return;
		}
	} else if (dd->flags & AG_DIRDLG_SAVE) {
		AG_FileInfo info;

		if (AG_GetFileInfo(dd->cwd, &info) == 0) {
			if (!(info.perms & AG_FILE_WRITEABLE)) {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: Directory is non-writeable"),
				    dd->cwd);
			}
			return;
		}
	}
	ChooseDir(dd, pwin);
}

static void
PressedOK(AG_Event *event)
{
	AG_DirDlg *dd = AG_PTR(1);

	AG_ObjectLock(dd);
	if (dd->okAction != NULL) {
		AG_PostEventByPtr(NULL, dd, dd->okAction, "%s", dd->cwd);
	} else {
		CheckAccessAndChoose(dd);
	}
	AG_ObjectUnlock(dd);
}

static void
SetDirpath(AG_DirDlg *dd, const char *dir)
{
	if (dir[0] == AG_PATHSEPCHAR) {
		Strlcpy(dd->cwd, dir, sizeof(dd->cwd));
	} else {
		if (!AG_PathIsFilesystemRoot(dd->cwd) &&
		    (dd->cwd[0] != '\0' &&
		     dd->cwd[strlen(dd->cwd)-1] != AG_PATHSEPCHAR)) {
			Strlcat(dd->cwd, AG_PATHSEP, sizeof(dd->cwd));
		}
		Strlcat(dd->cwd, dir, sizeof(dd->cwd));
	}
}

static void
TextboxChanged(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Textbox *tb = AG_SELF();
	AG_DirDlg *dd = AG_PTR(1);

	AG_ObjectLock(dd);
	AG_TextboxCopyString(tb, path, sizeof(path));
	SetDirpath(dd, path);
	AG_ObjectUnlock(dd);
}

#ifdef HAVE_GLOB
static void
SelectGlobResult(AG_Event *event)
{
	char file[AG_PATHNAME_MAX];
	AG_Window *win = AG_PTR(1);
	AG_DirDlg *dd = AG_PTR(2);
	AG_TlistItem *ti = AG_PTR(3);
	AG_Textbox *tb = dd->tbInput;

	AG_ObjectLock(dd);
	Strlcpy(file, ti->text, sizeof(file));
	AG_TextboxSetString(tb, file);
	if (AG_DirDlgSetDirectoryS(dd, file) != 0) {
		AG_TextMsgFromError();
		goto out;
	}
	CheckAccessAndChoose(dd);
out:
	AG_ObjectUnlock(dd);
	AG_ObjectDetach(win);
}

static void
CloseGlobResults(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	AG_ObjectDetach(win);
}

static void
ExpandGlobResults(AG_DirDlg *dd, glob_t *gl, const char *pattern)
{
	AG_Window *winParent = WIDGET(dd)->window;
	AG_Window *win;
	Uint wView, hView;
	int x, y, w, h;
	int wMax = 0, hMax = 0;
	AG_Button *btn;
	AG_Tlist *tl;
	int i;
	
	win = AG_WindowNew(0);
	AG_WindowAttach(winParent, win);
	AG_WindowSetCaption(win, _("Matching \"%s\""), pattern);

	tl = AG_TlistNew(win, AG_TLIST_EXPAND);
	AG_SetEvent(tl, "tlist-selected", SelectGlobResult, "%p,%p", win, dd);
	btn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Dismiss"),
	    CloseGlobResults, "%p", win);
	AG_WidgetFocus(btn);

	for (i = 0; i < gl->gl_pathc; i++) {
		char *p = gl->gl_pathv[i];
		AG_FileInfo fi;
		AG_TlistItem *ti;

		if (AG_GetFileInfo(p, &fi) != 0) {
			continue;
		}
		if (fi.type == AG_FILE_DIRECTORY) {
			if (p[strlen(p)-1] != AG_PATHSEPCHAR) {
				ti = AG_TlistAdd(tl, agIconDirectory.s,
				    "%s%c", p, AG_PATHSEPCHAR);
			} else {
				ti = AG_TlistAddS(tl, agIconDirectory.s, p);
			}
		} else {
			ti = AG_TlistAddS(tl, agIconDoc.s, p);
		}
		ti->p1 = &gl->gl_pathv[i];
		AG_TextSize(p, &w, NULL);
		if (w > wMax) { wMax = w; }
		hMax++;
	}

	/* Compute geometry. */
	w = wMax + 100 + tl->item_h+2 +
	    agTextFontHeight; /* scrollbar size */
	h = hMax*tl->item_h + 32;
	x = WIDGET(dd->tbInput)->rView.x2 - w;
	y = WIDGET(dd->tbInput)->rView.y1;
	if (AGDRIVER_MULTIPLE(WIDGET(dd)->drv) &&
	    winParent != NULL) {
		x += WIDGET(winParent)->x;
		y += WIDGET(winParent)->y;
	}

	/* Limit to display area. */
	AG_GetDisplaySize(WIDGET(dd)->drv, &wView, &hView);
	if (x < 0) { x = 0; }
	if (y < 0) { y = 0; }
	if (x+w > wView) { w = wView - x; }
	if (y+h > hView) { h = hView - y; }
	if (w < 5 || h < 5) {
		AG_ObjectDetach(win);
		return;
	}

	AG_WindowSetGeometry(win, x, y, w, h);
	AG_WindowShow(win);
}

static int
GlobExpansion(AG_DirDlg *dd, char *path, size_t path_len)
{
	char *pathOrig;
	glob_t gl;
	int glFlags = GLOB_TILDE;

#if defined(GLOB_ONLYDIR)
	glFlags |= GLOB_ONLYDIR;
#endif
	if ((pathOrig = TryStrdup(path)) == NULL) {
		return (0);
	}
	if (glob(path, glFlags, NULL, &gl) != 0) {
		goto out;
	}
	if (gl.gl_pathc == 1) {
		Strlcpy(path, gl.gl_pathv[0], path_len);
	} else if (gl.gl_pathc > 1) {
		ExpandGlobResults(dd, &gl, pathOrig);
		free(pathOrig);
		return (1);
	}
	globfree(&gl);
out:
	free(pathOrig);
	return (0);
}
#endif /* HAVE_GLOB */

static void
TextboxReturn(AG_Event *event)
{
	char dir[AG_PATHNAME_MAX];
	AG_Textbox *tb = AG_SELF();
	AG_DirDlg *dd = AG_PTR(1);
	AG_FileInfo info;
	int endSep;
	
	AG_ObjectLock(dd);
	AG_TextboxCopyString(tb, dir, sizeof(dir));
#ifdef HAVE_GLOB
	if (GlobExpansion(dd, dir, sizeof(dir)))
		goto out;
#endif
	if (dir[0] == '\0') {
		goto out;
	}
	endSep = (dir[strlen(dir)-1]==AG_PATHSEPCHAR) ? 1 : 0;
	if (ProcessFilename(dir, sizeof(dir)) == -1) {
		goto out;
	}
	AG_TextboxSetString(tb, dir);

	if (endSep ||
	    (AG_GetFileInfo(dir,&info)==0 && info.type == AG_FILE_DIRECTORY)) {
		if (AG_DirDlgSetDirectoryS(dd, dir) == 0) {
			RefreshListing(dd);
		} else {
			/* AG_TextMsgFromError() */
			goto out;
		}
	} else {
		AG_TextError(_("%s: Not a directory"), dir);
	}
out:
	AG_ObjectUnlock(dd);
}

static void
PressedCancel(AG_Event *event)
{
	AG_DirDlg *dd = AG_PTR(1);
	AG_Window *pwin;

	AG_ObjectLock(dd);
	if (dd->cancelAction != NULL) {
		AG_PostEventByPtr(NULL, dd, dd->cancelAction, NULL);
	} else if (dd->flags & AG_DIRDLG_CLOSEWIN) {
		if ((pwin = AG_ParentWindow(dd)) != NULL) {
/*			AG_PostEvent(NULL, pwin, "window-close", NULL); */
			AG_ObjectDetach(pwin);
		}
	}
	AG_ObjectUnlock(dd);
}

static void
OnShow(AG_Event *event)
{
	AG_DirDlg *dd = AG_SELF();

	if (!(dd->flags & AG_DIRDLG_RESET_ONSHOW)) {
		return;
	}
	dd->flags &= ~(AG_DIRDLG_RESET_ONSHOW);

	AG_WidgetFocus(dd->tbInput);
	RefreshListing(dd);
	RefreshShortcuts(dd, 1);
}

/* Move to the specified directory (format string). */
int
AG_DirDlgSetDirectory(AG_DirDlg *dd, const char *fmt, ...)
{
	char path[AG_PATHNAME_MAX];
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);
	
	return AG_DirDlgSetDirectoryS(dd, path);
}

/* Move to the specified directory (C string). */
int
AG_DirDlgSetDirectoryS(AG_DirDlg *dd, const char *dir)
{
	AG_FileInfo info;
	char ncwd[AG_PATHNAME_MAX], *c;
	
	AG_ObjectLock(dd);

	if (dir[0] == '.' && dir[1] == '\0') {
		Strlcpy(ncwd, dd->cwd, sizeof(ncwd));
	} else if (dir[0] == '.' && dir[1] == '.' && dir[2] == '\0') {
		if (!AG_PathIsFilesystemRoot(dd->cwd)) {
			Strlcpy(ncwd, dd->cwd, sizeof(ncwd));
			if ((c = strrchr(ncwd, AG_PATHSEPCHAR)) != NULL) {
				*c = '\0';
			}
			if (c == &ncwd[0]) {
				ncwd[0] = AG_PATHSEPCHAR;
				ncwd[1] = '\0';
			}
#ifdef _XBOX
			if (AG_PathIsFilesystemRoot(ncwd) &&
			    ncwd[2] != AG_PATHSEPCHAR) {
				Strlcat(ncwd, AG_PATHSEP, sizeof(ncwd));
			}
#endif
		}
	} else if (!AG_PathIsAbsolute(dir)) {
		Strlcpy(ncwd, dd->cwd, sizeof(ncwd));
		if (!(ncwd[0] == AG_PATHSEPCHAR &&
		      ncwd[1] == '\0') &&
			  ncwd[strlen(ncwd) - 1] != AG_PATHSEPCHAR) {
			Strlcat(ncwd, AG_PATHSEP, sizeof(ncwd));
		}
		Strlcat(ncwd, dir, sizeof(ncwd));
	} else {
		Strlcpy(ncwd, dir, sizeof(ncwd));
	}

	if (AG_GetFileInfo(ncwd, &info) == -1) {
		goto fail;
	}
	if (info.type != AG_FILE_DIRECTORY) {
		AG_SetError(_("%s: Not a directory"), ncwd);
		goto fail;
	}
	if ((info.perms & (AG_FILE_READABLE|AG_FILE_EXECUTABLE)) == 0) {
		AG_SetError(_("%s: Permission denied"), ncwd);
		goto fail;
	}
	if (Strlcpy(dd->cwd, ncwd, sizeof(dd->cwd)) >= sizeof(dd->cwd)) {
		AG_SetError(_("Path is too long: `%s'"), ncwd);
		goto fail;
	}
	if (dd->dirMRU != NULL) {
		AG_SetString(AG_ConfigObject(), dd->dirMRU, dd->cwd);
		AG_ConfigSave();
	}

	AG_TextboxSetString(dd->tbInput, dd->cwd);
/*	AG_TextboxSetCursorPos(dd->tbInput, -1); */
	AG_TlistScrollToStart(dd->tlDirs);

	AG_ObjectUnlock(dd);
	return (0);
fail:
	AG_ObjectUnlock(dd);
	return (-1);
}

/* Set the current directory (fetch default from specified MRU). */
void
AG_DirDlgSetDirectoryMRU(AG_DirDlg *dd, const char *key, const char *dflt)
{
	AG_Config *cfg = AG_ConfigObject();
	char *s;

	AG_ObjectLock(dd);
	AG_ObjectLock(cfg);

	if (AG_Defined(cfg,key) && (s = AG_GetStringDup(cfg,key))) {
		AG_DirDlgSetDirectoryS(dd, s);
		Free(s);
	} else {
		AG_SetString(cfg, key, dflt);
		if (AG_ConfigSave() == -1) {
			Verbose("Saving MRU: %s\n", AG_GetError());
		}
		AG_DirDlgSetDirectoryS(dd, dflt);
	}
	dd->dirMRU = Strdup(key);

	AG_ObjectUnlock(cfg);
	AG_ObjectUnlock(dd);
}

static void
Init(void *obj)
{
	AG_DirDlg *dd = obj;

	dd->flags = AG_DIRDLG_RESET_ONSHOW;
	dd->dirMRU = NULL;
	(void)AG_GetCWD(dd->cwd, sizeof(dd->cwd));

	dd->comLoc = AG_ComboNewS(dd, AG_COMBO_HFILL, NULL);
	AG_ComboSizeHint(dd->comLoc, "XXXXXXXXXXXXXXXXXXXXXXXXXXXX", 5);
	AG_TlistSetCompareFn(dd->comLoc->list, AG_TlistCompareStrings);

	dd->tlDirs = AG_TlistNew(dd, AG_TLIST_EXPAND);

	dd->tbInput = AG_TextboxNewS(dd, AG_TEXTBOX_EXCL, _("Directory: "));
	AG_TlistSizeHint(dd->tlDirs, "XXXXXXXXXXXXXX", 8);

	dd->btnOk = AG_ButtonNewS(dd, 0, _("OK"));
	dd->btnCancel = AG_ButtonNewS(dd, 0, _("Cancel"));
	dd->okAction = NULL;
	dd->cancelAction = NULL;

	AG_AddEvent(dd, "widget-shown", OnShow, NULL);
	AG_SetEvent(dd->tlDirs, "tlist-dblclick", DirSelected, "%p", dd);
	AG_SetEvent(dd->comLoc, "combo-selected", LocSelected, "%p", dd);
	AG_SetEvent(dd->tbInput, "textbox-postchg", TextboxChanged, "%p", dd);
	AG_SetEvent(dd->tbInput, "textbox-return", TextboxReturn, "%p", dd);
	AG_SetEvent(dd->btnOk, "button-pushed", PressedOK, "%p", dd);
	AG_SetEvent(dd->btnCancel, "button-pushed", PressedCancel, "%p", dd);
}

/*
 * Register an event handler for the "OK" button. Overrides type-specific
 * handlers.
 */
void
AG_DirDlgOkAction(AG_DirDlg *dd, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(dd);
	if (dd->okAction != NULL) {
		AG_UnsetEvent(dd, dd->okAction->name);
	}
	dd->okAction = AG_SetEvent(dd, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(dd->okAction, fmt);
#ifdef AG_THREADS
	if (dd->flags & AG_DIRDLG_ASYNC)
		dd->okAction->flags |= AG_EVENT_ASYNC;
#endif
	AG_ObjectUnlock(dd);
}

/* Register an event handler for the "Cancel" button. */
void
AG_DirDlgCancelAction(AG_DirDlg *dd, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(dd);
	if (dd->cancelAction != NULL) {
		AG_UnsetEvent(dd, dd->cancelAction->name);
	}
	dd->cancelAction = AG_SetEvent(dd, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(dd->cancelAction, fmt);
	AG_ObjectUnlock(dd);
}

static void
Destroy(void *obj)
{
	AG_DirDlg *dd = obj;

	Free(dd->dirMRU);
}

static void
Draw(void *obj)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, obj, ag_widget)
		AG_WidgetDraw(chld);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_DirDlg *dd = obj;
	AG_SizeReq rChld, rOk, rCancel;

	r->w = 0;
	r->h = 4;
	AG_WidgetSizeReq(dd->tbInput, &rChld);
	r->h += rChld.h+2;

	if (!(dd->flags & AG_DIRDLG_NOBUTTONS)) {
		AG_WidgetSizeReq(dd->btnOk, &rOk);
		AG_WidgetSizeReq(dd->btnCancel, &rCancel);
		r->h += MAX(rOk.h,rCancel.h)+1;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_DirDlg *dd = obj;
	AG_SizeReq r, rLoc, rInput;
	AG_SizeAlloc aChld;
	int hBtn = 0, wBtn = a->w/2;

	if (!(dd->flags & AG_DIRDLG_NOBUTTONS)) {
		AG_WidgetSizeReq(dd->btnOk, &r);
		hBtn = MAX(hBtn, r.h);
		AG_WidgetSizeReq(dd->btnCancel, &r);
		hBtn = MAX(hBtn, r.h);
	}

	AG_WidgetSizeReq(dd->comLoc, &rLoc);
	AG_WidgetSizeReq(dd->tbInput, &rInput);

	/* Shortcuts */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w;
	aChld.h = rLoc.h;
	AG_WidgetSizeAlloc(dd->comLoc, &aChld);

	/* Listing */
	aChld.w = a->w;
	aChld.h = a->h - (hBtn + rInput.h + rLoc.h + 4);
	aChld.y += rLoc.h;
	AG_WidgetSizeAlloc(dd->tlDirs, &aChld);

	/* Input textbox */
	aChld.y += aChld.h+4;
	aChld.h = rInput.h;
	AG_WidgetSizeAlloc(dd->tbInput, &aChld);

	if (!(dd->flags & AG_DIRDLG_NOBUTTONS)) {
		/* Size buttons */
		aChld.y += aChld.h+2;
		aChld.w = wBtn;
		aChld.h = hBtn;
		AG_WidgetSizeAlloc(dd->btnOk, &aChld);
		aChld.x = wBtn;
		if (wBtn*2 < a->w) { aChld.w++; }
		aChld.h = hBtn;
		AG_WidgetSizeAlloc(dd->btnCancel, &aChld);
	}
	return (0);
}

AG_WidgetClass agDirDlgClass = {
	{
		"Agar(Widget:DirDlg)",
		sizeof(AG_DirDlg),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
