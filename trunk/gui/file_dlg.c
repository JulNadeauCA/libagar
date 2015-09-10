/*
 * Copyright (c) 2005-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/gui/file_dlg.h>
#include <agar/gui/hbox.h>
#include <agar/gui/numerical.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>
#include <agar/gui/icons.h>

#ifdef __NetBSD__
#define _NETBSD_SOURCE
#endif

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

AG_FileDlg *
AG_FileDlgNewMRU(void *parent, const char *mruKey, Uint flags)
{
	char savePath[AG_PATHNAME_MAX];
	AG_FileDlg *fd;

	fd = AG_FileDlgNew(parent, flags);
	AG_GetString(AG_ConfigObject(), "save-path", savePath, sizeof(savePath));
	AG_FileDlgSetDirectoryMRU(fd, mruKey, savePath);
	return (fd);
}

void
AG_FileDlgSetOptionContainer(AG_FileDlg *fd, void *ctr)
{
	AG_ObjectLock(fd);
	fd->optsCtr = ctr;
	AG_ObjectUnlock(fd);
}

/* Filter files by extension. */
static int
FilterByExtension(AG_FileDlg *fd, char *file)
{
	char *ext, *s;
	AG_FileType *ft;
	Uint i;

	if ((ext = strrchr(file, '.')) == NULL) {
		return (1);
	}
	TAILQ_FOREACH(ft, &fd->types, types) {
		for (i = 0; i < ft->nexts; i++) {
			if ((s = strrchr(ft->exts[i], '.')) != NULL &&
			    Strcasecmp(s, ext) == 0)
				break;
		}
		if (i < ft->nexts)
			break;
	}
	return (ft == NULL);
}

/* Update the file / directory listing */
static void
RefreshListing(AG_FileDlg *fd)
{
	AG_TlistItem *it;
	AG_FileInfo info;
	AG_Dir *dir;
	char **dirs, **files;
	size_t i, ndirs = 0, nfiles = 0;

	if ((dir = AG_OpenDir(fd->cwd)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", fd->cwd, AG_GetError());
		return;
	}
	
	dirs = Malloc(sizeof(char *));
	files = Malloc(sizeof(char *));
	
	AG_ObjectLock(fd->tlDirs);
	AG_ObjectLock(fd->tlFiles);

	for (i = 0; i < dir->nents; i++) {
		char *ent = dir->ents[i];
		char path[AG_FILENAME_MAX];
		
		Strlcpy(path, fd->cwd, sizeof(path));
		if(path[strlen(path) - 1] != AG_PATHSEPCHAR) {
			Strlcat(path, AG_PATHSEP, sizeof(path));
		}
		Strlcat(path, ent, sizeof(path));

		if (AG_PathIsFilesystemRoot(fd->cwd) &&
		    strcmp(ent, "..")==0) {
			continue;
		}
		if (AG_GetFileInfo(path, &info) == -1) {
			continue;
		}
		/* XXX TODO: check for symlinks to directories */
		if (info.type == AG_FILE_DIRECTORY) {
			dirs = Realloc(dirs, (ndirs + 1) * sizeof(char *));
			dirs[ndirs++] = Strdup(ent);
		} else {
			if (fd->flags & AG_FILEDLG_MASK_HIDDEN &&
			    ent[0] == '.') {
				continue;
			}
			if (fd->flags & AG_FILEDLG_MASK_EXT &&
			    FilterByExtension(fd, ent)) {
				continue;
			}
			files = Realloc(files, (nfiles + 1) * sizeof(char *));
			files[nfiles++] = Strdup(ent);
		}
	}
	qsort(dirs, ndirs, sizeof(char *), AG_FilenameCompare);
	qsort(files, nfiles, sizeof(char *), AG_FilenameCompare);

	AG_TlistClear(fd->tlDirs);
	AG_TlistClear(fd->tlFiles);
	for (i = 0; i < ndirs; i++) {
		it = AG_TlistAddS(fd->tlDirs, agIconDirectory.s, dirs[i]);
		it->cat = "dir";
		it->p1 = it;
		Free(dirs[i]);
	}
	for (i = 0; i < nfiles; i++) {
		it = AG_TlistAddS(fd->tlFiles, agIconDoc.s, files[i]);
		it->cat = "file";
		it->p1 = it;
		Free(files[i]);
	}
	Free(dirs);
	Free(files);
	AG_TlistRestore(fd->tlDirs);
	AG_TlistRestore(fd->tlFiles);
	
	AG_ObjectUnlock(fd->tlFiles);
	AG_ObjectUnlock(fd->tlDirs);
	AG_CloseDir(dir);
}

/* Update the shortcuts. */
static void
RefreshShortcuts(AG_FileDlg *fd, int init)
{
	AG_Tlist *tl = fd->comLoc->list;

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
			    toupper(fd->cwd[0]) == path[0] &&
			    fd->cwd[1] == ':') {
				AG_ComboSelect(fd->comLoc, ti);
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
		    (fd->flags & AG_FILEDLG_SAVE) ? "save-path" : "load-path",
		    path, sizeof(path));
		while ((p = AG_Strsep(&pPath, AG_PATHSEPMULTI)) != NULL) {
			if (!AG_FileExists(p)) {
				continue;
			}
			AG_TlistAddS(tl, agIconDirectory.s, path);
		}
		AG_ComboSelectText(fd->comLoc, fd->cwd);
	}
	AG_TlistUniq(tl);
