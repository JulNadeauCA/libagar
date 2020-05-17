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

/*
 * Sketch feature using VG(3).
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/vg.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>
#include <agar/map/rg_texsel.h>
#include <agar/map/rg_icons.h>
#include <agar/map/rg_math.h>

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

	sk->vg = Malloc(sizeof(VG));
	VG_Init(sk->vg, VG_ANTIALIAS | VG_ALPHA);

	sk->ublks = Malloc(sizeof(RG_SketchUndoBlk));
	sk->nublks = 1;
	sk->curblk = 0;
	RG_SketchBeginUndoBlk(sk);
	sk->ublks[0].mods = Malloc(sizeof(RG_SketchMod));
	sk->ublks[0].nmods = 0;
}

void
RG_SketchScale(RG_Sketch *sk, int w, int h, float scale, int x, int y)
{
	VG *vg = sk->vg;
#if 0
	float xoffs = (float)x/(float)RG_TILESZ/scale;
	float yoffs = (float)y/(float)RG_TILESZ/scale;
#endif
	AG_Rect r;

	r.w = (w == -1) ? vg->rDst.w : w;
	r.h = (h == -1) ? vg->rDst.h : h;

	VG_Scale(vg, r.w, r.h, scale*RG_TILESZ);
#if 0
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vge->vtx[i].x += xoffs;
			vge->vtx[i].y += yoffs;
		}
	}
#endif
}

void
RG_SketchDestroy(RG_Sketch *sk)
{
	int i;

	VG_Destroy(sk->vg);

	for (i = 0; i < sk->nublks; i++) {
		Free(sk->ublks[i].mods);
	}
	Free(sk->ublks);
}

int
RG_SketchLoad(RG_Sketch *sk, AG_DataSource *buf)
{
	int vgflags;
	
	AG_CopyString(sk->name, buf, sizeof(sk->name));
	sk->flags = (int)AG_ReadUint32(buf);
	vgflags = (int)AG_ReadUint32(buf);

	sk->vg = VG_New(vgflags);
	if (VG_Load(sk->vg, buf) == -1) {
		VG_Destroy(sk->vg);
		Free(sk->vg);
		return (-1);
	}
	VG_Rasterize(sk->vg);
	return (0);
}

void
RG_SketchSave(RG_Sketch *sk, AG_DataSource *buf)
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
	AG_Rect rd;

	AG_FillRect(vg->su, NULL, vg->fillColor);
	
	if (vg->flags & VG_VISGRID)
		VG_DrawGrid(vg);
#ifdef AG_DEBUG
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
UpdateScale(AG_Event *_Nonnull event)
{
	RG_Tileview *tv = RG_TILEVIEW_PTR(1);
	RG_TileElement *tel = AG_PTR(2);
	RG_Sketch *sk = tel->tel_sketch.sk;

	RG_SketchScale(sk, -1, -1, tel->tel_sketch.scale, 0, 0);

	tv->tile->flags |= RG_TILE_DIRTY;
}

AG_Window *
RG_SketchEdit(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Sketch %s"), sk->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL | AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_BindPointer(pal, "pixel-format", &tv->ts->fmt);
		AG_BindFloat(pal, "hue", &sk->h);
		AG_BindFloat(pal, "saturation", &sk->s);
		AG_BindFloat(pal, "value", &sk->v);
		AG_BindFloat(pal, "alpha", &sk->a);
	}

	ntab = AG_NotebookAdd(nb, _("Settings"), AG_BOX_VERT);
	{
		AG_MSpinbutton *msb;
		AG_Numerical *num;

		num = AG_NumericalNewFltR(ntab, 0, NULL, _("Scale: "),
		    &tel->tel_sketch.scale, 1e-4, 1e4);
		AG_NumericalSetIncrement(num, 0.1);
		AG_SetEvent(num, "numerical-changed",
		    UpdateScale, "%p,%p", tv, tel);
		
		msb = AG_MSpinbuttonNew(ntab, 0, ",", _("Coordinates: "));
		AG_BindInt(msb, "xvalue", &tel->tel_sketch.x);
		AG_BindInt(msb, "yvalue", &tel->tel_sketch.y);

		AG_SetEvent(num, "numerical-changed",
		    UpdateScale, "%p,%p", tv, tel);
		
		AG_NumericalNewIntR(ntab, 0, NULL, _("Overall alpha: "),
		    &tel->tel_sketch.alpha, 0, 255);
	}
	return (win);
}

static void
PollStyles(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
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
RG_SketchEditElement(RG_Tileview *tv, RG_TileElement *tel, VG_Element *vge)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	const VG_ElementOps *ops;
	AG_Combo *com;
	AG_Label *lbl;
	
	if ((win = AG_WindowNewNamed(0, "%s-%p-%p", sk->name, tel, vge))
	    == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Sketch element (%s)"), sk->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 0);

	ops = vgElementTypes[vge->type];
	AG_LabelNew(win, 0, _("Element type: %s"), ops->name);
	lbl = AG_LabelNewPolled(win, AG_LABEL_HFILL, _("Vertices: %u"),
	    &vge->nvtx);
	AG_LabelSizeHint(lbl, 1, _("Vertices: 00000000"));
	
	AG_NumericalNewIntR(win, 0, NULL, _("Layer: "), &vge->layer, 0, 255);

	com = AG_ComboNew(win, AG_COMBO_POLL|AG_COMBO_HFILL, _("Style: "));
	AG_SetEvent(com->list, "tlist-poll", PollStyles, "%p", vg);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_BindPointer(pal, "pixel-format", &sk->vg->fmt);
		AG_BindUint32(pal, "pixel", &vge->color);
	}
	ntab = AG_NotebookAdd(nb, _("Line style"), AG_BOX_VERT);
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
	
		AG_LabelNewS(ntab, 0, _("Line style: "));
		AG_RadioNewUint(ntab, AG_RADIO_HFILL, line_styles,
		    &vge->line_st.style);
		
		AG_LabelNewS(ntab, 0, _("Endpoint style: "));
		AG_RadioNewUint(ntab, AG_RADIO_HFILL, endpoint_styles,
		    &vge->line_st.endpoint_style);

		AG_NumericalNewUint16(ntab, 0, NULL, _("Stipple pattern: "),
		    &vge->line_st.stipple);
		AG_NumericalNewUint8R(ntab, 0, "px", _("Thickness: "),
		    &vge->line_st.thickness, 1, 20);
		AG_NumericalNewUint8R(ntab, 0, "px", _("Miter length: "),
		    &vge->line_st.miter_len, 1, 20);
	}
	ntab = AG_NotebookAdd(nb, _("Filling style"), AG_BOX_VERT);
	{
		const char *fill_styles[] = {
			N_("None"),
			N_("Solid"),
			N_("Textured"),
			NULL
		};
		RG_TextureSelector *texsel;

		AG_LabelNewS(ntab, 0, _("Filling style: "));
		AG_RadioNewUint(ntab, AG_RADIO_HFILL, fill_styles,
		    &vge->fill_st.style);
			
		AG_LabelNewS(ntab, 0, _("Texture: "));
		texsel = RG_TextureSelectorNew(ntab, tv->ts, 0);
		WIDGET(texsel)->flags |= AG_WIDGET_HFILL|AG_WIDGET_VFILL;
		AG_BindString(texsel, "texture-name", vge->fill_st.texture,
		    sizeof(vge->fill_st.texture));
	}

	return (win);
}

void
RG_SketchBeginUndoBlk(RG_Sketch *sk)
{
	RG_SketchUndoBlk *ublk;

	while (sk->nublks > sk->curblk+1) {
		ublk = &sk->ublks[sk->nublks-1];
		Free(ublk->mods);
		sk->nublks--;
	}

	sk->ublks = Realloc(sk->ublks, ++sk->nublks*sizeof(RG_SketchUndoBlk));
	sk->curblk++;

	ublk = &sk->ublks[sk->curblk];
	ublk->mods = Malloc(sizeof(RG_SketchMod));
	ublk->nmods = 0;
}

void
RG_SketchUndo(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
/*	RG_SketchUndoBlk *ublk = &sk->ublks[sk->curblk]; */
/*	int i; */

	if (sk->curblk-1 <= 0)
		return;

