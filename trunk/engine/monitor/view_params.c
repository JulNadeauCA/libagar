/*	$Csoft: view_params.c,v 1.16 2003/07/28 15:29:59 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/textbox.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

struct window *
view_params_window(void)
{
	struct window *win;
	struct vbox *vb;
	struct label *lab;

	if ((win = window_new("monitor-view-params")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Viewport"));
	window_set_closure(win, WINDOW_DETACH);
	
	vb = vbox_new(win, 0);
	{
		char *engine = "";

		switch (view->gfx_engine) {
		case GFX_ENGINE_GUI:
			engine = _("GUI");
			break;
		case GFX_ENGINE_TILEBASED:
			engine = _("Tile-based");
			break;
		}
		label_new(vb, _("Graphic engine: %s %s"), engine,
		    view->opengl ? "(OpenGL)" : "");
		lab = label_polled_new(vb, &view->lock, "%dx%dx%d", &view->w,
		    &view->h, &view->depth);
		label_prescale(lab, "0000x0000x00");
		lab = label_polled_new(vb, &view->lock,
		    _("Window op: %d (%p)"),
		    &view->winop, &view->wop_win);
		label_prescale(lab,
		    _("Window op: 000 (0x00000000)"));

		if (view->rootmap != NULL) {
			struct viewmap *rm = view->rootmap;
		
			label_polled_new(vb, &view->lock,
			    _("Map: %[obj]"), &rm->map);
			label_new(vb,
			    _("Map geometry: %dx%d"), rm->w, rm->h);
			label_polled_new(vb, &view->lock,
			    _("Map offset: %d,%d (soft %d,%d)"),
			    &rm->x, &rm->y, &rm->sx, &rm->sy);
		}
	}
	return (win);
}

#endif	/* DEBUG */
