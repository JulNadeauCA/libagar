/*	$Csoft: sketch.c,v 1.28 2005/10/07 07:16:27 vedge Exp $	*/

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

#include <core/core.h>

#include <vg/vg.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/hsvpal.h>
#include <gui/fspinbutton.h>
#include <gui/spinbutton.h>
#include <gui/mspinbutton.h>
#include <gui/checkbox.h>
#include <gui/combo.h>
#include <gui/notebook.h>
#include <gui/units.h>
#include <gui/radio.h>

#include "tileset.h"
#include "tileview.h"
#include "texsel.h"

#include <string.h>

void
RG_SketchInit(RG_Sketch *sk, RG_Tileset *ts, int flags)
{
	sk->name[0] = '\0';
	sk->flags = flags;
	sk->ts = ts;
	sk->h = 0.0;
	sk->s = 0.0;
	sk->v = 0.0;
	sk->a = 1.0;

	sk->vg = Malloc(sizeof(VG), M_VG);
	VG_Init(sk->vg, VG_ANTIALIAS|VG_ALPHA);

	sk->ublks = Malloc(sizeof(struct rg_sketch_undoblk), M_RG);
	sk->nublks = 1;
	sk->curblk = 0;
	RG_SketchBeginUndoBlk(sk);
	sk->ublks[0].mods = Malloc(sizeof(struct rg_sketch_mod), M_RG);
	sk->ublks[0].nmods = 0;
}

void
RG_SketchScale(RG_Sketch *sk, int w, int h, float scale, int x, int y)
{
	VG *vg = sk->vg;
	VG_Element *vge;
	double xoffs = (float)x/(float)RG_TILESZ/scale;
	double yoffs = (float)y/(float)RG_TILESZ/scale;
	Uint32 i;
	double vw, vh;

	if (w == -1) {
		vw = vg->rDst.w;
	} else {
		vw = (float)w/(float)RG_TILESZ/scale;
	}
	if (h == -1) {
		vh = vg->rDst.h;
	} else {
		vh = (float)h/(float)RG_TILESZ/scale;
	}

	VG_Scale(vg, vw, vh, scale);
	
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vge->vtx[i].x += xoffs;
			vge->vtx[i].y += yoffs;
		}
	}
}

void
RG_SketchDestroy(RG_Sketch *sk)
{
	int i;

	VG_Destroy(sk->vg);

	for (i = 0; i < sk->nublks; i++) {
		Free(sk->ublks[i].mods, M_RG);
	}
	Free(sk->ublks, M_RG);
}

int
RG_SketchLoad(RG_Sketch *sk, AG_Netbuf *buf)
{
	int vgflags;
	
	AG_CopyString(sk->name, buf, sizeof(sk->name));
	sk->flags = (int)AG_ReadUint32(buf);
	vgflags = (int)AG_ReadUint32(buf);

	sk->vg = VG_New(vgflags);
	if (VG_Load(sk->vg, buf) == -1) {
		VG_Destroy(sk->vg);
		Free(sk->vg, M_VG);
		return (-1);
	}
	VG_Rasterize(sk->vg);
	return (0);
}

void
RG_SketchSave(RG_Sketch *sk, AG_Netbuf *buf)
{
	AG_WriteString(buf, sk->name);
	AG_WriteUint32(buf, (Uint32)sk->flags);
	AG_WriteUint32(buf, (Uint32)sk->vg->flags);
	VG_Save(sk->vg, buf);
}

void
RG_SketchRender(RG_Tile *t, RG_TileElement *tel)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	VG_Element *vge;
	SDL_Rect rd;

	SDL_FillRect(vg->su, NULL, vg->fill_color);
	
	if (vg->flags & VG_VISGRID)
		VG_DrawGrid(vg);
#ifdef DEBUG
	if (vg->flags & VG_VISBBOXES)
		VG_DrawExtents(vg);