#endif /* _WIN32 */

	AG_TlistRestore(tl);
}

void
AG_FileDlgRefresh(AG_FileDlg *fd)
{
	RefreshListing(fd);
	RefreshShortcuts(fd, 0);
}

static void
DirSelected(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *ti;

	AG_ObjectLock(fd);
	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (AG_FileDlgSetDirectoryS(fd, ti->text) == -1) {
			/* AG_TextMsgFromError() */
		} else {
			AG_PostEvent(NULL, fd, "dir-selected", "%s", fd->cwd);
			RefreshListing(fd);
		}
	}
	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(fd);
}

static void
LocSelected(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	if (ti == NULL) {
		return;
	}
	if (AG_FileDlgSetDirectoryS(fd, ti->text) == -1) {
		/* AG_TextMsgFromError() */
	} else {
		AG_PostEvent(NULL, fd, "dir-selected", "%s", fd->cwd);
		RefreshListing(fd);
	}
}

static void
ChooseFile(AG_FileDlg *fd, AG_Window *pwin)
{
	AG_TlistItem *it;
	AG_FileType *ft = NULL;
	char *ext;
	int i;

	AG_ObjectLock(fd);

	if (fd->comTypes != NULL &&
	    (it = AG_TlistSelectedItem(fd->comTypes->list)) != NULL) {
		ft = it->p1;
	} else if ((ext = strrchr(fd->cfile, '.')) != NULL) {
		TAILQ_FOREACH(ft, &fd->types, types) {
			for (i = 0; i < ft->nexts; i++) {
				char *s;
				if ((s = strrchr(ft->exts[i], '.')) != NULL &&
				    Strcasecmp(s, ext) == 0)
					break;
			}
			if (i < ft->nexts)
				break;
		}
	}
	if (ft != NULL && ft->action != NULL) {
		AG_PostEventByPtr(NULL, fd, ft->action, "%s,%p", fd->cfile, ft);
	}
	AG_PostEvent(NULL, fd, "file-chosen", "%s,%p", fd->cfile, ft);

	if (fd->flags & AG_FILEDLG_CLOSEWIN) {
/*		AG_PostEvent(NULL, pwin, "window-close", NULL); */
		AG_ObjectDetach(pwin);
	}
	AG_ObjectUnlock(fd);
}

static void
ReplaceFileConfirm(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	AG_Window *qwin = AG_PTR(2);
	AG_Window *pwin = AG_PTR(3);

	ChooseFile(fd, pwin);
	AG_ObjectDetach(qwin);
}

static void
ReplaceFileDlg(AG_FileDlg *fd, AG_Window *pwin)
{
	AG_Window *win;
	AG_Button *btn;
	AG_HBox *hb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_LabelNew(win, 0, _("File %s exists. Overwrite?"), fd->cfile);
	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Yes"),
		    ReplaceFileConfirm, "%p,%p,%p", fd, win, pwin);
		btn = AG_ButtonNewFn(hb, 0, _("Cancel"), AGWINDETACH(win));
		AG_WidgetFocus(btn);
	}
//	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

int
AG_FileDlgCheckReadAccess(AG_FileDlg *fd)
{
	AG_FileInfo info;

	AG_ObjectLock(fd);
	if (AG_GetFileInfo(fd->cfile, &info) == -1) {
		goto fail;
	}
	if ((info.perms & AG_FILE_READABLE) == 0) {
		AG_SetError(_("%s: Read permission denied"), fd->cfile);
		goto fail;
	}
	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

int
AG_FileDlgCheckWriteAccess(AG_FileDlg *fd)
{
	AG_FileInfo info;
	
	AG_ObjectLock(fd);
	if (AG_GetFileInfo(fd->cfile, &info) == -1) {
		goto fail;
	}
	if ((info.perms & AG_FILE_WRITEABLE) == 0) {
		AG_SetError(_("%s: Write permission denied"), fd->cfile);
		goto fail;
	}
	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

static void
CheckAccessAndChoose(AG_FileDlg *fd)
{
	AG_Window *pwin = AG_ParentWindow(fd);
	char *s;

	for (s = &fd->cfile[0]; *s != '\0'; s++) {
		if (!isspace((int) *s))
			break;
	}
	if (*s == '\0')
		return;

	if (fd->flags & AG_FILEDLG_LOAD) {
		if (AG_FileDlgCheckReadAccess(fd) == -1) {
			AG_TextMsgFromError();
			return;
		}
	} else if (fd->flags & AG_FILEDLG_SAVE) {
		AG_FileInfo info;

		/*
		 * Display a "replace file?" dialog if this file
		 * already exists.
		 */
		if (AG_GetFileInfo(fd->cfile, &info) == 0) {
			if (info.perms & AG_FILE_WRITEABLE) {
				ReplaceFileDlg(fd, pwin);
			} else {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: File exists and is non-writeable"),
				    fd->cfile);
			}
			return;
		}
	}
	ChooseFile(fd, pwin);
}

static void
FileSelected(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *ti;
	char *ext;

	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_FileDlgSetFilenameS(fd, ti->text);
		AG_PostEvent(NULL, fd, "file-selected", "%s", fd->cfile);
	}
	AG_ObjectUnlock(tl);

	if (fd->comTypes != NULL && (ext = strrchr(fd->cfile, '.')) != NULL) {
		AG_ObjectLock(fd->comTypes->list);
		TAILQ_FOREACH(ti, &fd->comTypes->list->items, items) {
			AG_FileType *ft = ti->p1;
			char *ftext;
			Uint i;

			for (i = 0; i < ft->nexts; i++) {
				if ((ftext = strrchr(ft->exts[i], '.'))
				    == NULL) {
					continue;
				}
				if (Strcasecmp(ftext, ext) == 0)
					break;
			}
			if (i < ft->nexts) {
				AG_ComboSelect(fd->comTypes, ti);
				AG_PostEvent(NULL, fd->comTypes,
				    "combo-selected", "%p", ti);
				break;
			}
		}
		AG_ObjectUnlock(fd->comTypes->list);
	}
}

