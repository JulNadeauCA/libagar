/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "file_selector.h"

AG_FileSelector *
AG_FileSelectorNew(void *parent, Uint flags, const char *label)
{
	AG_FileSelector *fs;

	fs = Malloc(sizeof(AG_FileSelector));
	AG_ObjectInit(fs, &agFileSelectorClass);
	fs->flags |= flags;

	if (label != NULL) {
		AG_TextboxSetLabelS(fs->tbox, label);
	}
	if (flags & AG_FILE_SELECTOR_HFILL) { AG_ExpandHoriz(fs); }
	if (flags & AG_FILE_SELECTOR_VFILL) { AG_ExpandVert(fs); }
	
	AG_ObjectAttach(parent, fs);
	return (fs);
}

static void
Collapse(AG_FileSelector *fs)
{
	if (fs->panel == NULL) {
		return;
	}
	fs->wSaved = WIDTH(fs->panel);
	fs->hSaved = HEIGHT(fs->panel);

	AG_WindowHide(fs->panel);
	AG_ObjectDetach(fs->filedlg);
	AG_ObjectDetach(fs->panel);
	fs->panel = NULL;

	AG_SetInt(fs->button, "state", 0);
}

static void
ModalClose(AG_Event *event)
{
	AG_FileSelector *fs = AG_PTR(1);

	if (fs->panel != NULL)
		Collapse(fs);
}

static void
Expand(AG_Event *event)
{
	AG_FileSelector *fs = AG_PTR(1);
	AG_Driver *drv = WIDGET(fs)->drv;
	int expand = AG_INT(2);
	AG_SizeReq rFileDlg;
	int x, y, w, h;
	Uint wView, hView;

	if (expand) {						/* Expand */
		fs->panel = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE);
		AG_WindowSetPadding(fs->panel, 0, 0, 0, 0);
		AG_ObjectAttach(fs->panel, fs->filedlg);
		
		if (fs->wSaved > 0) {
			w = fs->wSaved;
			h = fs->hSaved;
		} else {
			AG_WidgetSizeReq(fs->filedlg, &rFileDlg);
			w = rFileDlg.w + fs->panel->wBorderSide*2;
			h = rFileDlg.h + fs->panel->wBorderBot;
 		}
		x = WIDGET(fs)->rView.x2 - w;
		y = WIDGET(fs)->rView.y1;

		if (AGDRIVER_SINGLE(drv) &&
		    AG_GetDisplaySize(drv, &wView, &hView) == 0) {
			if (x+w > wView) { w = wView - x; }
			if (y+h > hView) { h = hView - y; }
		}
		if (w < 4 || h < 4) {
			Collapse(fs);
			return;
		}
		AG_SetEvent(fs->panel, "window-modal-close",
		    ModalClose, "%p", fs);
		AG_WindowSetGeometry(fs->panel, x, y, w, h);
		AG_WindowShow(fs->panel);
	} else {
		Collapse(fs);
	}
}

static void
SetDirectoryAndFile(AG_FileSelector *fs, const char *pPath)
{
	char path[AG_FILENAME_MAX], *file;
	
	Strlcpy(path, pPath, sizeof(path));
	if ((file = strrchr(path, AG_PATHSEPCHAR)) != NULL) {
		AG_FileDlgSetFilenameS(fs->filedlg, file);
		*file = '\0';
		AG_FileDlgSetDirectoryS(fs->filedlg, path);
	}
}

static void
SetDirectory(AG_FileSelector *fs, const char *pPath)
{
	char path[AG_FILENAME_MAX];
	
	Strlcpy(path, pPath, sizeof(path));
	AG_FileDlgSetDirectoryS(fs->filedlg, path);
}

void
AG_FileSelectorSetFile(AG_FileSelector *fs, const char *path)
{
	AG_ObjectLock(fs->filedlg);
	SetDirectoryAndFile(fs, path);
	AG_TextboxSetString(fs->tbox, path);
	AG_ObjectUnlock(fs->filedlg);
}

