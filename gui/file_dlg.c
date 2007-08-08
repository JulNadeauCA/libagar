/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/view.h>

#include <compat/dir.h>
#include <compat/file.h>

#include "file_dlg.h"

#include <gui/hbox.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

static AG_WidgetOps agFileDlgOps = {
	{
		"AG_Widget:AG_FileDlg",
		sizeof(AG_FileDlg),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		AG_FileDlgDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	AG_FileDlgScale
};

AG_FileDlg *
AG_FileDlgNew(void *parent, Uint flags)
{
	AG_FileDlg *fd;

	fd = Malloc(sizeof(AG_FileDlg), M_OBJECT);
	AG_FileDlgInit(fd, flags);
	AG_ObjectAttach(parent, fd);
	if (fd->flags & AG_FILEDLG_FOCUS) {
		AG_WidgetFocus(fd);
	}
	return (fd);
}

static int
AG_FilenameCompare(const void *p1, const void *p2)
{
	const char *s1 = *(const void **)p1;
	const char *s2 = *(const void **)p2;

	return (strcoll(s1, s2));
}

static void
AG_RefreshListing(AG_FileDlg *fd)
{
	AG_TlistItem *it;
	AG_FileInfo info;
	AG_Dir *dir;
	char **dirs, **files;
	size_t i, ndirs = 0, nfiles = 0;

	if ((dir = AG_OpenDir(fd->cwd)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", fd->cwd, strerror(errno));
		return;
	}
	
	dirs = Malloc(sizeof(char *), M_WIDGET);
	files = Malloc(sizeof(char *), M_WIDGET);
	
	AG_MutexLock(&fd->tlDirs->lock);
	AG_MutexLock(&fd->tlFiles->lock);

	for (i = 0; i < dir->nents; i++) {
		char path[FILENAME_MAX];
		
		strlcpy(path, fd->cwd, sizeof(path));
		strlcat(path, AG_PATHSEP, sizeof(path));
		strlcat(path, dir->ents[i], sizeof(path));

		if (AG_FileDlgAtRoot(fd) && strcmp(dir->ents[i], "..")==0) {
			continue;
		}
		if (AG_GetFileInfo(path, &info) == -1) {
			continue;
		}
		if (info.type == AG_FILE_DIRECTORY) {
			dirs = Realloc(dirs, (ndirs + 1) * sizeof(char *));
			dirs[ndirs++] = Strdup(dir->ents[i]);
		} else {
			files = Realloc(files, (nfiles + 1) * sizeof(char *));
			files[nfiles++] = Strdup(dir->ents[i]);
		}
	}
	qsort(dirs, ndirs, sizeof(char *), AG_FilenameCompare);
	qsort(files, nfiles, sizeof(char *), AG_FilenameCompare);

	AG_TlistClear(fd->tlDirs);
	AG_TlistClear(fd->tlFiles);
	for (i = 0; i < ndirs; i++) {
		it = AG_TlistAdd(fd->tlDirs, AGICON(DIRECTORY_ICON),
		    "%s", dirs[i]);
		it->cat = "dir";
		it->p1 = it;
		Free(dirs[i], M_WIDGET);
	}
	for (i = 0; i < nfiles; i++) {
		it = AG_TlistAdd(fd->tlFiles, AGICON(FILE_ICON),
		    "%s", files[i]);
		it->cat = "file";
		it->p1 = it;
		Free(files[i], M_WIDGET);
	}
	Free(dirs, M_WIDGET);
	Free(files, M_WIDGET);
	AG_TlistRestore(fd->tlDirs);
	AG_TlistRestore(fd->tlFiles);
	
	AG_MutexUnlock(&fd->tlFiles->lock);
	AG_MutexUnlock(&fd->tlDirs->lock);
	AG_CloseDir(dir);
}

static void
DirSelected(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *ti;

	AG_MutexLock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (AG_FileDlgSetDirectory(fd, ti->text) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		} else {
			AG_PostEvent(NULL, fd, "dir-selected", NULL);
			AG_RefreshListing(fd);
		}
	}
	AG_MutexUnlock(&tl->lock);
}

static void
ChooseFile(AG_FileDlg *fd, AG_Window *pwin)
{
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(fd->comTypes->list)) != NULL) {
		AG_FileType *ft = it->p1;

		if (ft->action != NULL) {
			AG_PostEvent(NULL, fd, ft->action->name, "%s",
			    fd->cfile);
		}
		AG_PostEvent(NULL, fd, "file-chosen", "%s,%p", fd->cfile, ft);
	} else {
		AG_PostEvent(NULL, fd, "file-chosen", "%s,%p", fd->cfile, NULL);
	}
	if (fd->flags & AG_FILEDLG_CLOSEWIN)
		AG_PostEvent(NULL, pwin, "window-close", NULL);
}

