/*	$Csoft: vgobj.c,v 1.6 2004/04/23 03:38:40 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mfspinbutton.h>
#include <engine/widget/palette.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/combo.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "vgobj.h"

const struct version vgobj_ver = {
	"agar vgobj",
	0, 0
};

const struct object_ops vgobj_ops = {
	vgobj_init,
	NULL,
	vgobj_destroy,
	vgobj_load,
	vgobj_save,
	vgobj_edit
};

void
vgobj_init(void *p, const char *name)
{
	struct vgobj *vgo = p;

	object_init(vgo, "vgobj", name, &vgobj_ops);
	vgo->vg = vg_new(vgo, VG_VISORIGIN|VG_VISGRID);
	vg_scale(vgo->vg, 8.5, 11, 1);
}

void
vgobj_destroy(void *p)
{
	struct vgobj *vgo = p;

	vg_destroy(vgo->vg);
}

int
vgobj_load(void *p, struct netbuf *buf)
{
	struct vgobj *vgo = p;

	if (version_read(buf, &vgobj_ver, NULL) != 0)
		return (-1);

	return (0);
}

int
vgobj_save(void *p, struct netbuf *buf)
{
	struct vgobj *vgo = p;

	version_write(buf, &vgobj_ver);
	return (0);
}

static void
vgobj_settings(int argc, union evarg *argv)
{
	struct window *pwin = argv[1].p;
	struct vgobj *vgo = argv[2].p;
	struct vg *vg = vgo->vg;
	struct window *win;
	struct mfspinbutton *mfsu;
	struct fspinbutton *fsu;
	struct palette *pal;
	
	win = window_new(NULL);
	window_set_caption(win, _("Parameters for \"%s\""), OBJECT(vgo)->name);
	window_set_closure(win, WINDOW_DETACH);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	mfsu = mfspinbutton_new(win, NULL, "x", _("Geometry: "));
	widget_bind(mfsu, "xvalue", WIDGET_DOUBLE, &vg->w);
	widget_bind(mfsu, "yvalue", WIDGET_DOUBLE, &vg->h);
	mfspinbutton_set_min(mfsu, 1.0);
	mfspinbutton_set_increment(mfsu, 0.1);
	event_new(mfsu, "mfspinbutton-changed", vg_geo_changed, "%p", vg);

	fsu = fspinbutton_new(win, NULL, _("Grid interval: "));
	widget_bind(fsu, "value", WIDGET_DOUBLE, &vg->grid_gap);
	fspinbutton_set_min(fsu, 0.0625);
	fspinbutton_set_increment(fsu, 0.0625);
	event_new(fsu, "fspinbutton-changed", vg_changed, "%p", vg);
	
	fsu = fspinbutton_new(win, NULL, _("Scaling factor: "));
	widget_bind(fsu, "value", WIDGET_DOUBLE, &vg->scale);
	fspinbutton_set_min(fsu, 0.1);
	fspinbutton_set_increment(fsu, 0.1);
	event_new(fsu, "fspinbutton-changed", vg_geo_changed, "%p", vg);
		
	label_new(win, LABEL_STATIC, _("Background color: "));
	pal = palette_new(win, PALETTE_RGB);
	widget_bind(pal, "color", WIDGET_UINT32, &vg->fill_color);
	event_new(pal, "palette-changed", vg_changed, "%p", vg);
	
	label_new(win, LABEL_STATIC, _("Grid color: "));
	pal = palette_new(win, PALETTE_RGB);
	widget_bind(pal, "color", WIDGET_UINT32, &vg->grid_color);
	event_new(pal, "palette-changed", vg_changed, "%p", vg);

	window_attach(pwin, win);
	window_show(win);
}

static void
rasterize_vgobj(struct mapview *mv, void *p)
{
	struct vg *vg = p;

	if (vg->redraw) {
		vg->redraw = 0;
		vg_clear(vg);
		vg_rasterize(vg);
	}
}

struct window *
vgobj_edit(void *obj)
{
	extern const struct tool line_tool, point_tool, origin_tool,
	    circle_tool, ellipse_tool;
	struct vgobj *vgo = obj;
	struct vg *vg = vgo->vg;
	struct window *win;
	struct box *bo;
	struct toolbar *tbar;
	struct statusbar *statbar;

	win = window_new(NULL);
	window_set_caption(win, _("Vector graphic: %s"), OBJECT(vgo)->name);
	window_set_closure(win, WINDOW_DETACH);

	tbar = toolbar_new(win, TOOLBAR_HORIZ, 2);
	toolbar_add_button(tbar, 0, ICON(SETTINGS_ICON), 0, 0,
	    vgobj_settings, "%p, %p", win, vgo);

	statbar = Malloc(sizeof(struct statusbar), M_OBJECT);
	statusbar_init(statbar);

	bo = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HFILL);
	box_set_spacing(bo, 0);
	box_set_padding(bo, 0);
	{
		struct toolbar *snbar;
		struct button *bu;

		snbar = vg_snap_toolbar(bo, vg, TOOLBAR_VERT);

		bo = box_new(bo, BOX_VERT, BOX_WFILL|BOX_HFILL);
		box_set_spacing(bo, 0);
		box_set_padding(bo, 0);
		{
			struct combo *laysel;
			struct mapview *mv;
		
			laysel = vg_layer_selector(bo, vg);
			mv = mapview_new(bo, vg->map, MAPVIEW_EDIT, tbar,
			    statbar);
			mapview_prescale(mv, 10, 8);
			mapview_reg_draw_cb(mv, rasterize_vgobj, vg);

			mapview_reg_tool(mv, &origin_tool, vg);
			mapview_reg_tool(mv, &point_tool, vg);
			mapview_reg_tool(mv, &line_tool, vg);
			mapview_reg_tool(mv, &circle_tool, vg);
			mapview_reg_tool(mv, &ellipse_tool, vg);
		}
	}

	object_attach(win, statbar);
	return (win);
}
