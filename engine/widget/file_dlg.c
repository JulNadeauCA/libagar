/*	$Csoft: file_dlg.c,v 1.4 2005/05/24 08:15:11 vedge Exp $	*/

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

static void
update_listing(struct AGFileDlg *fdg)
{
	struct tlist_item *it;
	struct dirent *dent;
	struct stat sb;
	DIR *dir;

	if ((dir = opendir(".")) == NULL) {
		text_msg(MSG_ERROR, ".: %s", strerror(errno));
		return;
	}
	
	pthread_mutex_lock(&fdg->tl_dirs->lock);
	pthread_mutex_lock(&fdg->tl_files->lock);
	tlist_clear_items(fdg->tl_dirs);
	tlist_clear_items(fdg->tl_files);
	while ((dent = readdir(dir)) != NULL) {
		stat(dent->d_name, &sb);

		if ((sb.st_mode & S_IFDIR) == S_IFDIR) {
			it = tlist_insert(fdg->tl_dirs, NULL, "%s",
			    dent->d_name);
			it->class = "dir";
			it->p1 = dent;
		} else {
			it = tlist_insert(fdg->tl_files, NULL, "%s",
			    dent->d_name);
			it->class = "file";
			it->p1 = dent;
		}
	}
	tlist_restore_selections(fdg->tl_dirs);
	tlist_restore_selections(fdg->tl_files);
	pthread_mutex_unlock(&fdg->tl_files->lock);
	pthread_mutex_unlock(&fdg->tl_dirs->lock);
	
	closedir(dir);
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
select_file(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct AGFileDlg *fdg = argv[1].p;
	struct tlist_item *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = tlist_selected_item(tl)) != NULL) {
		textbox_printf(fdg->tb_file, "%s", ti->text);
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
		textbox_printf(fdg->tb_file, "%s", ti->text);
		event_post(NULL, fdg, "file-validated", "%s", ti->text);
	}
	pthread_mutex_unlock(&tl->lock);
}

static void
validate_file(int argc, union evarg *argv)
{
	char file[MAXPATHLEN];
	struct AGFileDlg *fdg = argv[1].p;

	textbox_copy_string(fdg->tb_file, file, sizeof(file));
	event_post(NULL, fdg, "file-validated", "%s", file);
}

static void
do_cancel(int argc, union evarg *argv)
{
	struct AGFileDlg *fdg = argv[1].p;

	event_post(NULL, fdg, "file-cancelled", NULL);
}

static void
file_dlg_shown(int argc, union evarg *argv)
{
	struct AGFileDlg *fdg = argv[0].p;

	widget_scale(fdg, WIDGET(fdg)->w, WIDGET(fdg)->h);
	widget_focus(fdg->tb_file);
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

	fdg->tl_dirs = tlist_new(fdg, 0);
	fdg->tl_files = tlist_new(fdg, (flags&FILEDLG_MULTI) ? TLIST_MULTI : 0);
	fdg->tb_file = textbox_new(fdg, _("File: "));
	if (file != NULL)
		textbox_printf(fdg->tb_file, "%s", file);

	tlist_prescale(fdg->tl_dirs, "XXXXXXXXXXXXXX", 8);
	tlist_prescale(fdg->tl_files, "XXXXXXXXXXXXXXXXXX", 8);

	fdg->btn_ok = button_new(fdg, _("OK"));
	fdg->btn_cancel = button_new(fdg, _("Cancel"));

	event_new(fdg, "widget-shown", file_dlg_shown, NULL);
	event_new(fdg->tl_dirs, "tlist-dblclick", select_dir, "%p", fdg);
	event_new(fdg->tl_files, "tlist-selected", select_file, "%p", fdg);
	event_new(fdg->tl_files, "tlist-dblclick", select_and_validate_file,
	    "%p", fdg);

	event_new(fdg->tb_file, "textbox-return", validate_file, "%p", fdg);
	event_new(fdg->btn_ok, "button-pushed", validate_file, "%p", fdg);
	event_new(fdg->btn_cancel, "button-pushed", do_cancel, "%p", fdg);
}

void
file_dlg_destroy(void *p)
{
	struct AGFileDlg *fdg = p;

	widget_destroy(fdg);
}

void
file_dlg_scale(void *p, int w, int h)
{
	struct AGFileDlg *fdg = p;
	int btn_h;
	
	if (w == -1 && h == -1) {
		WIDGET_SCALE(fdg->tl_dirs, -1, -1);
		WIDGET_SCALE(fdg->tl_files, -1, -1);
		WIDGET_SCALE(fdg->tb_file, -1, -1);
		WIDGET_SCALE(fdg->btn_ok, -1, -1);
		WIDGET_SCALE(fdg->btn_cancel, -1, -1);
	
		WIDGET(fdg)->w = WIDGET(fdg->tl_dirs)->w +
		                 WIDGET(fdg->tl_files)->w;
		WIDGET(fdg)->h = MAX(WIDGET(fdg->tl_dirs)->h,
				     WIDGET(fdg->tl_files)->h) +
				 WIDGET(fdg->tb_file)->h +
				 MAX(WIDGET(fdg->btn_ok)->h,
				     WIDGET(fdg->btn_cancel)->h);
		return;
	}

	btn_h = MAX(WIDGET(fdg->btn_ok)->h, WIDGET(fdg->btn_cancel)->h);

	widget_scale(fdg->tl_dirs,
	    WIDGET(fdg->tl_dirs)->w,
	    h - WIDGET(fdg->tb_file)->h - btn_h);
	widget_scale(fdg->tl_files,
	    w - WIDGET(fdg->tl_dirs)->w,
	    h - WIDGET(fdg->tb_file)->h - btn_h);
	    
	widget_scale(fdg->tb_file, w, WIDGET(fdg->tb_file)->h);
	widget_scale(fdg->btn_ok, w/2, WIDGET(fdg->tb_file)->h);
	widget_scale(fdg->btn_cancel, w/2, WIDGET(fdg->tb_file)->h);
	
	WIDGET(fdg->tl_dirs)->x = 0;
	WIDGET(fdg->tl_dirs)->y = 0;
	WIDGET(fdg->tl_files)->x = WIDGET(fdg->tl_dirs)->w;
	WIDGET(fdg->tl_files)->y = 0;

	WIDGET(fdg->tb_file)->x = 0;
	WIDGET(fdg->tb_file)->y = WIDGET(fdg->tl_files)->h;
	
	WIDGET(fdg->btn_ok)->x = 0;
	WIDGET(fdg->btn_ok)->y = WIDGET(fdg->tl_dirs)->h +
				 WIDGET(fdg->tb_file)->h;

	WIDGET(fdg->btn_cancel)->x = w/2;
	WIDGET(fdg->btn_cancel)->y = WIDGET(fdg->tl_files)->h +
			 	     WIDGET(fdg->tb_file)->h;
	
	WIDGET(fdg)->w = w;
	WIDGET(fdg)->h = h;
}


