/*	$Csoft: file_dlg.c,v 1.6 2005/09/19 03:04:44 vedge Exp $	*/

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

static struct widget_ops file_dlg_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		file_dlg_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	file_dlg_scale
};

struct AGFileDlg *
file_dlg_new(void *parent, int flags, const char *cwd, const char *file)
{
	struct AGFileDlg *fdg;

	fdg = Malloc(sizeof(struct AGFileDlg), M_OBJECT);
	file_dlg_init(fdg, flags, cwd, file);
	object_attach(parent, fdg);
	return (fdg);
}

static int
compare_filenames(const void *p1, const void *p2)
{
	const struct dirent *d1 = *(const void **)p1;
	const struct dirent *d2 = *(const void **)p2;

	return (strcoll(d1->d_name, d2->d_name));
}

static void
update_listing(struct AGFileDlg *fdg)
{
	struct tlist_item *it;
	struct dirent *dent;
	struct stat sb;
	DIR *dir;
	struct dirent **dirs, **files;
	size_t i, ndirs = 0, nfiles = 0;

	if ((dir = opendir(".")) == NULL) {
		text_msg(MSG_ERROR, ".: %s", strerror(errno));
		return;
	}
	
	dirs = Malloc(sizeof(struct dirent *), M_WIDGET);
	files = Malloc(sizeof(struct dirent *), M_WIDGET);
	
	pthread_mutex_lock(&fdg->tlDirs->lock);
	pthread_mutex_lock(&fdg->tlFiles->lock);

	while ((dent = readdir(dir)) != NULL) {
		stat(dent->d_name, &sb);
		if ((sb.st_mode & S_IFDIR) == S_IFDIR) {
			dirs = Realloc(dirs, (ndirs + 1) *
			                     sizeof(struct dirent *));
			dirs[ndirs++] = dent;
		} else {
			files = Realloc(files, (nfiles + 1) *
					       sizeof(struct dirent *));
			files[nfiles++] = dent;
		}
	}
	qsort(&dirs[0], ndirs, sizeof(struct dirent *), compare_filenames);
	qsort(&files[0], nfiles, sizeof(struct dirent *), compare_filenames);

	tlist_clear_items(fdg->tlDirs);
	tlist_clear_items(fdg->tlFiles);
	for (i = 0; i < ndirs; i++) {
		struct dirent *dent = dirs[i];

		it = tlist_insert(fdg->tlDirs, NULL, "%s", dent->d_name);
		it->class = "dir";
		it->p1 = dent;
	}
	for (i = 0; i < nfiles; i++) {
		struct dirent *dent = files[i];

		it = tlist_insert(fdg->tlFiles, NULL, "%s", dent->d_name);
		it->class = "file";
		it->p1 = dent;
	}
	tlist_restore_selections(fdg->tlDirs);
	tlist_restore_selections(fdg->tlFiles);

	pthread_mutex_unlock(&fdg->tlFiles->lock);
	pthread_mutex_unlock(&fdg->tlDirs->lock);
	closedir(dir);
	Free(dirs, M_WIDGET);
	Free(files, M_WIDGET);
}

