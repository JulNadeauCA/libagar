/*	$Csoft: view_params.c,v 1.26 2005/09/27 00:25:19 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef DEBUG

#include <engine/view.h>
#include <engine/map/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/textbox.h>

#include "monitor.h"

AG_Window *
AG_DebugViewSettings(void)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Label *lab;

	if ((win = AG_WindowNew(AG_WINDOW_DETACH|AG_WINDOW_NO_RESIZE,
	    "monitor-view-params")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Viewport"));
	
	vb = AG_VBoxNew(win, 0);
	{
		AG_LabelNew(vb, AG_LABEL_STATIC, _("OpenGL mode: %s"),
		    agView->opengl ? _("yes") : _("no"));

		lab = AG_LabelNew(vb, AG_LABEL_POLLED_MT, "%dx%d",
		    &agView->lock, &agView->w, &agView->h);
		AG_LabelPrescale(lab, "0000x0000x00");

		AG_LabelNew(vb, AG_LABEL_STATIC, _("Depth: %dbpp"),
		    agVideoInfo->vfmt->BitsPerPixel);
		AG_LabelNew(vb, AG_LABEL_STATIC,
		    _("Video masks: %08x,%08x,%08x"),
		    agVideoInfo->vfmt->Rmask,
		    agVideoInfo->vfmt->Gmask,
		    agVideoInfo->vfmt->Bmask);
		AG_LabelNew(vb, AG_LABEL_STATIC, _("Color key: %d"),
		    agVideoInfo->vfmt->colorkey);
		AG_LabelNew(vb, AG_LABEL_STATIC, _("Alpha: %d"),
		    agVideoInfo->vfmt->alpha);

		lab = AG_LabelNew(vb, AG_LABEL_POLLED_MT,
		    _("Window op: %d (%p)"),
		    &agView->lock, &agView->winop, &agView->wop_win);
		AG_LabelPrescale(lab,
		    _("Window op: 000 (0x00000000)"));
		
		lab = AG_LabelNew(vb, AG_LABEL_POLLED_MT,
		    _("Refresh rate (effective): %d"),
		    &agView->lock, &agView->rCur);
		lab = AG_LabelNew(vb, AG_LABEL_POLLED_MT,
		    _("Refresh rate (nominal): %d"),
		    &agView->lock, &agView->rNom);
	}
	return (win);
}

#endif	/* DEBUG */