#if 0
	for (i = 0; i < ublk->nmods; i++) {
		RG_SketchMod *mod = &ublk->mods[i];
	}
#endif
	sk->curblk--;
	tv->tile->flags |= RG_TILE_DIRTY;
}

void
RG_SketchRedo(RG_Tileview *tv, RG_TileElement *tel)
{
	/* TODO */
}

static void
UpdateCircleRadius(AG_Event *_Nonnull event)
{
	VG_Element *vge = AG_PTR(2);

	vge->vg_circle.radius = Sqrt(
	    pow(vge->vtx[1].x - vge->vtx[0].x, 2) +
	    pow(vge->vtx[1].y - vge->vtx[0].y, 2));
}

static void
UpdateCircleVertex(AG_Event *_Nonnull event)
{
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
		ctrl->motion = AG_SetEvent(tv, NULL, UpdateCircleVertex,
		    "%p,%p", vg, vge);
		
		ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[1].x, &vge->vtx[1].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = NULL;
		ctrl->motion = AG_SetEvent(tv, NULL, UpdateCircleRadius,
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
	AG_Driver *drv = WIDGET(tv)->drv;

	/* XXX odd way to handle this */
	TAILQ_FOREACH(win, &drv->visible, windows) {
		Snprintf(name, sizeof(name), "win-%s-%p-%p", sk->name, tel,
		    vge);
		if (strcmp(name, OBJECT(win)->name) == 0) {
			AG_ObjectDetach(win);
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
RG_SketchButtondown(RG_Tileview *tv, RG_TileElement *tel, float x, float y,
    int button)
{
	RG_Sketch *sk = tel->tel_sketch.sk;
	VG *vg = sk->vg;

	if (button == AG_MOUSE_MIDDLE) {
		int x, y;

		AG_MouseGetState(WIDGET(tv)->drv->mouse, &x, &y);
		RG_SketchOpenMenu(tv, x, y);
		return;
	} else if (button == AG_MOUSE_RIGHT) {
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
		float idx, closest_idx = AG_FLT_MAX;
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
		if (closest_vge != NULL && closest_idx < AG_FLT_MAX-2) {
			if (closest_vge->selected) {
				RG_SketchUnselect(tv, tel, closest_vge);
			} else {
				AG_Window *pwin = AG_ParentWindow(tv);
				AG_Window *win;

				if (!(AG_GetModState(tv) & AG_KEYMOD_CTRL)) {
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
					AG_WidgetFocus(tv);
					AG_WindowFocus(win);
				}
			}
		}
	}
}

void
RG_SketchButtonup(RG_Tileview *tv, RG_TileElement *tel, float x, float y,
    int button)
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
RG_SketchMotion(RG_Tileview *tv, RG_TileElement *tel, float x, float y,
    float xrel, float yrel, int state)
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
		float idx, closest_idx = AG_FLT_MAX;
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
		if (closest_vge != NULL && closest_idx < AG_FLT_MAX-2) {
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
	case AG_KEY_DELETE:
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
SelectTool(AG_Event *_Nonnull event)
{
	RG_Tileview *tv = RG_TILEVIEW_PTR(1);
	RG_TileviewTool *tvt = AG_PTR(2);

	RG_TileviewSelectTool(tv, tvt);
}

static void
SelectToolFromToolbar(AG_Event *_Nonnull event)
{
	AG_Button *btn = AG_BUTTON_SELF();
	AG_Toolbar *tbar = AG_TOOLBAR_PTR(1);
	RG_Tileview *tv = RG_TILEVIEW_PTR(2);
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

	me = tv->tv_sketch.menu = AG_MenuNew(NULL, 0);
	mi = tv->tv_sketch.menu_item = AG_MenuNode(me, NULL, NULL);
	{
		RG_TileviewTool *tvt;
		AG_MenuItem *m_tool;

		m_tool = AG_MenuAction(mi, _("Tools"), rgIconEdit.s, NULL,NULL);
		TAILQ_FOREACH(tvt, &tv->tools, tools) {
			if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
				continue;
			}
			AG_MenuAction(m_tool, _(tvt->ops->name),
			    (tvt->ops->icon != NULL) ? tvt->ops->icon->s : NULL,
			    SelectTool, "%p,%p", tv, tvt);
		}

		AG_MenuSeparator(mi);
	
		AG_MenuIntFlags(mi, _("Show sketch origin"), rgIconOrigin.s,
		    &sk->vg->flags, VG_VISORIGIN, 0);
		AG_MenuIntFlags(mi, _("Show sketch grid"), rgIconGrid.s,
		    &sk->vg->flags, VG_VISGRID, 0);
		AG_MenuIntFlags(mi, _("Show sketch extents"), NULL,
		    &sk->vg->flags, VG_VISBBOXES, 0);

		AG_MenuSeparator(mi);
		
		RG_TileviewGenericMenu(tv, mi);
	}
	tv->tv_sketch.menu->itemSel = mi;
	tv->tv_sketch.menu_win = AG_MenuExpand(tv, me, mi, x, y);
}

void
RG_SketchCloseMenu(RG_Tileview *tv)
{
	AG_Menu *me = tv->tv_sketch.menu;
	AG_MenuItem *mi = tv->tv_sketch.menu_item;

	AG_MenuCollapse(me, mi);
	AG_ObjectDestroy(me);
	tv->tv_sketch.menu = NULL;
	tv->tv_sketch.menu_item = NULL;
	tv->tv_sketch.menu_win = NULL;
}

AG_Toolbar *
RG_SketchToolbar(RG_Tileview *tv, RG_TileElement *tel)
{
	AG_Toolbar *tbar;
	RG_TileviewTool *tvt;

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1,
	    AG_TOOLBAR_STICKY);
	TAILQ_FOREACH(tvt, &tv->tools, tools) {
		if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
			continue;
		}
		AG_ToolbarButtonIcon(tbar,
		    tvt->ops->icon != NULL ? tvt->ops->icon->s : NULL, 0,
		    SelectToolFromToolbar, "%p,%p,%p", tbar, tv, tvt);
	}
	return (tbar);
}

