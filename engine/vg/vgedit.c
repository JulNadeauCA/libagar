/*	$Csoft$	*/

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
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mfspinbutton.h>
#include <engine/widget/palette.h>
#include <engine/mapedit/mapview.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "vgedit.h"

const struct version vgedit_ver = {
	"agar vgedit",
	0, 0
};

const struct object_ops vgedit_ops = {
	vgedit_init,
	NULL,
	vgedit_destroy,
	vgedit_load,
	vgedit_save,
	vgedit_edit
};

void
vgedit_init(void *p, const char *name)
{
	struct vgedit *vged = p;

	object_init(vged, "vgedit", name, &vgedit_ops);
	vged->vg = vg_new(vged, 0);
	vg_scale(vged->vg, 1, 1, 1);
	vg_origin(vged->vg, 0, 0);
	vg_rasterize(vged->vg);
}

void
vgedit_destroy(void *p)
{
	struct vgedit *vged = p;

	vg_destroy(vged->vg);
}

int
vgedit_load(void *p, struct netbuf *buf)
{
	struct vgedit *vged = p;

	if (version_read(buf, &vgedit_ver, NULL) != 0)
		return (-1);

	return (0);
}

int
vgedit_save(void *p, struct netbuf *buf)
{
	struct vgedit *vged = p;

	version_write(buf, &vgedit_ver);
	return (0);
}

static void
geo_changed(int argc, union evarg *argv)
{
	struct vgedit *vged = argv[1].p;

	vg_scale(vged->vg, vged->vg->w, vged->vg->h, vged->vg->scale);
	vg_rasterize(vged->vg);
}

static void
color_changed(int argc, union evarg *argv)
{
	struct vgedit *vged = argv[1].p;

	vg_rasterize(vged->vg);
}

struct window *
vgedit_edit(void *obj)
{
	struct vgedit *vged = obj;
	struct window *win;
	struct vbox *vb;

	win = window_new(NULL);
	window_set_caption(win, _("Vector drawing: %s"), OBJECT(vged)->name);
	window_set_closure(win, WINDOW_DETACH);

	vb = vbox_new(win, VBOX_WFILL|VBOX_HFILL);
	{
		struct mapview *mv;
		struct toolbar *tbar;

		tbar = toolbar_new(vb, TOOLBAR_HORIZ, 1);
		mv = mapview_new(vb, vged->vg->map, MAPVIEW_INDEPENDENT,
		    tbar);
		mapview_prescale(mv, 4, 4);
	}

	vb = vbox_new(win, VBOX_WFILL);
	vbox_set_padding(vb, 5);
	{
		struct mfspinbutton *mfsu;
		struct fspinbutton *fsu;
		struct hbox *hb;
	
		hb = hbox_new(vb, HBOX_WFILL);
		{
			mfsu = mfspinbutton_new(hb, NULL, "x", _("Geometry: "));
			widget_bind(mfsu, "xvalue", WIDGET_DOUBLE,
			    &vged->vg->w);
			widget_bind(mfsu, "yvalue", WIDGET_DOUBLE,
			    &vged->vg->h);
			mfspinbutton_set_min(mfsu, 1.0);
			mfspinbutton_set_increment(mfsu, 0.1);
			event_new(mfsu, "mfspinbutton-changed", geo_changed,
			    "%p", vged);

			fsu = fspinbutton_new(hb, NULL, " *");
			widget_bind(fsu, "value", WIDGET_DOUBLE,
			    &vged->vg->scale);
			fspinbutton_set_min(fsu, 0.1);
			fspinbutton_set_increment(fsu, 0.1);
			event_new(fsu, "fspinbutton-changed", geo_changed,
			    "%p", vged);
		}

		mfsu = mfspinbutton_new(vb, NULL, ",", _("Origin point: "));
		widget_bind(mfsu, "xvalue", WIDGET_DOUBLE, &vged->vg->ox);
		widget_bind(mfsu, "yvalue", WIDGET_DOUBLE, &vged->vg->oy);
		mfspinbutton_set_range(mfsu, 0.0, 1.0);
		mfspinbutton_set_increment(mfsu, 0.1);
		mfspinbutton_set_precision(mfsu, "f", 2);

		vb = vbox_new(vb, VBOX_WFILL);
		{
			struct palette *pal;

			label_new(vb, LABEL_STATIC, _("Background color: "));
			pal = palette_new(vb, PALETTE_RGB);
			widget_bind(pal, "color", WIDGET_UINT32,
			    &vged->vg->fill_color);
			event_new(pal, "palette-changed", color_changed, "%p",
			    vged);
		}
	}
	return (win);
}
