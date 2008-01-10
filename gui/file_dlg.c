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

#include <config/threads.h>

#include <core/core.h>
#include <core/config.h>
#include <core/dir.h>
#include <core/file.h>

#include "file_dlg.h"

#include <gui/hbox.h>
#include <gui/numerical.h>
#include <gui/checkbox.h>

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "icons.h"

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

	AG_ObjectAttach(parent, fd);
	return (fd);
}

AG_FileDlg *
AG_FileDlgNewMRU(void *parent, const char *mruKey, Uint flags)
{
	AG_FileDlg *fd;

	fd = AG_FileDlgNew(parent, flags);
	AG_FileDlgSetDirectoryMRU(fd, mruKey, AG_String(agConfig,"save-path"));
	return (fd);
}

void
AG_FileDlgSetOptionContainer(AG_FileDlg *fd, void *ctr)
{
	AG_ObjectLock(fd);
	fd->optsCtr = ctr;
	AG_ObjectUnlock(fd);
}

static int
AG_FilenameCompare(const void *p1, const void *p2)
{
	const char *s1 = *(const void **)p1;
	const char *s2 = *(const void **)p2;

	return (strcmp(s1, s2));
}

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
		char path[FILENAME_MAX];
		
		Strlcpy(path, fd->cwd, sizeof(path));
		Strlcat(path, AG_PATHSEP, sizeof(path));
		Strlcat(path, dir->ents[i], sizeof(path));

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
		it = AG_TlistAdd(fd->tlDirs, agIconDirectory.s, "%s", dirs[i]);
		it->cat = "dir";
		it->p1 = it;
		Free(dirs[i]);
	}
	for (i = 0; i < nfiles; i++) {
		it = AG_TlistAdd(fd->tlFiles, agIconDoc.s, "%s", files[i]);
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

static void
DirSelected(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_TlistItem *ti;

	AG_ObjectLock(fd);
	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (AG_FileDlgSetDirectory(fd, ti->text) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		} else {
			AG_PostEvent(NULL, fd, "dir-selected", NULL);
			RefreshListing(fd);
		}
	}
	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(fd);
}

static void
ChooseFile(AG_FileDlg *fd, AG_Window *pwin)
{
	AG_TlistItem *it;

	AG_ObjectLock(fd);
	if ((it = AG_TlistSelectedItem(fd->comTypes->list)) != NULL) {
		AG_FileType *ft = it->p1;

		if (ft->action != NULL) {
			AG_PostEvent(NULL, fd, ft->action->name, "%s,%p",
			    fd->cfile, ft);
		}
		AG_PostEvent(NULL, fd, "file-chosen", "%s,%p", fd->cfile, ft);
	} else {
		AG_PostEvent(NULL, fd, "file-chosen", "%s,%p", fd->cfile, NULL);
	}
	if (fd->flags & AG_FILEDLG_CLOSEWIN) {
/*		AG_PostEvent(NULL, pwin, "window-close", NULL); */
		AG_ViewDetach(pwin);
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
	AG_ViewDetach(qwin);
}

static void
ReplaceFileDlg(AG_FileDlg *fd, AG_Window *pwin)
{
	AG_Window *win;
	AG_HBox *hb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_LabelNewStatic(win, 0, _("File %s exists. Overwrite?"), fd->cfile);
	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Yes"),
		    ReplaceFileConfirm, "%p,%p,%p", fd, win, pwin);
		AG_WidgetFocus(AG_ButtonNewFn(hb, 0, _("Cancel"),
		    AGWINDETACH(win)));
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
		AG_FileDlgSetFilename(fd, "%s", ti->text);
		AG_PostEvent(NULL, fd, "file-selected", "%s", fd->cfile);
	}
	AG_ObjectUnlock(tl);

	if ((ext = strrchr(fd->cfile, '.')) != NULL) {
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
		AG_FileDlgSetFilename(fd, "%s", itFile->text);

		if (fd->okAction != NULL) {
			AG_PostEvent(NULL, fd, fd->okAction->name, "%s,%p",
			    fd->cfile,
			    AG_TlistSelectedItemPtr(fd->comTypes->list));
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
		AG_PostEvent(NULL, fd, fd->okAction->name, "%s", fd->cfile);
	} else {
		CheckAccessAndChoose(fd);
	}
	AG_ObjectUnlock(fd);
}

static int
ProcessFilename(char *file, size_t len)
{
	char *end = &file[strlen(file)-1];
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

	AG_ObjectLock(fd);
	AG_TextboxCopyString(tb, path, sizeof(path));
	AG_FileDlgSetFilename(fd, "%s", path);
	AG_ObjectUnlock(fd);
}

