/*	$Csoft: view_params.c,v 1.5 2002/12/31 01:20:24 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <engine/world.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/textbox.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static struct label *cliprect_label;

static void
apply(int argc, union evarg *argv)
{
	struct textbox *mindelay_tbox = argv[1].p;
	struct textbox *maxdelay_tbox = argv[2].p;

	view_set_refresh(textbox_int(mindelay_tbox),
	    textbox_int(maxdelay_tbox));
}

struct window *
view_params_window(void)
{
	struct window *win;
	struct region *reg;

	if ((win = window_generic_new(289, 339, "monitor-view-params"))
	    == NULL) {
		return (NULL);	/* Exists */
	}
	window_set_caption(win, "View parameters");
	
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		struct textbox *mindelay_tbox, *maxdelay_tbox;
		struct button *button;
		char *engine = "???";

		switch (view->gfx_engine) {
		case GFX_ENGINE_GUI:
			engine = "GUI";
			break;
		case GFX_ENGINE_TILEBASED:
			engine = "Tile-based";
			break;
		}
		label_new(reg, 100, 0, "Graphic engine: %s %s", engine,
		    view->opengl ? "(OpenGL)" : "");
		label_polled_new(reg, 100, 0, &view->lock, "Depth: %dbpp",
		    &view->depth);
		label_polled_new(reg, 100, 0, &view->lock, "Geometry: %dx%d",
		    &view->w, &view->h);
		label_polled_new(reg, 100, 0, &view->lock, "Window op: %d (%p)",
		    &view->winop, &view->wop_win);
		label_polled_new(reg, 100, 0, NULL,
		    "Clipping: [%[u16]x%[u16] at %[s16],%[s16]]",
		    &view->v->clip_rect.w,
		    &view->v->clip_rect.h,
		    &view->v->clip_rect.x,
		    &view->v->clip_rect.y);

		if (view->rootmap != NULL) {
			struct viewmap *rm = view->rootmap;
		
			label_polled_new(reg, 100, 0, &view->lock,
			    "Root map: %p", &rm->map);
			label_new(reg, 100, 0,
			    "Root map geometry: %dx%d", rm->w, rm->h);
			label_polled_new(reg, 100, 0, &view->lock,
			    "Root map offset: %d,%d (soft %d,%d)",
			    &rm->x, &rm->y, &rm->sx, &rm->sy);
		}
		
		mindelay_tbox = textbox_new(reg, "Minimum refresh delay (ms): ",
		    0, 100, 10);
		maxdelay_tbox = textbox_new(reg, "Maximum refresh delay (ms): ",
		    0, 100, 10);
		
		textbox_printf(mindelay_tbox, "%d", view->refresh.min_delay);
		textbox_printf(maxdelay_tbox, "%d", view->refresh.max_delay);

		button = button_new(reg, "Apply", NULL, 0, 100, 10);
		event_new(button, "button-pushed", apply, "%p, %p",
		    mindelay_tbox, maxdelay_tbox);
	}
	
	return (win);
}

#endif	/* DEBUG */
