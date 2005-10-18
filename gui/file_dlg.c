/*	$Csoft: file_dlg.c,v 1.11 2005/10/01 14:15:38 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifdef WIN32
#define FILESEPC '\\'
#define FILESEPSTR "\\"
#else
#define FILESEPC '/'
#define FILESEPSTR "/"
#endif

static AG_WidgetOps agFileDlgOps = {
	{
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
AG_FileDlgNew(void *parent, int flags)
{
	AG_FileDlg *fdg;

	fdg = Malloc(sizeof(AG_FileDlg), M_OBJECT);
	AG_FileDlgInit(fdg, flags);
	AG_ObjectAttach(parent, fdg);
	return (fdg);
}

static int
compare_filenames(const void *p1, const void *p2)
{
	const char *s1 = *(const void **)p1;
	const char *s2 = *(const void **)p2;

	return (strcoll(s1, s2));
}

static void
update_listing(AG_FileDlg *fdg)
{
	AG_TlistItem *it;
	AG_FileInfo info;
	AG_Dir *dir;
	char **dirs, **files;
	size_t i, ndirs = 0, nfiles = 0;

	if ((dir = AG_OpenDir(fdg->cwd)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, ".: %s", strerror(errno));
		return;
	}
	
	dirs = Malloc(sizeof(char *), M_WIDGET);
	files = Malloc(sizeof(char *), M_WIDGET);
	
	AG_MutexLock(&fdg->tlDirs->lock);
	AG_MutexLock(&fdg->tlFiles->lock);

	for (i = 0; i < dir->nents; i++) {
		char path[FILENAME_MAX];
		
		strlcpy(path, fdg->cwd, sizeof(path));
		strlcat(path, FILESEPSTR, sizeof(path));
		strlcat(path, dir->ents[i], sizeof(path));

		if (AG_FileDlgAtRoot(fdg) && strcmp(dir->ents[i], "..")==0) {
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
	qsort(dirs, ndirs, sizeof(char *), compare_filenames);
	qsort(files, nfiles, sizeof(char *), compare_filenames);

	AG_TlistClear(fdg->tlDirs);
	AG_TlistClear(fdg->tlFiles);
	for (i = 0; i < ndirs; i++) {
		it = AG_TlistAdd(fdg->tlDirs, AGICON(DIRECTORY_ICON),
		    "%s", dirs[i]);
		it->class = "dir";
		it->p1 = (void *)i;
		Free(dirs[i], M_WIDGET);
	}
	for (i = 0; i < nfiles; i++) {
		it = AG_TlistAdd(fdg->tlFiles, AGICON(FILE_ICON),
		    "%s", files[i]);
		it->class = "file";
		it->p1 = (void *)i;
		Free(files[i], M_WIDGET);
	}
	Free(dirs, M_WIDGET);
	Free(files, M_WIDGET);
	AG_TlistRestore(fdg->tlDirs);
	AG_TlistRestore(fdg->tlFiles);
	
	AG_MutexUnlock(&fdg->tlFiles->lock);
	AG_MutexUnlock(&fdg->tlDirs->lock);
	AG_CloseDir(dir);
}

static void
select_dir(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fdg = AG_PTR(1);
	AG_TlistItem *ti;

	AG_MutexLock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (AG_FileDlgSetDirectory(fdg, ti->text) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		} else {
			AG_PostEvent(NULL, fdg, "dir-selected", NULL);
			update_listing(fdg);
		}
	}
	AG_MutexUnlock(&tl->lock);
}

static void
AG_FileDlgProcessFile(AG_FileDlg *fdg)
{
	AG_TlistItem *it;
	AG_Window *pwin;

	if ((it = AG_TlistSelectedItem(fdg->comTypes->list)) != NULL) {
		AG_FileType *ft = it->p1;

		if (ft->action != NULL)
			AG_PostEvent(NULL, fdg, ft->action->name, "%s",
			    fdg->cfile);
	}
	if ((fdg->flags & AG_FILEDLG_CLOSEWIN) == 0) {
		pwin = AG_WidgetParentWindow(fdg);
		AG_PostEvent(NULL, pwin, "window-close", NULL);
	}
}

static void
select_file(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fdg = AG_PTR(1);
	AG_TlistItem *ti;

	AG_MutexLock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		char path[MAXPATHLEN];

		AG_FileDlgSetFilename(fdg, ti->text);
		AG_PostEvent(NULL, fdg, "file-selected", "%s", fdg->cfile);
	}
	AG_MutexUnlock(&tl->lock);
}

static void
select_and_validate_file(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_FileDlg *fdg = AG_PTR(1);
	AG_TlistItem *ti;

	AG_MutexLock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_FileDlgSetFilename(fdg, ti->text);
		AG_PostEvent(NULL, fdg, "file-validated", "%s", fdg->cfile);
		AG_FileDlgProcessFile(fdg);
	}
	AG_MutexUnlock(&tl->lock);
}

static void
validate_file(AG_Event *event)
{
	AG_FileDlg *fdg = AG_PTR(1);
	AG_FileType *ft;

	AG_PostEvent(NULL, fdg, "file-validated", "%s", fdg->cfile);
	AG_FileDlgProcessFile(fdg);
}

static void
enter_file(AG_Event *event)
{
	char file[MAXPATHLEN];
	AG_FileDlg *fdg = AG_PTR(1);
	AG_FileType *ft;
	AG_FileInfo info;

	AG_TextboxCopyString(fdg->tbFile, file, sizeof(file));
	if (AG_GetFileInfo(file, &info) == -1) {
		goto fail;
	}
	if (info.type == AG_FILE_DIRECTORY) {
		if (AG_FileDlgSetDirectory(fdg, file) == 0) {
			update_listing(fdg);
		} else {
			AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
			return;
		}
	} else {
		AG_FileDlgSetFilename(fdg, file);
		AG_PostEvent(NULL, fdg, "file-validated", "%s", fdg->cfile);
		AG_FileDlgProcessFile(fdg);
	}
	return;
fail:
	AG_TextMsg(AG_MSG_ERROR, "%s: %s", file, strerror(errno));
}

static void
do_cancel(AG_Event *event)
{
	AG_FileDlg *fdg = AG_PTR(1);
	AG_Window *pwin;
	
	if ((fdg->flags & AG_FILEDLG_CLOSEWIN) == 0) {
		pwin = AG_WidgetParentWindow(fdg);
		AG_PostEvent(NULL, pwin, "window-close", NULL);
	}
}

static void
file_dlg_shown(AG_Event *event)
{
	AG_FileDlg *fdg = AG_SELF();

	AG_WidgetScale(fdg, AGWIDGET(fdg)->w, AGWIDGET(fdg)->h);
	AG_WidgetFocus(fdg->tbFile);
	update_listing(fdg);
}

int
AG_FileDlgAtRoot(AG_FileDlg *fdg)
{
	return (fdg->cwd[0] == FILESEPC && fdg->cwd[1] == '\0');
}

int
AG_FileDlgSetDirectory(AG_FileDlg *fdg, const char *dir)
{
	char *c;

	if (dir[0] == '.' && dir[1] == '\0') {
		if ((getcwd(fdg->cwd, sizeof(fdg->cwd))) == NULL) {
			AG_SetError("getcwd: %s", strerror(errno));
			return (-1);
		}
	} else if (dir[0] == '.' && dir[1] == '.' && dir[2] == '\0') {
		if (!AG_FileDlgAtRoot(fdg)) {
			if ((c = strrchr(fdg->cwd, FILESEPC)) != NULL)
				*c = '\0';
		}
	} else if (dir[0] != FILESEPC) {
		strlcat(fdg->cwd, FILESEPSTR, sizeof(fdg->cwd));
		strlcat(fdg->cwd, dir, sizeof(fdg->cwd));
	} else {
		strlcpy(fdg->cwd, dir, sizeof(fdg->cwd));
	}
	return (0);
}

void
AG_FileDlgSetFilename(AG_FileDlg *fdg, const char *fmt, ...)
{
	char file[FILENAME_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(file, sizeof(file), fmt, ap);
	va_end(ap);

	AG_TextboxPrintf(fdg->tbFile, "%s", file);
	
	strlcpy(fdg->cfile, fdg->cwd, sizeof(fdg->cfile));
	if (!AG_FileDlgAtRoot(fdg)) {
		strlcat(fdg->cfile, FILESEPSTR, sizeof(fdg->cfile));
	}
	strlcat(fdg->cfile, file, sizeof(fdg->cfile));
}

void
AG_FileDlgInit(AG_FileDlg *fdg, int flags)
{
	AG_WidgetInit(fdg, "file-dlg", &agFileDlgOps,
	    AG_WIDGET_WFILL|AG_WIDGET_HFILL);
	fdg->flags = flags;
	fdg->cfile[0] = '\0';
	if ((getcwd(fdg->cwd, sizeof(fdg->cwd))) == NULL) {
		fprintf(stderr, "%s: %s", fdg->cwd, strerror(errno));
	}
	TAILQ_INIT(&fdg->types);

	fdg->hPane = AG_HPaneNew(fdg, AG_HPANE_WFILL|AG_HPANE_HFILL);
	fdg->hDiv = AG_HPaneAddDiv(fdg->hPane,
	    AG_BOX_VERT, AG_BOX_HFILL,
	    AG_BOX_VERT, AG_BOX_HFILL|AG_BOX_WFILL);
	{
		fdg->tlDirs = AG_TlistNew(fdg->hDiv->box1, 0);
		fdg->tlFiles = AG_TlistNew(fdg->hDiv->box2,
		    (flags&AG_FILEDLG_MULTI) ? AG_TLIST_MULTI : 0);
	}

	fdg->lbCwd = AG_LabelNew(fdg, AG_LABEL_POLLED, _("Cwd: %s"),
	    &fdg->cwd[0]);

	fdg->tbFile = AG_TextboxNew(fdg, _("File: "));
	fdg->comTypes = AG_ComboNew(fdg, 0, _("Type: "));
	AG_TlistPrescale(fdg->tlDirs, "XXXXXXXXXXXXXX", 8);
	AG_TlistPrescale(fdg->tlFiles, "XXXXXXXXXXXXXXXXXX", 8);

	fdg->btnOk = AG_ButtonNew(fdg, _("OK"));
	fdg->btnCancel = AG_ButtonNew(fdg, _("Cancel"));

	AG_SetEvent(fdg, "widget-shown", file_dlg_shown, NULL);
	AG_SetEvent(fdg->tlDirs, "tlist-dblclick", select_dir, "%p", fdg);
	AG_SetEvent(fdg->tlFiles, "tlist-selected", select_file, "%p", fdg);
	AG_SetEvent(fdg->tlFiles, "tlist-dblclick", select_and_validate_file,
	    "%p", fdg);

	AG_SetEvent(fdg->tbFile, "textbox-return", enter_file, "%p", fdg);
	AG_SetEvent(fdg->btnOk, "button-pushed", validate_file, "%p", fdg);
	AG_SetEvent(fdg->btnCancel, "button-pushed", do_cancel, "%p", fdg);
}

void
AG_FileDlgDestroy(void *p)
{
	AG_FileDlg *fdg = p;
	AG_FileType *ft, *ft2;
	u_int i;

	for (ft = TAILQ_FIRST(&fdg->types);
	     ft != TAILQ_END(&fdg->types);
	     ft = ft2) {
		ft2 = TAILQ_NEXT(ft, types);
		for (i = 0; i < ft->nexts; i++) {
			Free(ft->exts[i], M_WIDGET);
		}
		Free(ft->exts, M_WIDGET);
		Free(ft, M_WIDGET);
	}
	AG_WidgetDestroy(fdg);
}

void
AG_FileDlgScale(void *p, int w, int h)
{
	AG_FileDlg *fdg = p;
	int btn_h, y = 0;
	
	if (w == -1 && h == -1) {
		AGWIDGET_SCALE(fdg->hPane, -1, -1);
		AGWIDGET_SCALE(fdg->lbCwd , -1, -1);
		AGWIDGET_SCALE(fdg->tbFile, -1, -1);
		AGWIDGET_SCALE(fdg->comTypes, -1, -1);
		AGWIDGET_SCALE(fdg->btnOk, -1, -1);
		AGWIDGET_SCALE(fdg->btnCancel, -1, -1);
	
		AGWIDGET(fdg)->w = AGWIDGET(fdg->hPane)->w;
		AGWIDGET(fdg)->h = AGWIDGET(fdg->hPane)->h +
				 AGWIDGET(fdg->lbCwd)->h+1 +
				 AGWIDGET(fdg->tbFile)->h+1 +
				 AGWIDGET(fdg->comTypes)->h+1 +
				 MAX(AGWIDGET(fdg->btnOk)->h,
				     AGWIDGET(fdg->btnCancel)->h)+2;
		return;
	}

	btn_h = MAX(AGWIDGET(fdg->btnOk)->h, AGWIDGET(fdg->btnCancel)->h);
	
	AG_WidgetScale(fdg->hPane,
	    w,
	    h - AGWIDGET(fdg->lbCwd)->h - AGWIDGET(fdg->tbFile)->h -
	        AGWIDGET(fdg->comTypes)->h - btn_h);

	AG_WidgetScale(fdg->lbCwd, w, AGWIDGET(fdg->lbCwd)->h);
	AG_WidgetScale(fdg->tbFile, w, AGWIDGET(fdg->tbFile)->h);
	AG_WidgetScale(fdg->comTypes, w, AGWIDGET(fdg->comTypes)->h);
	AG_WidgetScale(fdg->btnOk, w/2, AGWIDGET(fdg->tbFile)->h);
	AG_WidgetScale(fdg->btnCancel, w/2, AGWIDGET(fdg->tbFile)->h);
	
	AGWIDGET(fdg->hPane)->x = 0;
	AGWIDGET(fdg->hPane)->y = 0;
	AGWIDGET(fdg->hPane)->w = w;
	AGWIDGET(fdg->hPane)->h = h -
	    AGWIDGET(fdg->tbFile)->h -
	    AGWIDGET(fdg->lbCwd)->h -
	    AGWIDGET(fdg->comTypes)->h - btn_h - 2;
	AG_WidgetScale(fdg->hPane, AGWIDGET(fdg->hPane)->w,
	    AGWIDGET(fdg->hPane)->h);
	y += AGWIDGET(fdg->hPane)->h + 1;

	AGWIDGET(fdg->lbCwd)->x = 0;
	AGWIDGET(fdg->lbCwd)->y = y;
	y += AGWIDGET(fdg->lbCwd)->h + 1;

	AGWIDGET(fdg->tbFile)->x = 0;
	AGWIDGET(fdg->tbFile)->y = y;
	y += AGWIDGET(fdg->tbFile)->h + 1;

	AGWIDGET(fdg->comTypes)->x = 0;
	AGWIDGET(fdg->comTypes)->y = y;
	y += AGWIDGET(fdg->comTypes)->h + 1;
	
	AGWIDGET(fdg->btnOk)->x = 0;
	AGWIDGET(fdg->btnOk)->y = y;

	AGWIDGET(fdg->btnCancel)->x = w/2;
	AGWIDGET(fdg->btnCancel)->y = y;
	
	AGWIDGET(fdg)->w = w;
	AGWIDGET(fdg)->h = h;
}

AG_FileType *
AG_FileDlgAddType(AG_FileDlg *fdg, const char *descr, const char *exts,
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
	while ((ext = strsep(&ds, ",;")) != NULL) {
		ft->exts = Realloc(ft->exts, (ft->nexts+1)*sizeof(char *));
		ft->exts[ft->nexts++] = Strdup(ext);
	}
	Free(dexts, M_WIDGET);

	ft->action = AG_SetEvent(fdg, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(ft->action, fmt);

	it = AG_TlistAdd(fdg->comTypes->list, NULL, "%s (%s)", descr, exts);
	it->p1 = ft;
	if (TAILQ_EMPTY(&fdg->types))
		AG_ComboSelectPointer(fdg->comTypes, ft);

	TAILQ_INSERT_TAIL(&fdg->types, ft, types);
	return (ft);
}