#endif
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		switch (vge->type) {
		case VG_POLYGON:
			RG_SketchDrawPolygon(t, vg, vge);
			break;
		default:
			VG_Rasterize(vg);
			break;
		}
	}
	
	if (vg->flags & VG_VISORIGIN)
		VG_DrawOrigin(vg);
	
	rd.x = tel->tel_sketch.x;
	rd.y = tel->tel_sketch.y;
	rd.w = vg->su->w;
	rd.h = vg->su->h;
	t->blend_fn(t, vg->su, &rd);
}

static void
update_sketch(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	RG_TileElement *tel = AG_PTR(2);
	RG_Sketch *sk = tel->tel_sketch.sk;

	RG_SketchScale(sk, -1, -1, tel->tel_sketch.scale, 0, 0);

	tv->tile->flags |= RG_TILE_DIRTY;
}

AG_Window *
RG_SketchEdit(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Sketch %s"), sk->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;
		AG_FSpinbutton *fsb;
		AG_Box *hb;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_WidgetBind(pal, "pixel-format", AG_WIDGET_POINTER,
		    &tv->ts->fmt);
		AG_WidgetBind(pal, "hue", AG_WIDGET_FLOAT, &sk->h);
		AG_WidgetBind(pal, "saturation", AG_WIDGET_FLOAT, &sk->s);
		AG_WidgetBind(pal, "value", AG_WIDGET_FLOAT, &sk->v);
		AG_WidgetBind(pal, "alpha", AG_WIDGET_FLOAT, &sk->a);
	}
	ntab = AG_NotebookAddTab(nb, _("Texture"), AG_BOX_VERT);

	ntab = AG_NotebookAddTab(nb, _("Settings"), AG_BOX_VERT);
	{
		AG_FSpinbutton *fsb;
		AG_Spinbutton *sb;
		AG_MSpinbutton *msb;

		fsb = AG_FSpinbuttonNew(ntab, 0, NULL, _("Scale: "));
		AG_WidgetBind(fsb, "value", AG_WIDGET_FLOAT,
		    &tel->tel_sketch.scale);
		AG_FSpinbuttonSetMin(fsb, 0.0001);
		AG_FSpinbuttonSetIncrement(fsb, 0.1);
		AG_SetEvent(fsb, "fspinbutton-changed", update_sketch, "%p,%p",
		    tv, tel);
		
		msb = AG_MSpinbuttonNew(ntab, 0, ",", _("Coordinates: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT, &tel->tel_sketch.x);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT, &tel->tel_sketch.y);
		AG_SetEvent(fsb, "fspinbutton-changed", update_sketch, "%p,%p",
		    tv, tel);
		
		sb = AG_SpinbuttonNew(ntab, 0, _("Overall alpha: "));
		AG_SpinbuttonSetRange(sb, 0, 255);
		AG_WidgetBind(sb, "value", AG_WIDGET_INT,
		    &tel->tel_sketch.alpha);
	}

	return (win);
}

static void
poll_styles(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	VG *vg = AG_PTR(1);
	VG_Style *vgs;

	AG_TlistClear(tl);
	AG_MutexLock(&vg->lock);
	TAILQ_FOREACH(vgs, &vg->styles, styles) {
		AG_TlistAddPtr(tl, NULL, vgs->name, vgs);
	}
	AG_MutexUnlock(&vg->lock);
	AG_TlistRestore(tl);
}

AG_Window *
RG_SketchEditElement(RG_Tileview *tv, RG_TileElement *tel,
    VG_Element *vge)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	const VG_ElementOps *ops;
	AG_Radio *rad;
	AG_Spinbutton *sb;
	AG_Combo *com;
	
	if ((win = AG_WindowNewNamed(0, "%s-%p-%p", sk->name, tel, vge))
	    == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Sketch element (%s)"), sk->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 0);

	ops = vgElementTypes[vge->type];
	AG_LabelNewFmt(win, _("Element type: %s"), ops->name);
	AG_LabelNew(win, AG_LABEL_POLLED, _("Vertices: %u"), &vge->nvtx);
	
	sb = AG_SpinbuttonNew(win, 0, _("Layer: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &vge->layer);
	AG_SpinbuttonSetMin(sb, 0);

	com = AG_ComboNew(win, AG_COMBO_POLL|AG_COMBO_HFILL, _("Style: "));
	AG_SetEvent(com->list, "tlist-poll", poll_styles, "%p", vg);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_WidgetBind(pal, "pixel-format", AG_WIDGET_POINTER,
		    &sk->vg->fmt);
		AG_WidgetBind(pal, "pixel", AG_WIDGET_UINT32, &vge->color);
	}
	ntab = AG_NotebookAddTab(nb, _("Line style"), AG_BOX_VERT);
	{
		const char *line_styles[] = {
			N_("Continuous"),
			N_("Stippled"),
			NULL
		};
		const char *endpoint_styles[] = {
			N_("Square"),
			N_("Beveled"),
			N_("Rounded"),
			N_("Mitered"),
			NULL
		};
	
		AG_LabelNewStatic(ntab, _("Line style: "));
		rad = AG_RadioNew(ntab, AG_RADIO_HFILL, line_styles);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &vge->line_st.style);
		
		AG_LabelNewStatic(ntab, _("Endpoint style: "));
		rad = AG_RadioNew(ntab, AG_RADIO_HFILL, endpoint_styles);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT,
		    &vge->line_st.endpoint_style);

		sb = AG_SpinbuttonNew(ntab, 0, _("Stipple pattern: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT16,
		    &vge->line_st.stipple);
		
		sb = AG_SpinbuttonNew(ntab, 0, _("Thickness: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT8,
		    &vge->line_st.thickness);
		AG_SpinbuttonSetMin(sb, 1);
		
		sb = AG_SpinbuttonNew(ntab, 0, _("Miter length: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT8,
		    &vge->line_st.miter_len);
	}
	ntab = AG_NotebookAddTab(nb, _("Filling style"), AG_BOX_VERT);
	{
		const char *fill_styles[] = {
			N_("None"),
			N_("Solid"),
			N_("Textured"),
			NULL
		};
		RG_TextureSelector *texsel;

		AG_LabelNewStatic(ntab, _("Filling style: "));
		rad = AG_RadioNew(ntab, AG_RADIO_HFILL, fill_styles);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &vge->fill_st.style);
			
		AG_LabelNewStatic(ntab, _("Texture: "));
		texsel = RG_TextureSelectorNew(ntab, tv->ts, 0);
		AGWIDGET(texsel)->flags |= AG_WIDGET_HFILL|AG_WIDGET_VFILL;
		AG_WidgetBind(texsel, "texture-name", AG_WIDGET_STRING,
		    vge->fill_st.texture, sizeof(vge->fill_st.texture));
	}

	return (win);
}

void
RG_SketchBeginUndoBlk(RG_Sketch *sk)
{
	struct rg_sketch_undoblk *ublk;

	while (sk->nublks > sk->curblk+1) {
		ublk = &sk->ublks[sk->nublks-1];
		Free(ublk->mods, M_RG);
		sk->nublks--;
	}

	sk->ublks = Realloc(sk->ublks, ++sk->nublks *
	                    sizeof(struct rg_pixmap_mod));
	sk->curblk++;

	ublk = &sk->ublks[sk->curblk];
	ublk->mods = Malloc(sizeof(struct rg_pixmap_mod), M_RG);
	ublk->nmods = 0;
}

void
RG_SketchUndo(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	struct rg_sketch_undoblk *ublk = &sk->ublks[sk->curblk];
	int i;

	if (sk->curblk-1 <= 0)
		return;

	for (i = 0; i < ublk->nmods; i++) {
		struct rg_sketch_mod *mod = &ublk->mods[i];

		dprintf("undo mod %p\n", mod);
	}
	sk->curblk--;
	tv->tile->flags |= RG_TILE_DIRTY;
}

void
RG_SketchRedo(RG_Tileview *tv, RG_TileElement *tel)
{
	/* TODO */
}

static void
update_circle_radius(AG_Event *event)
{
	VG *vg = AG_PTR(1);
	VG_Element *vge = AG_PTR(2);

	vge->vg_circle.radius = sqrt(
	    pow(vge->vtx[1].x - vge->vtx[0].x, 2) +
	    pow(vge->vtx[1].y - vge->vtx[0].y, 2));
}

static void
update_circle_vertex(AG_Event *event)
{
	VG *vg = AG_PTR(1);
	VG_Element *vge = AG_PTR(2);

	vge->vtx[1].x = vge->vtx[0].x + vge->vg_circle.radius;
	vge->vtx[1].y = vge->vtx[0].y;
}

AG_Window *
RG_SketchSelect(RG_Tileview *tv, RG_TileElement *tel,
    VG_Element *vge)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	RG_TileviewCtrl *ctrl;
	Uint i;

	vge->selected = 1;

	switch (vge->type) {
	case VG_LINES:
	case VG_LINE_STRIP:
	case VG_LINE_LOOP:
	case VG_POLYGON:
		for (i = 0; i < vge->nvtx; i++) {
			ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_VERTEX,
			    "%*d,%*d", &vge->vtx[i].x, &vge->vtx[i].y);
			ctrl->vg = vg;
			ctrl->vge = vge;
			ctrl->buttonup = NULL;
		}
		break;
	case VG_CIRCLE:
		ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[0].x, &vge->vtx[0].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = NULL;
		ctrl->motion = AG_SetEvent(tv, NULL, update_circle_vertex,
		    "%p,%p", vg, vge);
		
		ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[1].x, &vge->vtx[1].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = NULL;
		ctrl->motion = AG_SetEvent(tv, NULL, update_circle_radius,
		    "%p,%p", vg, vge);
		break;
	default:
		break;
	}
	return (RG_SketchEditElement(tv, tel, vge));
}