static void
AG_ReplaceFileEv(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	AG_Window *qwin = AG_PTR(2);
	AG_Window *pwin = AG_PTR(3);

	ChooseFile(fd, pwin);
	AG_ViewDetach(qwin);
}

static void
AG_ReplaceFileDlg(AG_FileDlg *fd, AG_Window *pwin)
{
	AG_Window *win;
	AG_HBox *hb;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_LabelNewStatic(win, 0, _("File %s exists. Overwrite?"), fd->cfile);
	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Yes"),
		    AG_ReplaceFileEv, "%p,%p,%p", fd, win, pwin);
		AG_WidgetFocus(AG_ButtonNewFn(hb, 0, _("Cancel"),
		    AGWINDETACH(win)));
	}
	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

int
AG_FileDlgCheckReadAccess(AG_FileDlg *fd)
{
	AG_FileInfo info;
	
	if (AG_GetFileInfo(fd->cfile, &info) == -1) {
		return (-1);
	}
	if ((info.perms & AG_FILE_READABLE) == 0) {
		AG_SetError(_("%s: Read permission denied"), fd->cfile);
		return (-1);
	}
	return (0);
}

int
AG_FileDlgCheckWriteAccess(AG_FileDlg *fd)
{
	AG_FileInfo info;
	
	if (AG_GetFileInfo(fd->cfile, &info) == -1) {
		return (-1);
	}
	if ((info.perms & AG_FILE_WRITEABLE) == 0) {
		AG_SetError(_("%s: Write permission denied"), fd->cfile);
		return (-1);
	}
	return (0);
}

