/*	$Csoft: vgobj.c,v 1.3 2004/04/10 04:55:17 vedge Exp $	*/

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
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mfspinbutton.h>
#include <engine/widget/palette.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>

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
	vgo->vg = vg_new(vgo, VG_VISORIGIN);
	vg_scale(vgo->vg, 8, 4, 1);
	vg_origin(vgo->vg, 0, 0);
	vg_rasterize(vgo->vg);
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
geochg(int argc, union evarg *argv)
{
	struct vgobj *vgo = argv[1].p;

	vg_scale(vgo->vg, vgo->vg->w, vgo->vg->h, vgo->vg->scale);
	vg_rasterize(vgo->vg);
}

static void
vgchg(int argc, union evarg *argv)
{
	struct vgobj *vgo = argv[1].p;

	vg_rasterize(vgo->vg);
}

static void
vgobj_settings(int argc, union evarg *argv)
{
	struct window *pwin = argv[1].p;
	struct vgobj *vgo = argv[2].p;
	struct window *win;
	struct mfspinbutton *mfsu;
	struct fspinbutton *fsu;
	struct palette *pal;
	
	win = window_new(NULL);
	window_set_caption(win, _("Parameters for \"%s\""), OBJECT(vgo)->name);
	window_set_closure(win, WINDOW_DETACH);

	mfsu = mfspinbutton_new(win, NULL, "x", _("Geometry: "));
	widget_bind(mfsu, "xvalue", WIDGET_DOUBLE, &vgo->vg->w);
	widget_bind(mfsu, "yvalue", WIDGET_DOUBLE, &vgo->vg->h);
	mfspinbutton_set_min(mfsu, 1.0);
	mfspinbutton_set_increment(mfsu, 0.1);
	event_new(mfsu, "mfspinbutton-changed", geochg, "%p", vgo);

	fsu = fspinbutton_new(win, NULL, _("Scaling factor: "));
	widget_bind(fsu, "value", WIDGET_DOUBLE, &vgo->vg->scale);
	fspinbutton_set_min(fsu, 0.1);
	fspinbutton_set_increment(fsu, 0.1);
	event_new(fsu, "fspinbutton-changed", geochg, "%p", vgo);
		
	mfsu = mfspinbutton_new(win, NULL, ",", _("Point of origin: "));
	widget_bind(mfsu, "xvalue", WIDGET_DOUBLE, &vgo->vg->ox);
	widget_bind(mfsu, "yvalue", WIDGET_DOUBLE, &vgo->vg->oy);
	mfspinbutton_set_min(mfsu, 0.0);
	mfspinbutton_set_increment(mfsu, 0.1);
	mfspinbutton_set_precision(mfsu, "f", 2);
	event_new(mfsu, "mfspinbutton-changed", vgchg, "%p", vgo);
	
	label_new(win, LABEL_STATIC, _("Background color: "));
	pal = palette_new(win, PALETTE_RGB);
	widget_bind(pal, "color", WIDGET_UINT32, &vgo->vg->fill_color);
	event_new(pal, "palette-changed", vgchg, "%p", vgo);
	
	label_new(win, LABEL_STATIC, _("Origin color: "));
	pal = palette_new(win, PALETTE_RGB);
	widget_bind(pal, "color", WIDGET_UINT32, &vgo->vg->origin_color);
	event_new(pal, "palette-changed", vgchg, "%p", vgo);

	window_attach(pwin, win);
	window_show(win);
}

struct window *
vgobj_edit(void *obj)
{
	extern const struct tool line_tool, point_tool;
	struct vgobj *vgo = obj;
	struct window *win;
	struct mapview *mv;
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
	statusbar_add_label(statbar, LABEL_STATIC, ".");

	mv = mapview_new(win, vgo->vg->map, MAPVIEW_EDIT|MAPVIEW_INDEPENDENT,
	    tbar, statbar);
	mapview_prescale(mv, 4, 4);
	mapview_reg_tool(mv, &line_tool, vgo->vg);
	mapview_reg_tool(mv, &point_tool, vgo->vg);

	object_attach(win, statbar);
	return (win);
}
