/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * This displays various parameters related to the display.
 */

#include <core/core.h>

#include <gui/window.h>
#include <gui/vbox.h>
#include <gui/button.h>
#include <gui/tlist.h>
#include <gui/label.h>
#include <gui/textbox.h>

#include "dev.h"

AG_Window *
DEV_DisplaySettings(void)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Label *lbl;

	if ((win = AG_WindowNewNamedS(AG_WINDOW_NORESIZE,
	    "DEV_DisplaySettings")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Display Settings"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	vb = AG_VBoxNew(win, 0);
	{
		AG_LabelNew(vb, 0, _("OpenGL mode: %s"),
		    agView->opengl ? _("yes") : _("no"));

		/* XXX thread unsafe */
		lbl = AG_LabelNewPolled(vb, AG_LABEL_HFILL, "%dx%d",
		    &agView->w, &agView->h);

#if 0
		AG_LabelNew(vb, 0, _("Depth: %dbpp"),
		    (int)agVideoInfo->vfmt->BitsPerPixel);
		AG_LabelNew(vb, 0, _("Video masks: %08x,%08x,%08x"),
		    (Uint)agVideoInfo->vfmt->Rmask,
		    (Uint)agVideoInfo->vfmt->Gmask,
		    (Uint)agVideoInfo->vfmt->Bmask);
		AG_LabelNew(vb, 0, _("Color key: 0x%x"),
		    (Uint)agVideoInfo->vfmt->colorkey);
		AG_LabelNew(vb, 0, _("Alpha: %d"),
		    agVideoInfo->vfmt->alpha);
#endif

		/* XXX thread unsafe */
		lbl = AG_LabelNewPolled(vb, AG_LABEL_HFILL,
		    _("Window op: %d (%p)"),
		    &agView->winop, &agView->winSelected);
		AG_LabelSizeHint(lbl, 1, _("Window op: 000 (0x00000000)"));
	
		/* XXX thread unsafe */
		lbl = AG_LabelNewPolled(vb, AG_LABEL_HFILL,
		    _("Refresh rate (effective): %d"), &agView->rCur);
		AG_LabelSizeHint(lbl, 1,
		    _("Refresh rate (effective): 000"));

		lbl = AG_LabelNewPolled(vb, AG_LABEL_HFILL,
		    _("Refresh rate (nominal): %d"), &agView->rNom);
		AG_LabelSizeHint(lbl, 1,
		    _("Refresh rate (nominal): 000"));
	}
	return (win);
}