void
RG_SketchUnselect(RG_Tileview *tv, RG_TileElement *tel,
    VG_Element *vge)
{
	char name[AG_OBJECT_NAME_MAX];
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	RG_TileviewCtrl *ctrl, *nctrl;
	AG_Window *win;

	TAILQ_FOREACH(win, &agView->windows, windows) {
		snprintf(name, sizeof(name), "win-%s-%p-%p", sk->name, tel,
		    vge);
		if (strcmp(name, AGOBJECT(win)->name) == 0) {
			AG_ViewDetach(win);
			break;
		}
	}

	for (ctrl = TAILQ_FIRST(&tv->ctrls);
	     ctrl != TAILQ_END(&tv->ctrls);
	     ctrl = nctrl) {
		nctrl = TAILQ_NEXT(ctrl, ctrls);
		if (ctrl->vg == vg && ctrl->vge == vge)
			RG_TileviewDelCtrl(tv, ctrl);
	}

	vge->selected = 0;
}

void
RG_SketchButtondown(RG_Tileview *tv, RG_TileElement *tel,
    double x, double y, int button)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;

	if (button == SDL_BUTTON_MIDDLE) {
		int x, y;

		AG_MouseGetState(&x, &y);
		RG_SketchOpenMenu(tv, x, y);
		return;
	} else if (button == SDL_BUTTON_RIGHT) {
		if (tv->cur_tool == NULL ||
		   (tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) == 0) {
			tv->scrolling++;
			return;
		}
	}

	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const RG_TileviewSketchToolOps *ops =
		    (const RG_TileviewSketchToolOps *)tv->cur_tool->ops;
		
		if (ops->mousebuttondown != NULL) {
			ops->mousebuttondown(tv->cur_tool, sk, x, y, button);
			return;
		}
	}

	{
		VG_Element *vge;
		float idx, closest_idx = FLT_MAX;
		VG_Element *closest_vge = NULL;

		TAILQ_FOREACH(vge, &vg->vges, vges) {
			if (vge->ops->intsect != NULL) {
				float ix = (float)x;
				float iy = (float)y;

				idx = vge->ops->intsect(vg, vge, &ix, &iy);
				if (idx < closest_idx) {
					closest_idx = idx;
					closest_vge = vge;
				}
			}
		}
		if (closest_vge != NULL && closest_idx < FLT_MAX-2) {
			if (closest_vge->selected) {
				RG_SketchUnselect(tv, tel, closest_vge);
			} else {
				AG_Window *pwin = AG_WidgetParentWindow(tv);
				AG_Window *win;

				if ((SDL_GetModState() & KMOD_CTRL) == 0) {
					TAILQ_FOREACH(vge, &vg->vges, vges) {
						if (vge->selected)
							RG_SketchUnselect(tv,
							    tel, vge);
					}
				}

				win = RG_SketchSelect(tv, tel, closest_vge);
				if (win != NULL) {
					AG_WindowAttach(pwin, win);
					AG_WindowShow(win);
					agView->focus_win = pwin;
					AG_WidgetFocus(tv);
				}
			}
		}
	}
}

