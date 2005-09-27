/*	$Csoft: sketch.c,v 1.24 2005/06/18 03:46:01 vedge Exp $	*/

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
#include <engine/input.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/combo.h>
#include <engine/widget/notebook.h>
#include <engine/widget/units.h>
#include <engine/widget/radio.h>

#include "tileset.h"
#include "tileview.h"
#include "texsel.h"

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
	double xoffs = (float)x/(float)AGTILESZ/scale;
	double yoffs = (float)y/(float)AGTILESZ/scale;
	Uint32 i;
	double vw, vh;

	if (w == -1) {
		vw = vg->w;
	} else {
		vw = (float)w/(float)AGTILESZ/scale;
	}
	if (h == -1) {
		vh = vg->h;
	} else {
		vh = (float)h/(float)AGTILESZ/scale;
	}

	VG_Scale(vg, vw, vh, scale);
	
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vge->vtx[i].x += xoffs;
			vge->vtx[i].y += yoffs;
		}
	}
	vg->redraw++;
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

	sk->vg = VG_New(NULL, vgflags);
	if (VG_Load(sk->vg, buf) == -1) {
		VG_Destroy(sk->vg);
		Free(sk->vg, M_VG);
		return (-1);
	}
	sk->vg->redraw++;
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
		vge->drawn = 0;
	}
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		switch (vge->type) {
		case VG_POLYGON:
			RG_SketchDrawPolygon(t, vg, vge);
			vge->drawn = 1;
			break;
		default:
			VG_RasterizeElement(vg, vge);
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
update_sketch(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	RG_TileElement *tel = argv[2].p;
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
	AG_Combo *com;
	
	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _("Sketch %s"), sk->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	com = VG_NewLayerSelector(win, vg);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;
		AG_FSpinbutton *fsb;
		AG_Box *hb;

		pal = AG_HSVPalNew(ntab);
		AGWIDGET(pal)->flags |= AG_WIDGET_WFILL|AG_WIDGET_HFILL;
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

		fsb = AG_FSpinbuttonNew(ntab, NULL, _("Scale: "));
		AG_WidgetBind(fsb, "value", AG_WIDGET_FLOAT,
		    &tel->tel_sketch.scale);
		AG_FSpinbuttonSetMin(fsb, 0.0001);
		AG_FSpinbuttonSetIncrement(fsb, 0.1);
		AG_SetEvent(fsb, "fspinbutton-changed", update_sketch, "%p,%p",
		    tv, tel);
		
		msb = AG_MSpinbuttonNew(ntab, ",", _("Coordinates: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT, &tel->tel_sketch.x);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT, &tel->tel_sketch.y);
		AG_SetEvent(fsb, "fspinbutton-changed", update_sketch, "%p,%p",
		    tv, tel);
		
		sb = AG_SpinbuttonNew(ntab, _("Overall alpha: "));
		AG_SpinbuttonSetRange(sb, 0, 255);
		AG_WidgetBind(sb, "value", AG_WIDGET_INT,
		    &tel->tel_sketch.alpha);
	}

	return (win);
}

static void
poll_styles(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	VG *vg = argv[1].p;
	VG_Style *vgs;

	AG_TlistClear(tl);
	pthread_mutex_lock(&vg->lock);
	TAILQ_FOREACH(vgs, &vg->styles, styles) {
		AG_TlistAddPtr(tl, NULL, vgs->name, vgs);
	}
	pthread_mutex_unlock(&vg->lock);
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
	
	if ((win = AG_WindowNew(0, "%s-%p-%p", sk->name, tel, vge)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Sketch element (%s)"), sk->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 0);

	ops = vgElementTypes[vge->type];
	AG_LabelStaticF(win, _("Element type: %s"), ops->name);
	AG_LabelNew(win, AG_LABEL_POLLED, _("Vertices: %u"), &vge->nvtx);
	
	sb = AG_SpinbuttonNew(win, _("Layer: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &vge->layer);
	AG_SpinbuttonSetMin(sb, 0);

	com = AG_ComboNew(win, AG_COMBO_POLL, _("Style: "));
	AG_SetEvent(com->list, "tlist-poll", poll_styles, "%p", vg);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab);
		AGWIDGET(pal)->flags |= AG_WIDGET_WFILL|AG_WIDGET_HFILL;
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
	
		AG_LabelStatic(ntab, _("Line style: "));
		rad = AG_RadioNew(ntab, line_styles);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &vge->line_st.style);
		
		AG_LabelStatic(ntab, _("Endpoint style: "));
		rad = AG_RadioNew(ntab, endpoint_styles);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT,
		    &vge->line_st.endpoint_style);

		sb = AG_SpinbuttonNew(ntab, _("Stipple pattern: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT16,
		    &vge->line_st.stipple);
		
		sb = AG_SpinbuttonNew(ntab, _("Thickness: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT8,
		    &vge->line_st.thickness);
		AG_SpinbuttonSetMin(sb, 1);
		
		sb = AG_SpinbuttonNew(ntab, _("Miter length: "));
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

		AG_LabelStatic(ntab, _("Filling style: "));
		rad = AG_RadioNew(ntab, fill_styles);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &vge->fill_st.style);
			
		AG_LabelStatic(ntab, _("Texture: "));
		texsel = RG_TextureSelectorNew(ntab, tv->ts, 0);
		AGWIDGET(texsel)->flags |= AG_WIDGET_WFILL|AG_WIDGET_HFILL;
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
redraw_sketch(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;

	vg->redraw++;
}

static void
update_circle_radius(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;
	VG_Element *vge = argv[2].p;

	vge->vg_circle.radius = sqrt(
	    pow(vge->vtx[1].x - vge->vtx[0].x, 2) +
	    pow(vge->vtx[1].y - vge->vtx[0].y, 2));
}

static void
update_circle_vertex(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;
	VG_Element *vge = argv[2].p;

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
	u_int i;

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
			ctrl->buttonup = AG_SetEvent(tv, NULL, redraw_sketch,
			    "%p", vg);
		}
		break;
	case VG_CIRCLE:
		ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[0].x, &vge->vtx[0].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = AG_SetEvent(tv, NULL, redraw_sketch, "%p", vg);
		ctrl->motion = AG_SetEvent(tv, NULL, update_circle_vertex,
		    "%p,%p", vg, vge);
		
		ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[1].x, &vge->vtx[1].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = AG_SetEvent(tv, NULL, redraw_sketch, "%p", vg);
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
RG_SketchMouseButtonDown(RG_Tileview *tv, RG_TileElement *tel,
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
				idx = vge->ops->intsect(vg, vge, x, y);
				if (idx < closest_idx) {
					closest_idx = idx;
					closest_vge = vge;
					if (idx == 0)
						break;
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
			vg->redraw++;
		}
	}
}

void
RG_SketchMouseButtonUp(RG_Tileview *tv, RG_TileElement *tel,
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
RG_SketchMouseMotion(RG_Tileview *tv, RG_TileElement *tel,
    double x, double y, double xrel, double yrel, int state)
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
				idx = vge->ops->intsect(vg, vge, x, y);
				if (idx < closest_idx) {
					closest_idx = idx;
					closest_vge = vge;
					if (idx == 0)
						break;
				}
			}
		}
		if (closest_vge != NULL && closest_idx < FLT_MAX-2) {
			closest_vge->mouseover = 1;
			vg->redraw++;
		}
	}
}

int
RG_SketchMouseWheel(RG_Tileview *tv, RG_TileElement *tel, int which)
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
				vg->redraw++;
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
select_tool(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	RG_TileviewTool *tvt = argv[2].p;

	RG_TileviewSelectTool(tv, tvt);
}

static void
select_tool_tbar(int argc, union evarg *argv)
{
	AG_Button *btn = argv[0].p;
	AG_Toolbar *tbar = argv[1].p;
	RG_Tileview *tv = argv[2].p;
	RG_TileviewTool *tvt = argv[3].p;

	AG_ToolbarSelectUnique(tbar, btn);
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
	AG_MenuInit(me);

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

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1, 0);
	TAILQ_FOREACH(tvt, &tv->tools, tools) {
		if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
			continue;
		}
		AG_ToolbarAddButton(tbar, 0, tvt->ops->icon >= 0 ?
		    AGICON(tvt->ops->icon) : NULL, 1, 0,
		    select_tool_tbar, "%p,%p,%p", tbar, tv, tvt);
	}
	return (tbar);
}