void
AG_FileSelectorSetDirectory(AG_FileSelector *fs, const char *path)
{
	char dir[AG_PATHNAME_MAX];

	Strlcpy(dir, path, sizeof(dir));
	if (dir[0] != '\0' && dir[strlen(dir)-1] != AG_PATHSEPCHAR) {
		dir[strlen(dir)-1] = AG_PATHSEPCHAR;
	}
	AG_ObjectLock(fs->filedlg);
	SetDirectory(fs, dir);
	AG_TextboxSetString(fs->tbox, dir);
	AG_ObjectUnlock(fs->filedlg);
}

static void
FileChosen(AG_Event *event)
{
	AG_FileSelector *fs = AG_PTR(1);
	char *path = AG_STRING(2);

	AG_TextboxSetString(fs->tbox, path);
	AG_PostEvent(NULL, fs, "file-chosen", "%s", path);
	Collapse(fs);
}

static void
Return(AG_Event *event)
{
	char path[AG_TEXTBOX_STRING_MAX];
	AG_Textbox *tbox = AG_SELF();
	AG_FileSelector *fs = AG_PTR(1);
	
	AG_ObjectLock(fs->filedlg);
	AG_TextboxCopyString(tbox, path, sizeof(path));
	/* XXX TODO: Check access, AG_FILE_SELECTOR_ANY_FILE */
	SetDirectoryAndFile(fs, path);
	AG_PostEvent(NULL, fs, "file-chosen", "%s", path);
	AG_ObjectUnlock(fs->filedlg);
}

static void
Init(void *obj)
{
	AG_FileSelector *fs = obj;

	fs->flags = 0;
	fs->panel = NULL;
	fs->wSaved = 0;
	fs->hSaved = 0;
	
	fs->tbox = AG_TextboxNewS(fs, AG_TEXTBOX_COMBO|AG_TEXTBOX_STATIC, NULL);
	fs->button = AG_ButtonNewS(fs, AG_BUTTON_STICKY, _("Browse..."));
	AG_ButtonSetPadding(fs->button, 1,1,1,1);
	AG_WidgetSetFocusable(fs->button, 0);

	fs->filedlg = Malloc(sizeof(AG_FileDlg));
	AG_ObjectInit(fs->filedlg, &agFileDlgClass);
	AG_Expand(fs->filedlg);
	
	AG_SetEvent(fs->button, "button-pushed", Expand, "%p", fs);
	AG_SetEvent(fs->filedlg, "file-chosen", FileChosen, "%p", fs);
	AG_SetEvent(fs->tbox, "textbox-return", Return, "%p", fs);
}

static void
Destroy(void *p)
{
	AG_FileSelector *fs = p;

	if (fs ->panel != NULL) {
		AG_WindowHide(fs->panel);
		AG_ObjectDetach(fs->filedlg);
		AG_ObjectDetach(fs->panel);
	}
	AG_ObjectDestroy(fs->filedlg);
}

static void
Draw(void *obj)
{
	AG_Widget *chld;

	WIDGET_FOREACH_CHILD(chld, obj)
		AG_WidgetDraw(chld);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_FileSelector *fs = obj;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(fs->tbox, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	AG_WidgetSizeReq(fs->button, &rChld);
	r->w += rChld.w;
	if (r->h < rChld.h) { r->h = rChld.h; }
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_FileSelector *fs = obj;
	AG_SizeReq rBtn;
	AG_SizeAlloc aChld;

	AG_WidgetSizeReq(fs->button, &rBtn);
	if (a->w < rBtn.w) {
		return (-1);
	}
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w - rBtn.w - 1;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(fs->tbox, &aChld);
	aChld.x = aChld.w + 1;
	aChld.w = rBtn.w;
	AG_WidgetSizeAlloc(fs->button, &aChld);
	return (0);
}

AG_WidgetClass agFileSelectorClass = {
	{
		"Agar(Widget:FileSelector)",
		sizeof(AG_FileSelector),
		{ 0,0 },
		Init,
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