void
RG_SketchButtonup(RG_Tileview *tv, RG_TileElement *tel,
    double x, double y, int button)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const RG_TileviewSketchToolOps *ops =
		    (const RG_TileviewSketchToolOps *)tv->cur_tool->ops;
		
		if (ops->mousebuttonup != NULL)
			ops->mousebuttonup(tv->cur_tool, tel->tel_sketch.sk,
			    x, y, button);
	}
}

void
RG_SketchMotion(RG_Tileview *tv, RG_TileElement *tel, double x, double y,
    double xrel, double yrel, int state)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const RG_TileviewSketchToolOps *ops =
		    (const RG_TileviewSketchToolOps *)tv->cur_tool->ops;
		
		if (ops->mousemotion != NULL) {
			ops->mousemotion(tv->cur_tool, tel->tel_sketch.sk,
			    x, y, xrel, yrel);
			return;
		}
	}

	{
		RG_Sketch *sk = tel->tel_sketch.sk;
		VG *vg = sk->vg;
		float idx, closest_idx = FLT_MAX;
		VG_Element *vge, *closest_vge = NULL;

		TAILQ_FOREACH(vge, &vg->vges, vges) {
			vge->mouseover = 0;
			if (vge->ops->intsect != NULL) {
				float ix = (float)x;
				float iy = (float)y;

				idx = vge->ops->intsect(vg, vge, &ix, &iy);
				if (idx < closest_idx) {
					closest_idx = idx;
					closest_vge = vge;
				}
			}
		}
		if (closest_vge != NULL && closest_idx < FLT_MAX-2) {
			closest_vge->mouseover = 1;
		}
	}
}