static void
select_dir(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct AGFileDlg *fdg = argv[1].p;
	struct tlist_item *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = tlist_selected_item(tl)) != NULL) {
		if (chdir(ti->text) == -1) {
			text_msg(MSG_ERROR, "%s: %s", ti->text,
			    strerror(errno));
		} else {
			event_post(NULL, fdg, "dir-selected", NULL);
			update_listing(fdg);
		}
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
process_file(struct AGFileDlg *fdg, const char *file)
{
	struct window *pwin = widget_parent_window(fdg);
	struct tlist_item *it;

	if ((fdg->flags & FILEDLG_NOCLOSE) == 0)
		view_detach(pwin);

	if ((it = tlist_selected_item(fdg->comTypes->list)) != NULL) {
		struct AGFileType *ft = it->p1;

		if (ft->action != NULL)
			event_post(NULL, fdg, ft->action->name, "%s", file);
	}
}

static void
select_file(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct AGFileDlg *fdg = argv[1].p;
	struct tlist_item *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = tlist_selected_item(tl)) != NULL) {
		textbox_printf(fdg->tbFile, "%s", ti->text);
		event_post(NULL, fdg, "file-selected", "%s/%s",
		    fdg->cwd, ti->text);
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
select_and_validate_file(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct AGFileDlg *fdg = argv[1].p;
	struct tlist_item *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = tlist_selected_item(tl)) != NULL) {
		textbox_printf(fdg->tbFile, "%s", ti->text);
		event_post(NULL, fdg, "file-validated", "%s", ti->text);
		process_file(fdg, ti->text);
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
validate_file(int argc, union evarg *argv)
{
	char file[MAXPATHLEN];
	struct AGFileDlg *fdg = argv[1].p;
	struct AGFileType *ft;

	textbox_copy_string(fdg->tbFile, file, sizeof(file));
	event_post(NULL, fdg, "file-validated", "%s", file);
	process_file(fdg, file);
}

static void
do_cancel(int argc, union evarg *argv)
{
	struct AGFileDlg *fdg = argv[1].p;
	struct window *pwin = widget_parent_window(fdg);
	
	if ((fdg->flags & FILEDLG_NOCLOSE) == 0)
		view_detach(pwin);
}

static void
file_dlg_shown(int argc, union evarg *argv)
{
	struct AGFileDlg *fdg = argv[0].p;

	widget_scale(fdg, WIDGET(fdg)->w, WIDGET(fdg)->h);
	widget_focus(fdg->tbFile);
	update_listing(fdg);
}

void
file_dlg_init(struct AGFileDlg *fdg, int flags, const char *cwd,
    const char *file)
{
	widget_init(fdg, "file-dlg", &file_dlg_ops, WIDGET_WFILL|WIDGET_HFILL);
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

	fdg->tlDirs = tlist_new(fdg, 0);
	fdg->tlFiles = tlist_new(fdg, (flags&FILEDLG_MULTI) ? TLIST_MULTI : 0);
	fdg->tbFile = textbox_new(fdg, _("File: "));
	if (file != NULL) {
		textbox_printf(fdg->tbFile, "%s", file);
	}
	fdg->comTypes = combo_new(fdg, 0, _("Type: "));
	tlist_prescale(fdg->tlDirs, "XXXXXXXXXXXXXX", 8);
	tlist_prescale(fdg->tlFiles, "XXXXXXXXXXXXXXXXXX", 8);

	fdg->btnOk = button_new(fdg, _("OK"));
	fdg->btnCancel = button_new(fdg, _("Cancel"));

	event_new(fdg, "widget-shown", file_dlg_shown, NULL);
	event_new(fdg->tlDirs, "tlist-dblclick", select_dir, "%p", fdg);
	event_new(fdg->tlFiles, "tlist-selected", select_file, "%p", fdg);
	event_new(fdg->tlFiles, "tlist-dblclick", select_and_validate_file,
	    "%p", fdg);

	event_new(fdg->tbFile, "textbox-return", validate_file, "%p", fdg);
	event_new(fdg->btnOk, "button-pushed", validate_file, "%p", fdg);
	event_new(fdg->btnCancel, "button-pushed", do_cancel, "%p", fdg);
}

void
file_dlg_destroy(void *p)
{
	struct AGFileDlg *fdg = p;
	struct AGFileType *ft, *ft2;
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
	widget_destroy(fdg);
}

void
file_dlg_scale(void *p, int w, int h)
{
	struct AGFileDlg *fdg = p;
	int btn_h, y = 0;
	
	if (w == -1 && h == -1) {
		WIDGET_SCALE(fdg->tlDirs, -1, -1);
		WIDGET_SCALE(fdg->tlFiles, -1, -1);
		WIDGET_SCALE(fdg->tbFile, -1, -1);
		WIDGET_SCALE(fdg->comTypes, -1, -1);
		WIDGET_SCALE(fdg->btnOk, -1, -1);
		WIDGET_SCALE(fdg->btnCancel, -1, -1);
	
		WIDGET(fdg)->w = WIDGET(fdg->tlDirs)->w +
		                 WIDGET(fdg->tlFiles)->w;
		WIDGET(fdg)->h = MAX(WIDGET(fdg->tlDirs)->h,
				     WIDGET(fdg->tlFiles)->h)+1 +
				 WIDGET(fdg->tbFile)->h+1 +
				 WIDGET(fdg->comTypes)->h+1 +
				 MAX(WIDGET(fdg->btnOk)->h,
				     WIDGET(fdg->btnCancel)->h)+2;
		return;
	}

	btn_h = MAX(WIDGET(fdg->btnOk)->h, WIDGET(fdg->btnCancel)->h);

	widget_scale(fdg->tlDirs,
	    WIDGET(fdg->tlDirs)->w,
	    h - WIDGET(fdg->tbFile)->h - WIDGET(fdg->comTypes)->h - btn_h);
	widget_scale(fdg->tlFiles,
	    w - WIDGET(fdg->tlDirs)->w,
	    h - WIDGET(fdg->tbFile)->h - WIDGET(fdg->comTypes)->h - btn_h);
	    
	widget_scale(fdg->tbFile, w, WIDGET(fdg->tbFile)->h);
	widget_scale(fdg->comTypes, w, WIDGET(fdg->comTypes)->h);
	widget_scale(fdg->btnOk, w/2, WIDGET(fdg->tbFile)->h);
	widget_scale(fdg->btnCancel, w/2, WIDGET(fdg->tbFile)->h);
	
	WIDGET(fdg->tlDirs)->x = 0;
	WIDGET(fdg->tlDirs)->y = 0;
	WIDGET(fdg->tlFiles)->x = WIDGET(fdg->tlDirs)->w;
	WIDGET(fdg->tlFiles)->y = 0;
	y += WIDGET(fdg->tlFiles)->h + 1;

	WIDGET(fdg->tbFile)->x = 0;
	WIDGET(fdg->tbFile)->y = y;
	y += WIDGET(fdg->tbFile)->h + 1;

	WIDGET(fdg->comTypes)->x = 0;
	WIDGET(fdg->comTypes)->y = y;
	y += WIDGET(fdg->comTypes)->h + 1;
	
	WIDGET(fdg->btnOk)->x = 0;
	WIDGET(fdg->btnOk)->y = y;

	WIDGET(fdg->btnCancel)->x = w/2;
	WIDGET(fdg->btnCancel)->y = y;
	
	WIDGET(fdg)->w = w;
	WIDGET(fdg)->h = h;
}

struct AGFileType *
file_dlg_type(struct AGFileDlg *fdg, const char *descr, const char *exts,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	struct AGFileType *ft;
	char *dexts, *ds, *ext;
	va_list ap;
	struct tlist_item *it;

	ft = Malloc(sizeof(struct AGFileType), M_WIDGET);
	ft->descr = descr;
	ft->exts = Malloc(sizeof(char *), M_WIDGET);
	ft->nexts = 0;

	ds = dexts = Strdup(exts);
	while ((ext = strsep(&ds, ",;")) != NULL) {
		ft->exts = Realloc(ft->exts, (ft->nexts+1)*sizeof(char *));
		ft->exts[ft->nexts++] = Strdup(ext);
	}
	Free(dexts, M_WIDGET);

	ft->action = event_new(fdg, NULL, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, ft->action);
		}
		va_end(ap);
	}

	it = tlist_insert(fdg->comTypes->list, NULL, "%s (%s)", descr, exts);
	it->p1 = ft;
	if (TAILQ_EMPTY(&fdg->types))
		combo_select_pointer(fdg->comTypes, ft);

	TAILQ_INSERT_TAIL(&fdg->types, ft, types);
	return (ft);
}