static void
TextboxReturn(AG_Event *event)
{
	char file[MAXPATHLEN];
	AG_Textbox *tb = AG_SELF();
	AG_FileDlg *fd = AG_PTR(1);
	AG_FileInfo info;

	AG_ObjectLock(fd);
	AG_TextboxCopyString(tb, file, sizeof(file));
	if (file[0] == '\0' ||
	    ProcessFilename(file, sizeof(file)) == -1) {
		goto out;
	}
	AG_TextboxPrintf(tb, "%s", file);

	if ((AG_GetFileInfo(file, &info) == 0) &&
	    (info.type == AG_FILE_DIRECTORY)) {
		if (AG_FileDlgSetDirectory(fd, file) == 0) {
			RefreshListing(fd);
		} else {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
			goto out;
		}
	} else {
		AG_FileDlgSetFilename(fd, file);
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
		AG_PostEvent(NULL, fd, fd->cancelAction->name, NULL);
	} else if (fd->flags & AG_FILEDLG_CLOSEWIN) {
		if ((pwin = AG_WidgetParentWindow(fd)) != NULL) {
/*			AG_PostEvent(NULL, pwin, "window-close", NULL); */
			AG_ViewDetach(pwin);
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
	AG_Widget *chld;
	AG_Window *pWin;

	AG_ObjectLock(fd);
	if (fd->optsCtr == NULL) {
		AG_ObjectUnlock(fd);
		return;
	}
	OBJECT_FOREACH_CHILD(chld, fd->optsCtr, ag_widget) {
		AG_ObjectDetach(chld);
		AG_ObjectDestroy(chld);
	}
	TAILQ_FOREACH(fo, &ft->opts, opts) {
		switch (fo->type) {
		case AG_FILEDLG_BOOL:
			AG_CheckboxNewInt(fd->optsCtr, 0, &fo->data.i.val,
			    fo->descr);
			break;
		case AG_FILEDLG_INT:
			num = AG_NumericalNew(fd->optsCtr, 0, NULL, fo->descr);
			AG_WidgetBindInt(num, "value", &fo->data.i.val);
			AG_NumericalSetRangeInt(num, fo->data.i.min,
			    fo->data.i.max);
			break;
		case AG_FILEDLG_FLOAT:
			num = AG_NumericalNew(fd->optsCtr, 0, fo->unit,
			    fo->descr);
			AG_WidgetBindFloat(num, "value", &fo->data.flt.val);
			AG_NumericalSetRangeDbl(num, fo->data.flt.min,
			                             fo->data.flt.max);
			break;
		case AG_FILEDLG_DOUBLE:
			num = AG_NumericalNew(fd->optsCtr, 0, fo->unit,
			    fo->descr);
			AG_WidgetBindDouble(num, "value", &fo->data.dbl.val);
			AG_NumericalSetRange(num, fo->data.dbl.min,
			                          fo->data.dbl.max);
			break;
		case AG_FILEDLG_STRING:
			tbox = AG_TextboxNew(fd->optsCtr, AG_TEXTBOX_HFILL,
			    fo->descr);
			AG_TextboxBindUTF8(tbox, fo->data.s,
			    sizeof(fo->data.s));
			break;
		default:
			break;
		}
	}
	AG_ObjectUnlock(fd);

	if ((pWin = AG_WidgetParentWindow(fd)) != NULL)
		AG_WindowUpdate(pWin);
}

static void
WidgetShown(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	AG_TlistItem *it;
	int w, wMax = 0, nItems = 0;

/*	AG_WidgetFocus(fd->tbFile); */
	RefreshListing(fd);
	AG_PostEvent(NULL, fd->comTypes, "combo-selected", "%p", NULL);

	AG_COMBO_FOREACH(it, fd->comTypes) {
		AG_TextSize(it->text, &w, NULL);
		if (w > wMax) { wMax = w; }
		nItems++;
	}
	AG_ComboSizeHintPixels(fd->comTypes, wMax, nItems);
}

/*
 * Evaluate whether the cwd is the filesystem root. Return value is only
 * valid as long as the FileDlg is locked.
 */
int
AG_FileDlgAtRoot(AG_FileDlg *fd)
{
	int rv;
	
	AG_ObjectLock(fd);
	rv = (fd->cwd[0] == AG_PATHSEPC && fd->cwd[1] == '\0');
	AG_ObjectUnlock(fd);
	return (rv);
}

/* Move to the specified directory. */
int
AG_FileDlgSetDirectory(AG_FileDlg *fd, const char *dir)
{
	AG_FileInfo info;
	char ncwd[MAXPATHLEN], *c;
	
	AG_ObjectLock(fd);

	if (dir[0] == '.' && dir[1] == '\0') {
		if (AG_GetCWD(ncwd, sizeof(ncwd)) == -1) {
			AG_SetError("%s", AG_GetError());
			goto fail;
		}
	} else if (dir[0] == '.' && dir[1] == '.' && dir[2] == '\0') {
		if (!AG_FileDlgAtRoot(fd)) {
			Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
			if ((c = strrchr(ncwd, AG_PATHSEPC)) != NULL) {
				*c = '\0';
			}
			if (c == &ncwd[0]) {
				ncwd[0] = AG_PATHSEPC;
				ncwd[1] = '\0';
			}
		}
	} else if (dir[0] != AG_PATHSEPC) {
		Strlcpy(ncwd, fd->cwd, sizeof(ncwd));
		if (!(ncwd[0] == AG_PATHSEPC &&
		      ncwd[1] == '\0')) {
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
		AG_SetError(_("Path is too large: `%s'"), ncwd);
		goto fail;
	}
	if (fd->dirMRU != NULL) {
		AG_SetString(agConfig, fd->dirMRU, fd->cwd);
		AG_ObjectSave(agConfig);
	}

	AG_ObjectUnlock(fd);
	return (0);
fail:
	AG_ObjectUnlock(fd);
	return (-1);
}

void
AG_FileDlgSetDirectoryMRU(AG_FileDlg *fd, const char *key, const char *dflt)
{
	char *s;

	AG_ObjectLock(fd);
	if (AG_GetProp(agConfig, key, AG_PROP_STRING, &s) != NULL) {
		AG_FileDlgSetDirectory(fd, s);
	} else {
		AG_SetString(agConfig, key, dflt);
		AG_FileDlgSetDirectory(fd, dflt);
	}
	fd->dirMRU = Strdup(key);
	AG_ObjectUnlock(fd);
}

void
AG_FileDlgSetFilename(AG_FileDlg *fd, const char *fmt, ...)
{
	char file[FILENAME_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	Vsnprintf(file, sizeof(file), fmt, ap);
	va_end(ap);

	AG_ObjectLock(fd);
	AG_TextboxPrintf(fd->tbFile, "%s", file);
	if (file[0] == '/') {
		Strlcpy(fd->cfile, file, sizeof(fd->cfile));
	} else {
		Strlcpy(fd->cfile, fd->cwd, sizeof(fd->cfile));
		if (!AG_FileDlgAtRoot(fd)) {
			Strlcat(fd->cfile, AG_PATHSEP, sizeof(fd->cfile));
		}
		Strlcat(fd->cfile, file, sizeof(fd->cfile));
	}
	AG_ObjectUnlock(fd);
}

static void
Init(void *obj)
{
	AG_FileDlg *fd = obj;

	fd->flags = 0;
	fd->cfile[0] = '\0';
	fd->dirMRU = NULL;
	if (AG_GetCWD(fd->cwd, sizeof(fd->cwd)) == -1) {
		fprintf(stderr, "%s: %s", fd->cwd, AG_GetError());
	}
	fd->optsCtr = NULL;
	TAILQ_INIT(&fd->types);

	fd->hPane = AG_PaneNewHoriz(fd, AG_PANE_EXPAND);
	fd->tlDirs = AG_TlistNew(fd->hPane->div[0], AG_TLIST_EXPAND);
	fd->tlFiles = AG_TlistNew(fd->hPane->div[1], AG_TLIST_EXPAND);
	fd->lbCwd = AG_LabelNewPolled(fd, AG_LABEL_HFILL,
	    _("Directory: %s"), &fd->cwd[0]);
	AG_LabelSizeHint(fd->lbCwd, 1,
	    _("Directory: XXXXXXXXXXXXX"));

	fd->tbFile = AG_TextboxNew(fd, AG_TEXTBOX_HFILL, _("File: "));
	fd->comTypes = AG_ComboNew(fd, AG_COMBO_HFILL, _("Type: "));
	AG_TlistSizeHint(fd->tlDirs, "XXXXXXXXXXXXXX", 8);
	AG_TlistSizeHint(fd->tlFiles, "XXXXXXXXXXXXXXXXXX", 8);
	AG_WidgetFocus(fd->tbFile);

	fd->btnOk = AG_ButtonNew(fd, 0, _("OK"));
	fd->btnCancel = AG_ButtonNew(fd, 0, _("Cancel"));
	fd->okAction = NULL;
	fd->cancelAction = NULL;

	AG_SetEvent(fd, "widget-shown", WidgetShown, NULL);
	AG_SetEvent(fd->tlDirs, "tlist-dblclick", DirSelected, "%p", fd);
	AG_SetEvent(fd->tlFiles, "tlist-selected", FileSelected, "%p", fd);
	AG_SetEvent(fd->tlFiles, "tlist-dblclick", FileDblClicked, "%p", fd);
	AG_SetEvent(fd->tbFile, "textbox-postchg", TextboxChanged, "%p", fd);
	AG_SetEvent(fd->tbFile, "textbox-return", TextboxReturn, "%p", fd);
	AG_SetEvent(fd->btnOk, "button-pushed", PressedOK, "%p", fd);
	AG_SetEvent(fd->btnCancel, "button-pushed", PressedCancel, "%p", fd);
	AG_SetEvent(fd->comTypes, "combo-selected", SelectedType, "%p", fd);
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
#ifdef THREADS
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
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_FileDlg *fd = p;
	AG_SizeReq rChld, rOk, rCancel;

	AG_WidgetSizeReq(fd->hPane, &rChld);
	r->w = rChld.w;
	r->h = rChld.h+4;
	AG_WidgetSizeReq(fd->lbCwd, &rChld);
	r->h += rChld.h+1;
	AG_WidgetSizeReq(fd->tbFile, &rChld);
	r->h += rChld.h+2;
	AG_WidgetSizeReq(fd->comTypes, &rChld);
	r->h += rChld.h+2;
	AG_WidgetSizeReq(fd->btnOk, &rOk);
	AG_WidgetSizeReq(fd->btnCancel, &rCancel);
	r->h += MAX(rOk.h,rCancel.h)+1;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_FileDlg *fd = p;
	AG_SizeReq r;
	AG_SizeAlloc aChld;
	int hBtn = 0, wBtn = a->w/2;

	AG_WidgetSizeReq(fd->btnOk, &r);
	hBtn = MAX(hBtn, r.h);
	AG_WidgetSizeReq(fd->btnCancel, &r);
	hBtn = MAX(hBtn, r.h);

	/* Size horizontal pane */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w;
	aChld.h = a->h - hBtn - 10;
	AG_WidgetSizeReq(fd->lbCwd, &r);
	aChld.h -= r.h;
	AG_WidgetSizeReq(fd->tbFile, &r);
	aChld.h -= r.h;
	AG_WidgetSizeReq(fd->comTypes, &r);
	aChld.h -= r.h;
	AG_WidgetSizeAlloc(fd->hPane, &aChld);
	aChld.y += aChld.h+4;

	/* Size cwd label. */
	AG_WidgetSizeReq(fd->lbCwd, &r);
	aChld.h = r.h;
	AG_WidgetSizeAlloc(fd->lbCwd, &aChld);
	aChld.y += aChld.h+1;

	/* Size entry textbox. */
	AG_WidgetSizeReq(fd->tbFile, &r);
	aChld.h = r.h;
	AG_WidgetSizeAlloc(fd->tbFile, &aChld);
	aChld.y += aChld.h+2;

	/* Size type selector */
	AG_WidgetSizeReq(fd->comTypes, &r);
	aChld.h = r.h;
	AG_WidgetSizeAlloc(fd->comTypes, &aChld);
	aChld.y += aChld.h+2;

	/* Size buttons */
	aChld.w = wBtn;
	aChld.h = hBtn;
	AG_WidgetSizeAlloc(fd->btnOk, &aChld);
	aChld.x = wBtn;
	if (wBtn*2 < a->w) { aChld.w++; }
	aChld.h = hBtn;
	AG_WidgetSizeAlloc(fd->btnCancel, &aChld);

	return (0);
}

/* Register a new file type. */
AG_FileType *
AG_FileDlgAddType(AG_FileDlg *fd, const char *descr, const char *exts,
    void (*fn)(AG_Event *), const char *fmt, ...)
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
		ft->action = AG_SetEvent(fd, NULL, fn, NULL);
		AG_EVENT_GET_ARGS(ft->action, fmt);
#ifdef THREADS
		if (fd->flags & AG_FILEDLG_ASYNC)
			ft->action->flags |= AG_EVENT_ASYNC;
#endif
	} else {
		ft->action = NULL;
	}
	it = AG_TlistAdd(fd->comTypes->list, NULL, "%s (%s)", descr, exts);
	it->p1 = ft;
	if (TAILQ_EMPTY(&fd->types)) {
		AG_ComboSelectPointer(fd->comTypes, ft);
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
		"AG_Widget:AG_FileDlg",
		sizeof(AG_FileDlg),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	SizeRequest,
	SizeAllocate
};
