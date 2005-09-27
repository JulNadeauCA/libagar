/*	$Csoft: sketchproj.c,v 1.6 2005/08/29 03:29:05 vedge Exp $	*/

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

#include <engine/vg/vg.h>

#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/radio.h>
#include <engine/widget/box.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/combo.h>

#include "tileset.h"
#include "tileview.h"
#include "sketchproj.h"

const AG_Version rgSketchProjVer = {
	"agar rg sketch projection feature",
	0, 0
};

const RG_FeatureOps rgSketchProjOps = {
	"sketchproj",
	sizeof(struct rg_sketchproj),
	N_("Sketch projection."),
	FEATURE_AUTOREDRAW,
	RG_SketchProjInit,
	RG_SketchProjLoad,
	RG_SketchProjSave,
	NULL,		/* destroy */
	RG_SketchProjApply,
	NULL,
	NULL,
	RG_SketchProjEdit
};

void
RG_SketchProjInit(void *p, RG_Tileset *ts, int flags)
{
	struct rg_sketchproj *sproj = p;

	AG_FeatureInit(sproj, ts, flags, &rgSketchProjOps);
	sproj->alpha = 255;
	sproj->color = SDL_MapRGB(ts->fmt, 0, 0, 0);
	sproj->sketch[0] = '\0';
}

int
RG_SketchProjLoad(void *p, AG_Netbuf *buf)
{
	struct rg_sketchproj *sproj = p;
	RG_Tileset *ts = RG_FEATURE(sproj)->ts;

	if (AG_ReadVersion(buf, &rgSketchProjVer, NULL) == -1)
		return (-1);

	AG_CopyString(sproj->sketch, buf, sizeof(sproj->sketch));
	sproj->alpha = AG_ReadUint8(buf);
	sproj->color = AG_ReadColor(buf, ts->fmt);
	return (0);
}

void
RG_SketchProjSave(void *p, AG_Netbuf *buf)
{
	struct rg_sketchproj *sproj = p;
	RG_Tileset *ts = RG_FEATURE(sproj)->ts;

	AG_WriteVersion(buf, &rgSketchProjVer);

	AG_WriteString(buf, sproj->sketch);
	AG_WriteUint8(buf, sproj->alpha);
	AG_WriteColor(buf, ts->fmt, sproj->color);
}

static void
poll_sketches(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tile *t = argv[1].p;
	RG_Tileset *ts = t->ts;
	RG_TileElement *tel;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->type != RG_TILE_SKETCH) {
			continue;
		}
		it = AG_TlistAdd(tl, NULL, "%s", tel->name);
		it->p1 = tel;
		it->class = "tile-sketch";
		AG_TlistSetIcon(tl, it, tel->tel_sketch.sk->vg->su);
	}
	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
select_sketch(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	struct rg_sketchproj *sproj = argv[1].p;
	RG_Tile *t = argv[2].p;
	AG_TlistItem *it = argv[3].p;

	strlcpy(sproj->sketch, it->text, sizeof(sproj->sketch));
}

AG_Window *
RG_SketchProjEdit(void *p, RG_Tileview *tv)
{
	struct rg_sketchproj *sproj = p;
	AG_Window *win;
	AG_Box *box;
	AG_Combo *com;

	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _("Polygon"));

	com = AG_ComboNew(win, AG_COMBO_POLL, _("Sketch: "));
	AG_SetEvent(com->list, "tlist-poll", poll_sketches, "%p", tv->tile);
	AG_SetEvent(com, "combo-selected", select_sketch, "%p,%p", sproj,
	    tv->tile);
	AG_ComboSelectText(com, sproj->sketch);

	box = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL|AG_BOX_HFILL);
	{
		AG_HSVPal *hsv1, *hsv2;
		AG_Spinbutton *sb;
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Box *hb;

		nb = AG_NotebookNew(box, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
		ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
		{
			hsv1 = AG_HSVPalNew(ntab);
			AGWIDGET(hsv1)->flags |= AG_WIDGET_WFILL|
			                       AG_WIDGET_HFILL;
			AG_WidgetBind(hsv1, "pixel-format", AG_WIDGET_POINTER,
			    &tv->ts->fmt);
			AG_WidgetBind(hsv1, "pixel", AG_WIDGET_UINT32,
			    &sproj->color);
		}

		sb = AG_SpinbuttonNew(box, _("Overall alpha: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT8, &sproj->alpha);
		AG_SpinbuttonSetRange(sb, 0, 255);
		AG_SpinbuttonSetIncrement(sb, 5);
	}
	return (win);
}

void
RG_SketchProjApply(void *p, RG_Tile *t, int fx, int fy)
{
	struct rg_sketchproj *sproj = p;
	SDL_Surface *sDst = t->su;
	double x1, y1, x2, y2;
	VG *vg;
	VG_Element *vge;
	RG_TileElement *ske;
	RG_Sketch *sk;
	int i;

	if ((ske = RG_TileFindElement(t, RG_TILE_SKETCH, sproj->sketch))
	    == NULL) {
		return;
	}
	sk = ske->tel_sketch.sk;
	vg = ske->tel_sketch.sk->vg;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		switch (vge->type) {
		case VG_LINE_STRIP:
			VG_VtxCoords2d(vg, &vge->vtx[0], &x1, &y1);
			for (i = 1; i < vge->nvtx; i++) {
				VG_VtxCoords2d(vg, &vge->vtx[i], &x2, &y2);
				RG_ColorUint32(t, sproj->color);
				RG_WuLine(t,
				    ske->tel_sketch.x+x1,
				    ske->tel_sketch.y+y1,
				    ske->tel_sketch.x+x2,
				    ske->tel_sketch.y+y2);
				x1 = x2;
				y1 = y2;
			}
			break;
		default:
			break;
		}
	}
}

