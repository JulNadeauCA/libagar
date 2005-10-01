/*	$Csoft: file_dlg.c,v 1.10 2005/09/27 00:25:22 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>

#include "file_dlg.h"

#include <sys/stat.h>
#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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
AG_FileDlgNew(void *parent, int flags, const char *cwd, const char *file)
{
	AG_FileDlg *fdg;

	fdg = Malloc(sizeof(AG_FileDlg), M_OBJECT);
	AG_FileDlgInit(fdg, flags, cwd, file);
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
	struct dirent *dent;
	struct stat sb;
	DIR *dir;
	char **dirs, **files;
	size_t i, ndirs = 0, nfiles = 0;

	if ((dir = opendir(".")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, ".: %s", strerror(errno));
		return;
	}
	
	dirs = Malloc(sizeof(char *), M_WIDGET);
	files = Malloc(sizeof(char *), M_WIDGET);
	
	pthread_mutex_lock(&fdg->tlDirs->lock);
	pthread_mutex_lock(&fdg->tlFiles->lock);

	while ((dent = readdir(dir)) != NULL) {
		if (stat(dent->d_name, &sb) == -1) {
			continue;
		}
		if ((sb.st_mode & S_IFDIR) == S_IFDIR) {
			dirs = Realloc(dirs, (ndirs + 1) * sizeof(char *));
			dirs[ndirs++] = Strdup(dent->d_name);
		} else {
			files = Realloc(files, (nfiles + 1) * sizeof(char *));
			files[nfiles++] = Strdup(dent->d_name);
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
	
	pthread_mutex_unlock(&fdg->tlFiles->lock);
	pthread_mutex_unlock(&fdg->tlDirs->lock);
	closedir(dir);
}

static void
select_dir(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_FileDlg *fdg = argv[1].p;
	AG_TlistItem *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		if (chdir(ti->text) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s: %s", ti->text,
			    strerror(errno));
		} else {
			AG_PostEvent(NULL, fdg, "dir-selected", NULL);
			update_listing(fdg);
		}
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
process_file(AG_FileDlg *fdg, const char *file)
{
	AG_Window *pwin = AG_WidgetParentWindow(fdg);
	AG_TlistItem *it;

	if ((fdg->flags & AG_FILEDLG_NOCLOSE) == 0)
		AG_ViewDetach(pwin);

	if ((it = AG_TlistSelectedItem(fdg->comTypes->list)) != NULL) {
		AG_FileType *ft = it->p1;

		if (ft->action != NULL)
			AG_PostEvent(NULL, fdg, ft->action->name, "%s", file);
	}
}

static void
select_file(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_FileDlg *fdg = argv[1].p;
	AG_TlistItem *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_TextboxPrintf(fdg->tbFile, "%s", ti->text);
		AG_PostEvent(NULL, fdg, "file-selected", "%s/%s",
		    fdg->cwd, ti->text);
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
select_and_validate_file(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_FileDlg *fdg = argv[1].p;
	AG_TlistItem *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_TextboxPrintf(fdg->tbFile, "%s", ti->text);
		AG_PostEvent(NULL, fdg, "file-validated", "%s", ti->text);
		process_file(fdg, ti->text);
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
validate_file(int argc, union evarg *argv)
{
	char file[MAXPATHLEN];
	AG_FileDlg *fdg = argv[1].p;
	AG_FileType *ft;

	AG_TextboxCopyString(fdg->tbFile, file, sizeof(file));
	AG_PostEvent(NULL, fdg, "file-validated", "%s", file);
	process_file(fdg, file);
}

static void
enter_file(int argc, union evarg *argv)
{
	char file[MAXPATHLEN];
	AG_FileDlg *fdg = argv[1].p;
	AG_FileType *ft;
	struct stat sb;

	AG_TextboxCopyString(fdg->tbFile, file, sizeof(file));
	if (stat(file, &sb) == -1) {
		goto fail;
	}
	if ((sb.st_mode & S_IFDIR) == S_IFDIR) {
		if (chdir(file) == 0) {
			update_listing(fdg);
		} else {
			goto fail;
		}
	} else {
		AG_PostEvent(NULL, fdg, "file-validated", "%s", file);
		process_file(fdg, file);
	}
	return;
fail:
	AG_TextMsg(AG_MSG_ERROR, "%s: %s", file, strerror(errno));
}

static void
do_cancel(int argc, union evarg *argv)
{
	AG_FileDlg *fdg = argv[1].p;
	AG_Window *pwin = AG_WidgetParentWindow(fdg);
	
	if ((fdg->flags & AG_FILEDLG_NOCLOSE) == 0)
		AG_ViewDetach(pwin);
}

static void
file_dlg_shown(int argc, union evarg *argv)
{
	AG_FileDlg *fdg = argv[0].p;

	AG_WidgetScale(fdg, AGWIDGET(fdg)->w, AGWIDGET(fdg)->h);
	AG_WidgetFocus(fdg->tbFile);
	update_listing(fdg);
}

void
AG_FileDlgInit(AG_FileDlg *fdg, int flags, const char *cwd,
    const char *file)
{
	AG_WidgetInit(fdg, "file-dlg", &agFileDlgOps,
	    AG_WIDGET_WFILL|AG_WIDGET_HFILL);
	fdg->flags = flags;
	if (cwd != NULL) {
		strlcpy(fdg->cwd, cwd, sizeof(fdg->cwd));
	} else {
		if ((getcwd(fdg->cwd, sizeof(fdg->cwd))) == NULL) {
			fdg->cwd[0] = '/';
			fdg->cwd[1] = '\0';
		}
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

	fdg->tbFile = AG_TextboxNew(fdg, _("File: "));
	if (file != NULL) {
		AG_TextboxPrintf(fdg->tbFile, "%s", file);
	}
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
		AGWIDGET_SCALE(fdg->tbFile, -1, -1);
		AGWIDGET_SCALE(fdg->comTypes, -1, -1);
		AGWIDGET_SCALE(fdg->btnOk, -1, -1);
		AGWIDGET_SCALE(fdg->btnCancel, -1, -1);
	
		AGWIDGET(fdg)->w = AGWIDGET(fdg->hPane)->w;
		AGWIDGET(fdg)->h = AGWIDGET(fdg->hPane)->h +
				 AGWIDGET(fdg->tbFile)->h+1 +
				 AGWIDGET(fdg->comTypes)->h+1 +
				 MAX(AGWIDGET(fdg->btnOk)->h,
				     AGWIDGET(fdg->btnCancel)->h)+2;
		return;
	}

	btn_h = MAX(AGWIDGET(fdg->btnOk)->h, AGWIDGET(fdg->btnCancel)->h);
	
	AG_WidgetScale(fdg->hPane,
	    w,
	    h - AGWIDGET(fdg->tbFile)->h - AGWIDGET(fdg->comTypes)->h - btn_h);

	AG_WidgetScale(fdg->tbFile, w, AGWIDGET(fdg->tbFile)->h);
	AG_WidgetScale(fdg->comTypes, w, AGWIDGET(fdg->comTypes)->h);
	AG_WidgetScale(fdg->btnOk, w/2, AGWIDGET(fdg->tbFile)->h);
	AG_WidgetScale(fdg->btnCancel, w/2, AGWIDGET(fdg->tbFile)->h);
	
	AGWIDGET(fdg->hPane)->x = 0;
	AGWIDGET(fdg->hPane)->y = 0;
	AGWIDGET(fdg->hPane)->w = w;
	AGWIDGET(fdg->hPane)->h = h - AGWIDGET(fdg->tbFile)->h -
	    AGWIDGET(fdg->comTypes)->h - btn_h - 2;
	AG_WidgetScale(fdg->hPane, AGWIDGET(fdg->hPane)->w,
	    AGWIDGET(fdg->hPane)->h);
	y += AGWIDGET(fdg->hPane)->h + 1;

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
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	AG_FileType *ft;
	char *dexts, *ds, *ext;
	va_list ap;
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
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, ft->action);
		}
		va_end(ap);
	}

	it = AG_TlistAdd(fdg->comTypes->list, NULL, "%s (%s)", descr, exts);
	it->p1 = ft;
	if (TAILQ_EMPTY(&fdg->types))
		AG_ComboSelectPointer(fdg->comTypes, ft);

	TAILQ_INSERT_TAIL(&fdg->types, ft, types);
	return (ft);
}