static void
FileDblClicked(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *itFile;

	AG_ObjectLock(fd);
	AG_ObjectLock(tl);
	if ((itFile = AG_TlistSelectedItem(tl)) != NULL) {
		AG_FileDlgSetFilenameS(fd, itFile->text);

		if (fd->okAction != NULL) {
			AG_PostEventByPtr(NULL, fd, fd->okAction, "%s,%p",
			    fd->cfile,
			    fd->comTypes ? AG_TlistSelectedItemPtr(fd->comTypes->list) : NULL);
		} else {
			CheckAccessAndChoose(fd);
		}
	}
	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(fd);
}

static void
PressedOK(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);

	AG_ObjectLock(fd);
	if (fd->okAction != NULL) {
		AG_PostEventByPtr(NULL, fd, fd->okAction, "%s", fd->cfile);
	} else {
		CheckAccessAndChoose(fd);
	}
	AG_ObjectUnlock(fd);
}

static void
SetFilename(AG_FileDlg *fd, const char *file)
{
	if (file[0] == AG_PATHSEPCHAR) {
		Strlcpy(fd->cfile, file, sizeof(fd->cfile));
	} else {
		Strlcpy(fd->cfile, fd->cwd, sizeof(fd->cfile));
		if (!AG_PathIsFilesystemRoot(fd->cwd) &&
		    (fd->cfile[0] != '\0' &&
		     fd->cfile[strlen(fd->cfile)-1] != AG_PATHSEPCHAR)) {
			Strlcat(fd->cfile, AG_PATHSEP, sizeof(fd->cfile));
		}
		Strlcat(fd->cfile, file, sizeof(fd->cfile));
	}
}

static void
TextboxChanged(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Textbox *tb = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);

	AG_ObjectLock(fd);
	AG_TextboxCopyString(tb, path, sizeof(path));
	SetFilename(fd, path);
	AG_ObjectUnlock(fd);
}

#ifdef HAVE_GLOB
static void
SelectGlobResult(AG_Event *event)
{
	char file[AG_PATHNAME_MAX];
	AG_Window *win = AG_PTR(1);
	AG_FileDlg *fd = AG_PTR(2);
	AG_TlistItem *ti = AG_PTR(3);
	AG_Textbox *tb = fd->tbFile;
	AG_FileInfo info;
	int endSep;

	AG_ObjectLock(fd);
	Strlcpy(file, ti->text, sizeof(file));
	endSep = (file[strlen(file)-1]==AG_PATHSEPCHAR) ? 1 : 0;
	AG_TextboxSetString(tb, file);

	if (endSep ||
	    (AG_GetFileInfo(file,&info)==0 && info.type == AG_FILE_DIRECTORY)) {
		if (AG_FileDlgSetDirectoryS(fd, file) == 0) {
			RefreshListing(fd);
		} else {
			/* AG_TextMsgFromError() */
			goto out;
		}
	} else {
		AG_FileDlgSetFilenameS(fd, file);
		CheckAccessAndChoose(fd);
	}
out:
	AG_ObjectUnlock(fd);
	AG_ObjectDetach(win);
}

static void
CloseGlobResults(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	AG_ObjectDetach(win);
}