static void
CheckAccessAndChoose(AG_FileDlg *fd)
{
	AG_Window *pwin = AG_WidgetParentWindow(fd);
	char *s;

	for (s = &fd->cfile[0]; *s != '\0'; s++) {
		if (!isspace(*s))
			break;
	}
	if (*s == '\0')
		return;

	if (fd->flags & AG_FILEDLG_LOAD) {
		if (AG_FileDlgCheckReadAccess(fd) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
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
				AG_ReplaceFileDlg(fd, pwin);
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

	AG_MutexLock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		char path[MAXPATHLEN];

		AG_FileDlgSetFilename(fd, "%s", ti->text);
		AG_PostEvent(NULL, fd, "file-selected", "%s", fd->cfile);
	}
	AG_MutexUnlock(&tl->lock);
}

static void
FileDblClicked(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *itFile;

	AG_MutexLock(&tl->lock);
	if ((itFile = AG_TlistSelectedItem(tl)) != NULL) {
		AG_FileDlgSetFilename(fd, "%s", itFile->text);

		if (fd->okAction != NULL) {
			AG_PostEvent(NULL, fd, fd->okAction->name, "%s,%p",
			    fd->cfile,
			    AG_TlistSelectedItemPtr(fd->comTypes->list));
		} else {
			CheckAccessAndChoose(fd);
		}
	}
	AG_MutexUnlock(&tl->lock);
}

static void
PressedOK(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	AG_FileType *ft;

	if (fd->okAction != NULL) {
		AG_PostEvent(NULL, fd, fd->okAction->name, "%s", fd->cfile);
	} else {
		CheckAccessAndChoose(fd);
	}
}

static int
AG_ProcessFilename(char *file, size_t len)
{
	char *end = &file[strlen(file)-1];
	char dupseps[3];
	char *s;

	/* Remove trailing whitespaces. */
	while ((end >= file) && *end == ' ') {
		*end = '\0';
		end--;
	}
	if (file[0] == '\0')
		return (-1);

	/* Remove leading whitespaces. */
	for (s = file; *s == ' '; s++)
		;;
	if (s > file) {
		memmove(file, s, end-s+2);
		end -= (s-file);
	}
	if (file[0] == '\0')
		return (-1);

	/* Treat the root specially. */
	if (strcmp(file, AG_PATHSEP) == 0)
		return (0);

	/* Remove trailing path separators. */
	if (*end == AG_PATHSEPC) {
		*end = '\0';
		end--;
	}
	return (0);
}

static void
TextboxChanged(AG_Event *event)
{
	char path[MAXPATHLEN];
	AG_Textbox *tb = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	
	AG_TextboxCopyString(tb, path, sizeof(path));
	AG_FileDlgSetFilename(fd, "%s", path);
}

static void
TextboxReturn(AG_Event *event)
{
	char file[MAXPATHLEN];
	AG_Textbox *tb = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_FileType *ft;
	AG_FileInfo info;
	size_t last;

	AG_TextboxCopyString(tb, file, sizeof(file));
	if (file[0] == '\0' ||
	    AG_ProcessFilename(file, sizeof(file) == -1)) {
		return;
	}
	AG_TextboxPrintf(tb, "%s", file);

	if ((AG_GetFileInfo(file, &info) == 0) &&
	    (info.type == AG_FILE_DIRECTORY)) {
		if (AG_FileDlgSetDirectory(fd, file) == 0) {
			AG_RefreshListing(fd);
		} else {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
			return;
		}
	} else {
		AG_FileDlgSetFilename(fd, file);
		CheckAccessAndChoose(fd);
	}
}

static void
PressedCancel(AG_Event *event)
{
	AG_FileDlg *fd = AG_PTR(1);
	
	if (fd->cancelAction != NULL) {
		AG_PostEvent(NULL, fd, fd->cancelAction->name, NULL);
	} else if (fd->flags & AG_FILEDLG_CLOSEWIN) {
		AG_Window *pwin;
	
		if ((pwin = AG_WidgetParentWindow(fd)) != NULL)
			AG_PostEvent(NULL, pwin, "window-close", NULL);
	}
}

static void
WidgetShown(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();

	AG_WidgetScale(fd, WIDGET(fd)->w, WIDGET(fd)->h);
	AG_WidgetFocus(fd->tbFile);
	AG_RefreshListing(fd);
}

int
AG_FileDlgAtRoot(AG_FileDlg *fd)
{
	return (fd->cwd[0] == AG_PATHSEPC && fd->cwd[1] == '\0');
}

int
AG_FileDlgSetDirectory(AG_FileDlg *fd, const char *dir)
{
	AG_FileInfo info;
	char ncwd[MAXPATHLEN];
	char *c;

	if (dir[0] == '.' && dir[1] == '\0') {
		if ((getcwd(ncwd, sizeof(ncwd))) == NULL) {
			AG_SetError("getcwd: %s", strerror(errno));
			return (-1);
		}
	} else if (dir[0] == '.' && dir[1] == '.' && dir[2] == '\0') {
		if (!AG_FileDlgAtRoot(fd)) {
			strlcpy(ncwd, fd->cwd, sizeof(ncwd));
			if ((c = strrchr(ncwd, AG_PATHSEPC)) != NULL) {
				*c = '\0';
			}
			if (c == &ncwd[0]) {
				ncwd[0] = AG_PATHSEPC;
				ncwd[1] = '\0';
			}
		}
	} else if (dir[0] != AG_PATHSEPC) {
		strlcpy(ncwd, fd->cwd, sizeof(ncwd));
		if (!(ncwd[0] == AG_PATHSEPC &&
		      ncwd[1] == '\0')) {
			strlcat(ncwd, AG_PATHSEP, sizeof(ncwd));
		}
		strlcat(ncwd, dir, sizeof(ncwd));
	} else {
		strlcpy(ncwd, dir, sizeof(ncwd));
	}
	
	if (AG_GetFileInfo(ncwd, &info) == -1) {
		return (-1);
	}
	if (info.type != AG_FILE_DIRECTORY) {
		AG_SetError(_("%s: Not a directory"), ncwd);
		return (-1);
	}
	if ((info.perms & (AG_FILE_READABLE|AG_FILE_EXECUTABLE)) == 0) {
		AG_SetError(_("%s: Permission denied"), ncwd);
		return (-1);
	}
	if (strlcpy(fd->cwd, ncwd, sizeof(fd->cwd)) >= sizeof(fd->cwd)) {
		AG_SetError(_("Path is too large: `%s'"), ncwd);
		return (-1);
	}
	return (0);
}

void
AG_FileDlgSetFilename(AG_FileDlg *fd, const char *fmt, ...)
{
	char file[FILENAME_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(file, sizeof(file), fmt, ap);
	va_end(ap);

	AG_TextboxPrintf(fd->tbFile, "%s", file);
	if (file[0] == '/') {
		strlcpy(fd->cfile, file, sizeof(fd->cfile));
	} else {
		strlcpy(fd->cfile, fd->cwd, sizeof(fd->cfile));
		if (!AG_FileDlgAtRoot(fd)) {
			strlcat(fd->cfile, AG_PATHSEP, sizeof(fd->cfile));
		}
		strlcat(fd->cfile, file, sizeof(fd->cfile));
	}
}

void
AG_FileDlgInit(AG_FileDlg *fd, Uint flags)
{
	Uint wflags = 0;

	if (flags & AG_FILEDLG_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_FILEDLG_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(fd, &agFileDlgOps, wflags);
	fd->flags = flags;
	fd->cfile[0] = '\0';
	if ((getcwd(fd->cwd, sizeof(fd->cwd))) == NULL) {
		fprintf(stderr, "%s: %s", fd->cwd, strerror(errno));
	}
	TAILQ_INIT(&fd->types);

	fd->hPane = AG_HPaneNew(fd, AG_HPANE_HFILL|AG_HPANE_VFILL);
	fd->hDiv = AG_HPaneAddDiv(fd->hPane,
	    AG_BOX_VERT, AG_BOX_VFILL,
	    AG_BOX_VERT, AG_BOX_VFILL|AG_BOX_HFILL);
	fd->tlDirs = AG_TlistNew(fd->hDiv->box1, AG_TLIST_EXPAND);
	fd->tlFiles = AG_TlistNew(fd->hDiv->box2, AG_TLIST_EXPAND|
	    ((flags & AG_FILEDLG_MULTI) ? AG_TLIST_MULTI : 0));

	fd->lbCwd = AG_LabelNewPolled(fd, AG_LABEL_HFILL,
	    _("Directory: %s"), &fd->cwd[0]);
	AG_LabelPrescale(fd->lbCwd, 1,
	    _("Directory: XXXXXXXXXXXXX"));

	fd->tbFile = AG_TextboxNew(fd, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS,
	    _("File: "));
	fd->comTypes = AG_ComboNew(fd, AG_COMBO_HFILL, _("Type: "));
	AG_TlistPrescale(fd->tlDirs, "XXXXXXXXXXXXXX", 8);
	AG_TlistPrescale(fd->tlFiles, "XXXXXXXXXXXXXXXXXX", 8);

	fd->btnOk = AG_ButtonNew(fd, 0, _("OK"));
	fd->btnCancel = AG_ButtonNew(fd, 0, _("Cancel"));

	AG_SetEvent(fd, "widget-shown", WidgetShown, NULL);
	AG_SetEvent(fd->tlDirs, "tlist-dblclick", DirSelected, "%p", fd);
	AG_SetEvent(fd->tlFiles, "tlist-selected", FileSelected, "%p", fd);
	AG_SetEvent(fd->tlFiles, "tlist-dblclick", FileDblClicked, "%p", fd);

	AG_SetEvent(fd->tbFile, "textbox-postchg", TextboxChanged, "%p", fd);
	AG_SetEvent(fd->tbFile, "textbox-return", TextboxReturn, "%p", fd);
	AG_SetEvent(fd->btnOk, "button-pushed", PressedOK, "%p", fd);
	AG_SetEvent(fd->btnCancel, "button-pushed", PressedCancel, "%p", fd);

	fd->okAction = NULL;
	fd->cancelAction = NULL;
}

void
AG_FileDlgOkAction(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	if (fd->okAction != NULL) {
		AG_UnsetEvent(fd, fd->okAction->name);
	}
	fd->okAction = AG_SetEvent(fd, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(fd->okAction, fmt);
}

void
AG_FileDlgCancelAction(AG_FileDlg *fd, AG_EventFn fn, const char *fmt, ...)
{
	if (fd->cancelAction != NULL) {
		AG_UnsetEvent(fd, fd->cancelAction->name);
	}
	fd->cancelAction = AG_SetEvent(fd, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(fd->cancelAction, fmt);
}

void
AG_FileDlgDestroy(void *p)
{
	AG_FileDlg *fd = p;
	AG_FileType *ft, *ft2;
	Uint i;

	for (ft = TAILQ_FIRST(&fd->types);
	     ft != TAILQ_END(&fd->types);
	     ft = ft2) {
		ft2 = TAILQ_NEXT(ft, types);
		for (i = 0; i < ft->nexts; i++) {
			Free(ft->exts[i], M_WIDGET);
		}
		Free(ft->exts, M_WIDGET);
		Free(ft, M_WIDGET);
	}
	AG_WidgetDestroy(fd);
}

void
AG_FileDlgScale(void *p, int w, int h)
{
	AG_FileDlg *fd = p;
	int btn_h, y = 0;
	
	if (w == -1 && h == -1) {
		WIDGET_SCALE(fd->hPane, -1, -1);
		WIDGET_SCALE(fd->lbCwd , -1, -1);
		WIDGET_SCALE(fd->tbFile, -1, -1);
		WIDGET_SCALE(fd->comTypes, -1, -1);
		WIDGET_SCALE(fd->btnOk, -1, -1);
		WIDGET_SCALE(fd->btnCancel, -1, -1);
	
		WIDGET(fd)->w = WIDGET(fd->hPane)->w;
		WIDGET(fd)->h = WIDGET(fd->hPane)->h +
				 WIDGET(fd->lbCwd)->h+1 +
				 WIDGET(fd->tbFile)->h+1 +
				 WIDGET(fd->comTypes)->h+1 +
				 MAX(WIDGET(fd->btnOk)->h,
				     WIDGET(fd->btnCancel)->h)+2;
		return;
	}

	btn_h = MAX(WIDGET(fd->btnOk)->h, WIDGET(fd->btnCancel)->h);
	
	AG_WidgetScale(fd->hPane,
	    w,
	    h - WIDGET(fd->lbCwd)->h - WIDGET(fd->tbFile)->h -
	        WIDGET(fd->comTypes)->h - btn_h);

	AG_WidgetScale(fd->lbCwd, w, WIDGET(fd->lbCwd)->h);
	AG_WidgetScale(fd->tbFile, w, WIDGET(fd->tbFile)->h);
	AG_WidgetScale(fd->comTypes, w, WIDGET(fd->comTypes)->h);
	AG_WidgetScale(fd->btnOk, w/2, WIDGET(fd->tbFile)->h);
	AG_WidgetScale(fd->btnCancel, w/2, WIDGET(fd->tbFile)->h);
	
	WIDGET(fd->hPane)->x = 0;
	WIDGET(fd->hPane)->y = 0;
	WIDGET(fd->hPane)->w = w;
	WIDGET(fd->hPane)->h = h -
	    WIDGET(fd->tbFile)->h -
	    WIDGET(fd->lbCwd)->h -
	    WIDGET(fd->comTypes)->h - btn_h - 2;
	AG_WidgetScale(fd->hPane, WIDGET(fd->hPane)->w,
	    WIDGET(fd->hPane)->h);
	y += WIDGET(fd->hPane)->h + 1;

	WIDGET(fd->lbCwd)->x = 0;
	WIDGET(fd->lbCwd)->y = y;
	y += WIDGET(fd->lbCwd)->h + 1;

	WIDGET(fd->tbFile)->x = 0;
	WIDGET(fd->tbFile)->y = y;
	y += WIDGET(fd->tbFile)->h + 1;

	WIDGET(fd->comTypes)->x = 0;
	WIDGET(fd->comTypes)->y = y;
	y += WIDGET(fd->comTypes)->h + 4;
	
	WIDGET(fd->btnOk)->x = 0;
	WIDGET(fd->btnOk)->y = y;

	WIDGET(fd->btnCancel)->x = w/2;
	WIDGET(fd->btnCancel)->y = y;
	
	WIDGET(fd)->w = w;
	WIDGET(fd)->h = h;
}

AG_FileType *
AG_FileDlgAddType(AG_FileDlg *fd, const char *descr, const char *exts,
    void (*fn)(AG_Event *), const char *fmt, ...)
{
	AG_FileType *ft;
	char *dexts, *ds, *ext;
	AG_TlistItem *it;

	ft = Malloc(sizeof(AG_FileType), M_WIDGET);
	ft->descr = descr;
	ft->exts = Malloc(sizeof(char *), M_WIDGET);
	ft->nexts = 0;

	ds = dexts = Strdup(exts);
	while ((ext = AG_Strsep(&ds, ",;")) != NULL) {
		ft->exts = Realloc(ft->exts, (ft->nexts+1)*sizeof(char *));
		ft->exts[ft->nexts++] = Strdup(ext);
	}
	Free(dexts, M_WIDGET);

	if (fn != NULL) {
		ft->action = AG_SetEvent(fd, NULL, fn, NULL);
		AG_EVENT_GET_ARGS(ft->action, fmt);
	} else {
		ft->action = NULL;
	}

	it = AG_TlistAdd(fd->comTypes->list, NULL, "%s (%s)", descr, exts);
	it->p1 = ft;
	if (TAILQ_EMPTY(&fd->types))
		AG_ComboSelectPointer(fd->comTypes, ft);

	TAILQ_INSERT_TAIL(&fd->types, ft, types);
	return (ft);
}