int
RG_SketchWheel(RG_Tileview *tv, RG_TileElement *tel, int which)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const RG_TileviewSketchToolOps *ops =
		    (const RG_TileviewSketchToolOps *)tv->cur_tool->ops;
		
		if (ops->mousewheel != NULL)
			return (ops->mousewheel(tv->cur_tool,
			    tel->tel_sketch.sk, which));
	}
	return (0);
}

void
RG_SketchKeyDown(RG_Tileview *tv, RG_TileElement *tel, int keysym,
    int keymod)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	VG_Element *vge, *nvge;

	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const RG_TileviewSketchToolOps *ops =
		    (const RG_TileviewSketchToolOps *)tv->cur_tool->ops;
		
		if (ops->keydown != NULL) {
			ops->keydown(tv->cur_tool, sk, keysym, keymod);
			return;
		}
	}
	switch (keysym) {
	case SDLK_DELETE:
		for (vge = TAILQ_FIRST(&vg->vges);
		     vge != TAILQ_END(&vg->vges);
		     vge = nvge) {
		     	nvge = TAILQ_NEXT(vge, vges);
			if (vge->selected) {
				RG_SketchUnselect(tv, tel, vge);
				VG_DestroyElement(vg, vge);
			}
		}
		break;
	}
}

void
RG_SketchKeyUp(RG_Tileview *tv, RG_TileElement *tel, int keysym,
    int keymod)
{
	RG_Sketch *sk = tel->tel_sketch.sk;

	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const RG_TileviewSketchToolOps *ops =
		    (const RG_TileviewSketchToolOps *)tv->cur_tool->ops;
		
		if (ops->keyup != NULL) {
			ops->keyup(tv->cur_tool, sk, keysym, keymod);
			return;
		}
	}
}

