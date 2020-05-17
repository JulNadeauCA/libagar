/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/vg.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>
#include <agar/map/rg_sketchproj.h>

const AG_Version rgSketchProjVer = { 0, 0 };
const RG_FeatureOps rgSketchProjOps = {
	"sketchproj",
	sizeof(struct rg_sketchproj),
	N_("Sketch projection."),
	FEATURE_AUTOREDRAW,
	AG_KEYMOD_SHIFT, AG_KEY_P,
	1,
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
RG_SketchProjInit(void *p, RG_Tileset *ts, Uint flags)
{
	struct rg_sketchproj *sproj = p;

	RG_FeatureInit(sproj, ts, flags, &rgSketchProjOps);
	sproj->alpha = 255;
	sproj->color = AG_MapPixelRGB(ts->fmt, 0,0,0);
	sproj->sketch[0] = '\0';
}

int
RG_SketchProjLoad(void *p, AG_DataSource *buf)
{
	struct rg_sketchproj *sproj = p;
	RG_Tileset *ts = RG_FEATURE(sproj)->ts;

	if (AG_ReadVersion(buf, "RG_Feature:RG_SketchProj", &rgSketchProjVer,
	    NULL) == -1)
		return (-1);

	AG_CopyString(sproj->sketch, buf, sizeof(sproj->sketch));
	sproj->alpha = AG_ReadUint8(buf);
	sproj->color = AG_ReadColor(buf, ts->fmt);
	return (0);
}

void
RG_SketchProjSave(void *p, AG_DataSource *buf)
{
	struct rg_sketchproj *sproj = p;
	RG_Tileset *ts = RG_FEATURE(sproj)->ts;

	AG_WriteVersion(buf, "RG_Feature:RG_SketchProj", &rgSketchProjVer);

	AG_WriteString(buf, sproj->sketch);
	AG_WriteUint8(buf, sproj->alpha);
	AG_WriteColor(buf, ts->fmt, sproj->color);
}

static void
PollSketches(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Tile *t = AG_PTR(1);
	RG_Tileset *ts = t->ts;
	RG_TileElement *tel;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->type != RG_TILE_SKETCH) {
			continue;
		}
		it = AG_TlistAddS(tl, NULL, tel->name);
		it->p1 = tel;
		it->cat = "tile-sketch";
		AG_TlistSetIcon(tl, it, tel->tel_sketch.sk->vg->su);
	}
	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
SelectSketch(AG_Event *_Nonnull event)
{
	struct rg_sketchproj *sproj = AG_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(3);

	Strlcpy(sproj->sketch, it->text, sizeof(sproj->sketch));
}

AG_Window *
RG_SketchProjEdit(void *p, RG_Tileview *tv)
{
	struct rg_sketchproj *sproj = p;
	AG_Window *win;
	AG_Box *box;
	AG_Combo *comSK;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Polygon"));

	comSK = AG_ComboNew(win, AG_COMBO_POLL | AG_COMBO_HFILL, _("Sketch: "));
	AG_SetEvent(comSK->list, "tlist-poll",
	    PollSketches, "%p", tv->tile);
	AG_SetEvent(comSK, "combo-selected",
	    SelectSketch, "%p,%p", sproj, tv->tile);
	AG_ComboSelectText(comSK, sproj->sketch);

	box = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL | AG_BOX_VFILL);
	{
		AG_HSVPal *hsv1;
		AG_Numerical *num;
		AG_Notebook *nb;
		AG_NotebookTab *ntab;

		nb = AG_NotebookNew(box, AG_NOTEBOOK_HFILL | AG_NOTEBOOK_VFILL);
		ntab = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
		{
			hsv1 = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
			AG_BindPointer(hsv1, "pixel-format", &tv->ts->fmt);
			AG_BindUint32(hsv1, "pixel", &sproj->color);
		}

		num = AG_NumericalNewUint8(box, 0, NULL, _("Alpha: "),
		    &sproj->alpha);
		AG_NumericalSetRange(num, 0, 255);
		AG_NumericalSetIncrement(num, 5);
	}

	AG_WidgetFocus(comSK);
	return (win);
}

void
RG_SketchProjApply(void *p, RG_Tile *t, int fx, int fy)
{
	struct rg_sketchproj *sproj = p;
	float x1, y1, x2, y2;
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
			VG_VtxCoords2d(vg, vge, 0, &x1, &y1);
			for (i = 1; i < vge->nvtx; i++) {
				VG_VtxCoords2d(vg, vge, i, &x2, &y2);
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