static void
ExpandGlobResults(AG_FileDlg *fd, glob_t *gl, const char *pattern)
{
	AG_Window *winParent = WIDGET(fd)->window;
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
	AG_SetEvent(tl, "tlist-selected", SelectGlobResult, "%p,%p", win, fd);
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
	x = WIDGET(fd->tbFile)->rView.x2 - w;
	y = WIDGET(fd->tbFile)->rView.y1;
	if (AGDRIVER_MULTIPLE(WIDGET(fd)->drv) &&
	    winParent != NULL) {
		x += WIDGET(winParent)->x;
		y += WIDGET(winParent)->y;
	}

	/* Limit to display area. */
	AG_GetDisplaySize(WIDGET(fd)->drv, &wView, &hView);
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
GlobExpansion(AG_FileDlg *fd, char *path, size_t path_len)
{
	char *pathOrig;
	glob_t gl;

	if ((pathOrig = TryStrdup(path)) == NULL) {
		return (0);
	}
	if (glob(path, GLOB_TILDE, NULL, &gl) != 0) {
		goto out;
	}
	if (gl.gl_pathc == 1) {
		Strlcpy(path, gl.gl_pathv[0], path_len);
	} else if (gl.gl_pathc > 1) {
		ExpandGlobResults(fd, &gl, pathOrig);
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
	char file[AG_PATHNAME_MAX];
	AG_Textbox *tb = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_FileInfo info;
	int endSep;
	
	AG_ObjectLock(fd);
	AG_TextboxCopyString(tb, file, sizeof(file));
#ifdef HAVE_GLOB
	if (GlobExpansion(fd, file, sizeof(file)))
		goto out;
#endif
	if (file[0] == '\0') {
		goto out;
	}
	endSep = (file[strlen(file)-1]==AG_PATHSEPCHAR) ? 1 : 0;
	if (ProcessFilename(file, sizeof(file)) == -1) {
		goto out;
	}
	AG_TextboxSetString(tb, file);

	if (endSep ||
	    (AG_GetFileInfo(file,&info)==0 && info.type == AG_FILE_DIRECTORY)) {
		if (AG_FileDlgSetDirectoryS(fd, file) == 0) {
			RefreshListing(fd);
		} else {
			/* AG_TextMsgFromError() */
			goto out;
		}
	} else {
		AG_FileDlgSetFilenameS(fd, file);
		CheckAccessAndChoose(fd);
	}
out:
	AG_ObjectUnlock(fd);
}

static void
PressedCancel(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	AG_Window *pwin;

	AG_ObjectLock(fd);
	if (fd->cancelAction != NULL) {
		AG_PostEventByPtr(NULL, fd, fd->cancelAction, NULL);
	} else if (fd->flags & AG_FILEDLG_CLOSEWIN) {
		if ((pwin = AG_ParentWindow(fd)) != NULL) {
/*			AG_PostEvent(NULL, pwin, "window-close", NULL); */
			AG_ObjectDetach(pwin);
		}
	}
	AG_ObjectUnlock(fd);
}

static void
SelectedType(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	AG_FileType *ft = (it != NULL) ? it->p1 : TAILQ_FIRST(&fd->types);
	AG_FileOption *fo;
	AG_Numerical *num;
	AG_Textbox *tbox;

	AG_ObjectLock(fd);
	if (fd->optsCtr == NULL) {
		AG_ObjectUnlock(fd);
		return;
	}

	AG_ObjectFreeChildren(fd->optsCtr);
	
	TAILQ_FOREACH(fo, &ft->opts, opts) {
		switch (fo->type) {
		case AG_FILEDLG_BOOL:
			AG_CheckboxNewInt(fd->optsCtr, 0, fo->descr,
			    &fo->data.i.val);
			break;
		case AG_FILEDLG_INT:
			num = AG_NumericalNewS(fd->optsCtr, AG_NUMERICAL_HFILL,
			    NULL, fo->descr);
			AG_BindInt(num, "value", &fo->data.i.val);
			AG_BindInt(num, "min", &fo->data.i.min);
			AG_BindInt(num, "max", &fo->data.i.max);
			break;
		case AG_FILEDLG_FLOAT:
			num = AG_NumericalNewS(fd->optsCtr, AG_NUMERICAL_HFILL,
			    fo->unit, fo->descr);
			AG_BindFloat(num, "value", &fo->data.flt.val);
			AG_BindFloat(num, "min", &fo->data.flt.min);
			AG_BindFloat(num, "max", &fo->data.flt.max);
			break;
		case AG_FILEDLG_DOUBLE:
			num = AG_NumericalNewS(fd->optsCtr, AG_NUMERICAL_HFILL,
			    fo->unit, fo->descr);
			AG_BindDouble(num, "value", &fo->data.dbl.val);
			AG_BindDouble(num, "min", &fo->data.dbl.min);
			AG_BindDouble(num, "max", &fo->data.dbl.max);
			break;
		case AG_FILEDLG_STRING:
			tbox = AG_TextboxNewS(fd->optsCtr,
			    AG_TEXTBOX_EXCL|AG_TEXTBOX_HFILL,
			    fo->descr);
			AG_TextboxBindUTF8(tbox, fo->data.s,
			    sizeof(fo->data.s));
			break;
		}
	}
	AG_SetStyle(fd->optsCtr, "font-size", "90%");
	AG_ObjectUnlock(fd);
	AG_WidgetUpdate(fd);
}

static void
OnShow(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	AG_TlistItem *it;
	int w, wMax = 0, nItems = 0;
	
	if (!(fd->flags & AG_FILEDLG_RESET_ONSHOW)) {
		return;
	}
	fd->flags &= ~(AG_FILEDLG_RESET_ONSHOW);

	AG_WidgetFocus(fd->tbFile);
	RefreshListing(fd);
	RefreshShortcuts(fd, 1);

	if (fd->comTypes != NULL) {
		AG_PostEvent(NULL, fd->comTypes, "combo-selected", "%p", NULL);
		AG_COMBO_FOREACH(it, fd->comTypes) {
			AG_TextSize(it->text, &w, NULL);
			if (w > wMax) { wMax = w; }
			nItems++;
		}
		AG_ComboSizeHintPixels(fd->comTypes, wMax, nItems);
	}
}

/* Move to the specified directory (format string). */
int
AG_FileDlgSetDirectory(AG_FileDlg *fd, const char *fmt, ...)
{
	char path[AG_PATHNAME_MAX];
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(path, sizeof(path), fmt, ap);
	va_end(ap);
	
	return AG_FileDlgSetDirectoryS(fd, path);
}

/* Move to the specified directory (C string). */
int
AG_FileDlgSetDirectoryS(AG_FileDlg *fd, const char *dir)
{
	AG_FileInfo info;
	char ncwd[AG_PATHNAME_MAX], *c;
	
	AG_ObjectLock(fd);

	if (dir[0] == '.' && dir[1] == '\0') {
		Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
	} else if (dir[0] == '.' && dir[1] == '.' && dir[2] == '\0') {
		if (!AG_PathIsFilesystemRoot(fd->cwd)) {
			Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
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
		Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
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
	if (Strlcpy(fd->cwd, ncwd, sizeof(fd->cwd)) >= sizeof(fd->cwd)) {
		AG_SetError(_("Path is too long: `%s'"), ncwd);
		goto fail;
	}
	if (fd->dirMRU != NULL) {
		AG_SetString(AG_ConfigObject(), fd->dirMRU, fd->cwd);
		AG_ConfigSave();
	}

	AG_TlistScrollToStart(fd->tlDirs);
	AG_TlistScrollToStart(fd->tlFiles);

	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

/* Set the current directory (fetch default from specified MRU). */
void
AG_FileDlgSetDirectoryMRU(AG_FileDlg *fd, const char *key, const char *dflt)
{
	AG_Config *cfg = AG_ConfigObject();
	char *s;

	AG_ObjectLock(fd);
	AG_ObjectLock(cfg);

	if (AG_Defined(cfg,key) && (s = AG_GetStringDup(cfg,key))) {
		AG_FileDlgSetDirectoryS(fd, s);
		Free(s);
	} else {
		AG_SetString(cfg, key, dflt);
		if (AG_ConfigSave() == -1) {
			Verbose("Saving MRU(%s): %s\n", key, AG_GetError());
		}
		AG_FileDlgSetDirectoryS(fd, dflt);
	}
	fd->dirMRU = Strdup(key);
	
	AG_ObjectUnlock(cfg);
	AG_ObjectUnlock(fd);
}

/* Set the current filename (format string). */
void
AG_FileDlgSetFilename(AG_FileDlg *fd, const char *fmt, ...)
{
	char file[AG_FILENAME_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	Vsnprintf(file, sizeof(file), fmt, ap);
	va_end(ap);

	AG_ObjectLock(fd);
	SetFilename(fd, file);
	AG_TextboxSetString(fd->tbFile, file);
/*	AG_TextboxSetCursorPos(fd->tbFile, -1); */
	AG_ObjectUnlock(fd);
}

/* Set the current filename (C string). */
void
AG_FileDlgSetFilenameS(AG_FileDlg *fd, const char *s)
{
	AG_ObjectLock(fd);
	SetFilename(fd, s);
	AG_TextboxSetString(fd->tbFile, s);
/*	AG_TextboxSetCursorPos(fd->tbFile, -1); */
	AG_ObjectUnlock(fd);
}

static void
MaskOptionSelected(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);

	RefreshListing(fd);
}

AG_FileDlg *
AG_FileDlgNew(void *parent, Uint flags)
{
	AG_FileDlg *fd;

	fd = Malloc(sizeof(AG_FileDlg));
	AG_ObjectInit(fd, &agFileDlgClass);
	fd->flags |= flags;
	if (flags & AG_FILEDLG_HFILL) { AG_ExpandHoriz(fd); }
	if (flags & AG_FILEDLG_VFILL) { AG_ExpandVert(fd); }
	if (flags & AG_FILEDLG_MULTI) { fd->tlFiles->flags |= AG_TLIST_MULTI; }

	/* File type selector */
	if (!(flags & AG_FILEDLG_NOTYPESELECT)) {
		fd->comTypes = AG_ComboNew(fd, AG_COMBO_HFILL, _("Type: "));
		AG_SetEvent(fd->comTypes, "combo-selected",
		    SelectedType, "%p", fd);
	}
	/* "Mask files" checkboxes. */
	if (!(flags & AG_FILEDLG_NOMASKOPTS)) {
		fd->cbMaskExt = AG_CheckboxNewFlag(fd, 0,
		    _("Mask files by extension"),
		    &fd->flags, AG_FILEDLG_MASK_EXT);
		AG_SetEvent(fd->cbMaskExt, "checkbox-changed",
		    MaskOptionSelected, "%p", fd);
		AG_SetStyle(fd->cbMaskExt, "font-size", "80%");
	
		fd->cbMaskHidden = AG_CheckboxNewFlag(fd, 0,
		    _("Mask hidden files"),
		    &fd->flags, AG_FILEDLG_MASK_HIDDEN);
		AG_SetEvent(fd->cbMaskHidden, "checkbox-changed",
		    MaskOptionSelected, "%p", fd);
		AG_SetStyle(fd->cbMaskHidden, "font-size", "80%");
	}
	if (!(flags & AG_FILEDLG_NOBUTTONS)) {
		fd->btnOk = AG_ButtonNewS(fd, 0, _("OK"));
		AG_SetEvent(fd->btnOk, "button-pushed", PressedOK, "%p", fd);
		fd->btnCancel = AG_ButtonNewS(fd, 0, _("Cancel"));
		AG_SetEvent(fd->btnCancel, "button-pushed", PressedCancel, "%p", fd);
	}
	AG_ObjectAttach(parent, fd);
	return (fd);
}

static void
Init(void *obj)
{
	AG_FileDlg *fd = obj;

	fd->flags = AG_FILEDLG_RESET_ONSHOW;
	fd->cfile[0] = '\0';
	fd->dirMRU = NULL;
	(void)AG_GetCWD(fd->cwd, sizeof(fd->cwd));
	fd->optsCtr = NULL;
	fd->okAction = NULL;
	fd->cancelAction = NULL;
	fd->btnOk = NULL;
	fd->btnCancel = NULL;
	fd->cbMaskExt = NULL;
	fd->cbMaskHidden = NULL;
	fd->comTypes = NULL;
	TAILQ_INIT(&fd->types);

	fd->hPane = AG_PaneNewHoriz(fd, AG_PANE_EXPAND);
	AG_PaneMoveDividerPct(fd->hPane, 50);
	AG_PaneResizeAction(fd->hPane, AG_PANE_DIVIDE_EVEN);

	/* Shortcuts combo. */
	fd->comLoc = AG_ComboNewS(fd->hPane->div[0], AG_COMBO_HFILL, NULL);
	AG_ComboSizeHint(fd->comLoc, "XXXXXXXXXXXXXXXXXXXXXXXXXXXX", 5);
	AG_TlistSetCompareFn(fd->comLoc->list, AG_TlistCompareStrings);
	AG_SetEvent(fd->comLoc, "combo-selected", LocSelected, "%p", fd);

	/* Directories list. */
	fd->tlDirs = AG_TlistNew(fd->hPane->div[0], AG_TLIST_EXPAND);
	AG_SetEvent(fd->tlDirs, "tlist-dblclick", DirSelected, "%p", fd);
	AG_TlistSizeHint(fd->tlDirs, "XXXXXXXXXXXXXX", 8);

	/* Files list. */
	fd->tlFiles = AG_TlistNew(fd->hPane->div[1], AG_TLIST_EXPAND);
	AG_TlistSizeHint(fd->tlFiles, "XXXXXXXXXXXXXXXXXX", 8);
	AG_SetEvent(fd->tlFiles, "tlist-selected", FileSelected, "%p", fd);
	AG_SetEvent(fd->tlFiles, "tlist-dblclick", FileDblClicked, "%p", fd);

	/* Current directory label. */
	fd->lbCwd = AG_LabelNewPolled(fd, AG_LABEL_HFILL, ("Directory: %s"), &fd->cwd[0]);
	AG_LabelSizeHint(fd->lbCwd, 1, _("Directory: XXXXXXXXXXXXX"));
	AG_SetStyle(fd->lbCwd, "font-size", "90%");

	/* Manual file/directory entry textbox. */
	fd->tbFile = AG_TextboxNewS(fd, AG_TEXTBOX_EXCL, _("File: "));
	AG_SetEvent(fd->tbFile, "textbox-postchg", TextboxChanged, "%p", fd);
	AG_SetEvent(fd->tbFile, "textbox-return", TextboxReturn, "%p", fd);

	AG_AddEvent(fd, "widget-shown", OnShow, NULL);
}

/*
 * Register an event handler for the "OK" button. Overrides type-specific
 * handlers.
 */
void
AG_FileDlgOkAction(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(fd);
	if (fd->okAction != NULL) {
		AG_UnsetEvent(fd, fd->okAction->name);
	}
	fd->okAction = AG_SetEvent(fd, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(fd->okAction, fmt);
#ifdef AG_THREADS
	if (fd->flags & AG_FILEDLG_ASYNC)
		fd->okAction->flags |= AG_EVENT_ASYNC;
#endif
	AG_ObjectUnlock(fd);
}

/* Register an event handler for the "Cancel" button. */
void
AG_FileDlgCancelAction(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(fd);
	if (fd->cancelAction != NULL) {
		AG_UnsetEvent(fd, fd->cancelAction->name);
	}
	fd->cancelAction = AG_SetEvent(fd, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(fd->cancelAction, fmt);
	AG_ObjectUnlock(fd);
}

static void
Destroy(void *obj)
{
	AG_FileDlg *fd = obj;
	AG_FileType *ft, *ft2;
	AG_FileOption *fo, *fo2;
	Uint i;

	for (ft = TAILQ_FIRST(&fd->types);
	     ft != TAILQ_END(&fd->types);
	     ft = ft2) {
		ft2 = TAILQ_NEXT(ft, types);
	
		for (fo = TAILQ_FIRST(&ft->opts);
		     fo != TAILQ_END(&ft->opts);
		     fo = fo2) {
			fo2 = TAILQ_NEXT(fo, opts);
			Free(fo);
		}
		for (i = 0; i < ft->nexts; i++) {
			Free(ft->exts[i]);
		}
		Free(ft->exts);
		Free(ft);
	}
	Free(fd->dirMRU);
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
	AG_FileDlg *fd = obj;
	AG_SizeReq rChld, rOk, rCancel;

	AG_WidgetSizeReq(fd->hPane, &rChld);
	r->w = rChld.w;
	r->h = rChld.h+4;
	AG_WidgetSizeReq(fd->lbCwd, &rChld);
	r->h += rChld.h+5;
	AG_WidgetSizeReq(fd->tbFile, &rChld);
	r->h += rChld.h+5;
	if (fd->comTypes != NULL) {
		AG_WidgetSizeReq(fd->comTypes, &rChld);
		r->h += rChld.h+4;
	}
	if (!(fd->flags & AG_FILEDLG_NOMASKOPTS)) {
		AG_WidgetSizeReq(fd->cbMaskExt, &rChld);
		r->h += rChld.h+2;
		AG_WidgetSizeReq(fd->cbMaskHidden, &rChld);
		r->h += rChld.h+4;
	}
	if (!(fd->flags & AG_FILEDLG_NOBUTTONS)) {
		AG_WidgetSizeReq(fd->btnOk, &rOk);
		AG_WidgetSizeReq(fd->btnCancel, &rCancel);
		r->h += MAX(rOk.h,rCancel.h)+1;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_FileDlg *fd = obj;
	AG_SizeReq r;
	AG_SizeAlloc aChld;
	int hBtn = 0, wBtn = a->w/2;

	if (!(fd->flags & AG_FILEDLG_NOBUTTONS)) {
		AG_WidgetSizeReq(fd->btnOk, &r);
		hBtn = MAX(hBtn, r.h);
		AG_WidgetSizeReq(fd->btnCancel, &r);
		hBtn = MAX(hBtn, r.h);
	} else {
		wBtn = 0;
		hBtn = 0;
	}

	/* Size horizontal pane. XXX ridiculous */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w;
	aChld.h = a->h - hBtn - 10;
	AG_WidgetSizeReq(fd->lbCwd, &r);
	aChld.h -= r.h;
	AG_WidgetSizeReq(fd->tbFile, &r);
	aChld.h -= r.h;
	if (fd->comTypes != NULL) {
		AG_WidgetSizeReq(fd->comTypes, &r);
		aChld.h -= (r.h + 4);
	}
	if (!(fd->flags & AG_FILEDLG_NOMASKOPTS)) {
		AG_WidgetSizeReq(fd->cbMaskExt, &r);
		aChld.h -= (r.h + 2);
		AG_WidgetSizeReq(fd->cbMaskHidden, &r);
		aChld.h -= (r.h + 4);
	}
	aChld.h -= 10;
	AG_WidgetSizeAlloc(fd->hPane, &aChld);
	aChld.y += aChld.h+4;

	/* Size cwd label. */
	AG_WidgetSizeReq(fd->lbCwd, &r);
	aChld.h = r.h;
	AG_WidgetSizeAlloc(fd->lbCwd, &aChld);
	aChld.y += aChld.h+5;

	/* Size entry textbox. */
	AG_WidgetSizeReq(fd->tbFile, &r);
	aChld.h = r.h;
	AG_WidgetSizeAlloc(fd->tbFile, &aChld);
	aChld.y += aChld.h+5;

	/* Size type selector */
	if (fd->comTypes != NULL) {
		AG_WidgetSizeReq(fd->comTypes, &r);
		aChld.h = r.h;
		AG_WidgetSizeAlloc(fd->comTypes, &aChld);
		aChld.y += aChld.h+4;
	}
	if (!(fd->flags & AG_FILEDLG_NOMASKOPTS)) {
		AG_WidgetSizeReq(fd->cbMaskExt, &r);
		aChld.h = r.h;
		AG_WidgetSizeAlloc(fd->cbMaskExt, &aChld);
		aChld.y += aChld.h+2;
		AG_WidgetSizeReq(fd->cbMaskHidden, &r);
		aChld.h = r.h;
		AG_WidgetSizeAlloc(fd->cbMaskHidden, &aChld);
		aChld.y += aChld.h+4;
	}

	/* Size buttons */
	if (!(fd->flags & AG_FILEDLG_NOBUTTONS)) {
		aChld.w = wBtn;
		aChld.h = hBtn;
		AG_WidgetSizeAlloc(fd->btnOk, &aChld);
		aChld.x = wBtn;
		if (wBtn*2 < a->w) { aChld.w++; }
		aChld.h = hBtn;
		AG_WidgetSizeAlloc(fd->btnCancel, &aChld);
	}
	return (0);
}

/* Register a new file type. */
AG_FileType *
AG_FileDlgAddType(AG_FileDlg *fd, const char *descr, const char *exts,
    AG_IntFn fn, const char *fmt, ...)
{
	AG_FileType *ft;
	char *dexts, *ds, *ext;
	AG_TlistItem *it;

	ft = Malloc(sizeof(AG_FileType));
	ft->fd = fd;
	ft->descr = descr;
	ft->exts = Malloc(sizeof(char *));
	ft->nexts = 0;
	TAILQ_INIT(&ft->opts);

	ds = dexts = Strdup(exts);
	while ((ext = AG_Strsep(&ds, ",;")) != NULL) {
		ft->exts = Realloc(ft->exts, (ft->nexts+1)*sizeof(char *));
		ft->exts[ft->nexts++] = Strdup(ext);
	}
	Free(dexts);
	
	AG_ObjectLock(fd);

	if (fn != NULL) {
		ft->action = AG_SetIntFn(fd, fn, NULL);
		AG_EVENT_GET_ARGS(ft->action, fmt);
#ifdef AG_THREADS
		if (fd->flags & AG_FILEDLG_ASYNC)
			ft->action->flags |= AG_EVENT_ASYNC;
#endif
	} else {
		ft->action = NULL;
	}
	if (fd->comTypes != NULL) {
		it = AG_TlistAdd(fd->comTypes->list, NULL, "%s (%s)", descr, exts);
		it->p1 = ft;
		if (TAILQ_EMPTY(&fd->types)) {
			AG_ComboSelectPointer(fd->comTypes, ft);
		}
	}
	TAILQ_INSERT_TAIL(&fd->types, ft, types);
	
	AG_ObjectUnlock(fd);
	return (ft);
}

AG_FileOption *
AG_FileOptionNewBool(AG_FileType *ft, const char *descr, const char *key,
    int dflt)
{
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = descr;
	fto->key = key;
	fto->unit = NULL;
	fto->type = AG_FILEDLG_BOOL;
	fto->data.i.val = dflt;
	
	AG_ObjectLock(ft->fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(ft->fd);
	return (fto);
}

AG_FileOption *
AG_FileOptionNewInt(AG_FileType *ft, const char *descr, const char *key,
    int dflt, int min, int max)
{
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = descr;
	fto->key = key;
	fto->unit = NULL;
	fto->type = AG_FILEDLG_INT;
	fto->data.i.val = dflt;
	fto->data.i.min = min;
	fto->data.i.max = max;
	
	AG_ObjectLock(ft->fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(ft->fd);
	return (fto);
}

AG_FileOption *
AG_FileOptionNewFlt(AG_FileType *ft, const char *descr, const char *key,
    float dflt, float min, float max, const char *unit)
{
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = descr;
	fto->key = key;
	fto->unit = NULL;
	fto->type = AG_FILEDLG_FLOAT;
	fto->data.flt.val = dflt;
	fto->data.flt.min = min;
	fto->data.flt.max = max;
	
	AG_ObjectLock(ft->fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(ft->fd);
	return (fto);
}

AG_FileOption *
AG_FileOptionNewDbl(AG_FileType *ft, const char *descr, const char *key,
    double dflt, double min, double max, const char *unit)
{
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = descr;
	fto->key = key;
	fto->unit = unit;
	fto->type = AG_FILEDLG_DOUBLE;
	fto->data.dbl.val = dflt;
	fto->data.dbl.min = min;
	fto->data.dbl.max = max;
	
	AG_ObjectLock(ft->fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(ft->fd);
	return (fto);
}

AG_FileOption *
AG_FileOptionNewString(AG_FileType *ft, const char *descr, const char *key,
    const char *dflt)
{
	AG_FileOption *fto;

	fto = Malloc(sizeof(AG_FileOption));
	fto->descr = descr;
	fto->key = key;
	fto->unit = NULL;
	fto->type = AG_FILEDLG_STRING;
	Strlcpy(fto->data.s, dflt, sizeof(fto->data.s));
	
	AG_ObjectLock(ft->fd);
	TAILQ_INSERT_TAIL(&ft->opts, fto, opts);
	AG_ObjectUnlock(ft->fd);
	return (fto);
}

/* The FileDlg must be locked. */
AG_FileOption *
AG_FileOptionGet(AG_FileType *ft, const char *key)
{
	AG_FileOption *fo;

	TAILQ_FOREACH(fo, &ft->opts, opts) {
		if (strcmp(fo->key, key) == 0)
			break;
	}
	return (fo);
}

int
AG_FileOptionInt(AG_FileType *ft, const char *key)
{
	AG_FileOption *fo;
	int rv;

	AG_ObjectLock(ft->fd);
	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.i.val : -1;
	AG_ObjectUnlock(ft->fd);
	return (rv);
}

float
AG_FileOptionFlt(AG_FileType *ft, const char *key)
{
	AG_FileOption *fo;
	float rv;

	AG_ObjectLock(ft->fd);
	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.flt.val : 0.0;
	AG_ObjectUnlock(ft->fd);
	return (rv);
}

double
AG_FileOptionDbl(AG_FileType *ft, const char *key)
{
	AG_FileOption *fo;
	double rv;

	AG_ObjectLock(ft->fd);
	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.dbl.val : 0.0;
	AG_ObjectUnlock(ft->fd);
	return (rv);
}

char *
AG_FileOptionString(AG_FileType *ft, const char *key)
{
	AG_FileOption *fo;
	char *rv;

	AG_ObjectLock(ft->fd);
	fo = AG_FileOptionGet(ft, key);
	rv = (fo != NULL) ? fo->data.s : "";
	AG_ObjectUnlock(ft->fd);
	return (rv);
}

AG_WidgetClass agFileDlgClass = {
	{
		"Agar(Widget:FileDlg)",
		sizeof(AG_FileDlg),
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