static void
select_tool(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	RG_TileviewTool *tvt = AG_PTR(2);

	RG_TileviewSelectTool(tv, tvt);
}

static void
select_tool_tbar(AG_Event *event)
{
	AG_Button *btn = AG_SELF();
	AG_Toolbar *tbar = AG_PTR(1);
	RG_Tileview *tv = AG_PTR(2);
	RG_TileviewTool *tvt = AG_PTR(3);

	AG_ToolbarSelectOnly(tbar, btn);
	if (tv->cur_tool == tvt) {
		RG_TileviewUnselectTool(tv);
	} else {
		RG_TileviewSelectTool(tv, tvt);
	}
}

void
RG_SketchOpenMenu(RG_Tileview *tv, int x, int y)
{
	RG_Sketch *sk = tv->tv_sketch.sk;
	AG_Menu *me;
	AG_MenuItem *mi;
	
	if (tv->tv_sketch.menu != NULL)
		RG_SketchCloseMenu(tv);

	me = tv->tv_sketch.menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(me, 0);

	mi = tv->tv_sketch.menu_item = AG_MenuAddItem(me, NULL);
	{
		RG_TileviewTool *tvt;
		AG_MenuItem *m_tool;

		m_tool = AG_MenuAction(mi, _("Tools"), OBJEDIT_ICON,
		    NULL, NULL);
		TAILQ_FOREACH(tvt, &tv->tools, tools) {
			if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
				continue;
			}
			AG_MenuAction(m_tool, _(tvt->ops->name), tvt->ops->icon,
			    select_tool, "%p,%p", tv, tvt);
		}

		AG_MenuSeparator(mi);
	
		AG_MenuIntFlags(mi, _("Show sketch origin"), VGORIGIN_ICON,
		    &sk->vg->flags, VG_VISORIGIN, 0);
		AG_MenuIntFlags(mi, _("Show sketch grid"), SNAP_GRID_ICON,
		    &sk->vg->flags, VG_VISGRID, 0);
		AG_MenuIntFlags(mi, _("Show sketch extents"), VGBLOCK_ICON,
		    &sk->vg->flags, VG_VISBBOXES, 0);

		AG_MenuSeparator(mi);
		
		RG_TileviewGenericMenu(tv, mi);
	}
	tv->tv_sketch.menu->sel_item = mi;
	tv->tv_sketch.menu_win = AG_MenuExpand(me, mi, x, y);
}

void
RG_SketchCloseMenu(RG_Tileview *tv)
{
	AG_Menu *me = tv->tv_sketch.menu;
	AG_MenuItem *mi = tv->tv_sketch.menu_item;

	AG_MenuCollapse(me, mi);
	AG_ObjectDestroy(me);
	Free(me, M_OBJECT);

	tv->tv_sketch.menu = NULL;
	tv->tv_sketch.menu_item = NULL;
	tv->tv_sketch.menu_win = NULL;
}

AG_Toolbar *
RG_SketchToolbar(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	AG_Toolbar *tbar;
	RG_TileviewTool *tvt;

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1,
	    AG_TOOLBAR_STICKY);
	TAILQ_FOREACH(tvt, &tv->tools, tools) {
		if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
			continue;
		}
		AG_ToolbarButtonIcon(tbar, tvt->ops->icon >= 0 ?
		    AGICON(tvt->ops->icon) : NULL, 0,
		    select_tool_tbar, "%p,%p,%p", tbar, tv, tvt);
	}
	return (tbar);
}

