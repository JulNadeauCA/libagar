/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Visualization widget.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/primitive.h>
#include <agar/gui/opengl.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/vg_tools.h>
#include <agar/vg/tools.h>

#include <stdarg.h>
#include <string.h>

static const float scaleFactors[] = {
	1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f,
	12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f,
	30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 100.0f,
	200.0f, 300.0f, 400.0f, 600.0f, 700.0f, 800.0f, 900.0f
};
const int nScaleFactors = sizeof(scaleFactors)/sizeof(scaleFactors[0]);

static void DrawGrid(VG_View *_Nonnull, const VG_Grid *_Nonnull);
static void DrawNode(VG *_Nonnull, VG_Node *_Nonnull, VG_View *_Nonnull);
#ifdef AG_DEBUG
static void DrawNodeExtent(VG_Node *_Nonnull, VG_View *_Nonnull);
#endif

VG_View *
VG_ViewNew(void *parent, VG *vg, Uint flags)
{
	VG_View *vv;

	vv = Malloc(sizeof(VG_View));
	AG_ObjectInit(vv, &vgViewClass);
	
	if (flags & VG_VIEW_HFILL) { WIDGET(vv)->flags |= AG_WIDGET_HFILL; }
	if (flags & VG_VIEW_VFILL) { WIDGET(vv)->flags |= AG_WIDGET_VFILL; }
	vv->flags |= flags;

	vv->vg = vg;

	AG_ObjectAttach(parent, vv);
	return (vv);
}

/* Return the Point nearest to vPos. */
void *
VG_NearestPoint(VG_View *vv, VG_Vector vPos, void *ignore)
{
	float prox, proxNearest = AG_FLT_MAX;
	VG_Node *vn, *vnNearest = NULL;
	VG_Vector v;

	TAILQ_FOREACH(vn, &vv->vg->nodes, list) {
		if (vn->ops->pointProximity == NULL ||
		    vn == ignore ||
		    !VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < vv->grid[0].ival) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	return (vnNearest);
}

/* Return the entity nearest to vPos. */
void *
VG_Nearest(VG_View *vv, VG_Vector vPos)
{
	VG *vg = vv->vg;
	float prox, proxNearest;
	VG_Node *vn, *vnNearest;
	VG_Vector v;

	/* Prioritize points at a fixed distance. */
	proxNearest = AG_FLT_MAX;
	vnNearest = NULL;
	TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (!VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox <= vv->pointSelRadius) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	if (vnNearest != NULL)
		return (vnNearest);

	/* Fallback to a general query. */
	proxNearest = AG_FLT_MAX;
	vnNearest = NULL;
	TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (vn->ops->pointProximity == NULL) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < proxNearest) {
			proxNearest = prox;
			vnNearest = vn;
		}
	}
	return (vnNearest);
}

/* Highlight and return the Point nearest to vPos. */
void *
VG_HighlightNearestPoint(VG_View *vv, VG_Vector vPos, void *ignore)
{
	VG *vg = vv->vg;
	float prox, proxNearest = AG_FLT_MAX;
	VG_Node *vn, *vnNearest = NULL;
	VG_Vector v;

	TAILQ_FOREACH(vn, &vg->nodes, list) {
		vn->flags &= ~(VG_NODE_MOUSEOVER);
		if (vn->ops->pointProximity == NULL ||
		    vn == ignore ||
		    !VG_NodeIsClass(vn, "Point")) {
			continue;
		}
		v = vPos;
		prox = vn->ops->pointProximity(vn, vv, &v);
		if (prox < vv->grid[0].ival) {
			if (prox < proxNearest) {
				proxNearest = prox;
				vnNearest = vn;
			}
		}
	}
	return (vnNearest);
}

/*
 * Snap specified coordinates to the VG_View grid.
 * The VG_View must be locked.
 */
void
VG_ApplyConstraints(VG_View *vv, VG_Vector *pos)
{
	VG_Grid *grid;
	int ival;
	float r, ival_2;

	if (vv->snap_mode != VG_GRID) {
		return;
	}
	grid = &vv->grid[0];
	ival = grid->ival;
	ival_2 = (float)(ival >> 1);

	r = VG_Mod(pos->x, ival);
	pos->x -= r;
	if (r > ival_2)       { pos->x += ival; }
	else if (r < -ival_2) { pos->x -= ival; }
	
	r = VG_Mod(pos->y, ival);
	pos->y -= r;
	if (r > ival_2)       { pos->y += ival; }
	else if (r < -ival_2) { pos->y -= ival; }
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	VG_Tool *tool = VG_CURTOOL(vv);
	VG_Vector vCt;
	const int xCurs = AG_INT(1);
	const int yCurs = AG_INT(2);
	const float xRel = (float)AG_INT(3);
	const float yRel = (float)AG_INT(4);
	const int state = AG_INT(5);
	float x, y;

	if (vv->mouse.panning) {
		vv->x += (float)xRel;
		vv->y += (float)yRel;
		AG_Redraw(vv);
		return;
	}
	if (vv->vg == NULL)
		return;

	VG_GetVGCoords(vv, xCurs,yCurs, &vCt);
	x = vCt.x;
	y = vCt.y;

	if (tool != NULL && tool->ops->mousemotion != NULL) {
		if (!(tool->ops->flags & VG_MOUSEMOTION_NOSNAP)) {
			VG_ApplyConstraints(vv, &vCt);
		}
		tool->vCursor = vCt;
		AG_Redraw(vv);
		if (tool->ops->mousemotion(tool, vCt,
		    VG_ScaleVector(1.0f/vv->scale,VGVECTOR(xRel,yRel)),
		    state) == 1) {
			vv->mouse.x = x;
			vv->mouse.y = y;
			return;
		}
	} else {
		vv->mouse.x = x;
		vv->mouse.y = y;
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	VG_Tool *tool = VG_CURTOOL(vv);
	const int button = AG_INT(1);
	const int xCurs = AG_INT(2);
	const int yCurs = AG_INT(3);
	float x, y;
	VG_Vector vCt;
	
	if (vv->vg == NULL ||
	    AG_ExecMouseAction(vv, AG_ACTION_ON_BUTTONDOWN, button, xCurs, yCurs) == 1)
		return;
	
	VG_GetVGCoords(vv, xCurs,yCurs, &vCt);
	x = vCt.x;
	y = vCt.y;

	if (!AG_WidgetIsFocused(vv)) {
		AG_WidgetFocus(vv);
	}
	vv->mouse.x = x;
	vv->mouse.y = y;

	if (tool != NULL && tool->ops->mousebuttondown != NULL) {
		if (!(tool->ops->flags & VG_BUTTONDOWN_NOSNAP)) {
			VG_ApplyConstraints(vv, &vCt);
		}
		AG_Redraw(vv);
		if (tool->ops->mousebuttondown(tool, vCt, button) == 1)
			return;
	}
	if (vv->btndown_ev != NULL) {
		AG_PostEventByPtr(vv, vv->btndown_ev, "%i,%f,%f", button, x,y);
		AG_Redraw(vv);
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	VG_Tool *tool = VG_CURTOOL(vv);
	const int button = AG_INT(1);
	const int xCurs = AG_INT(2);
	const int yCurs = AG_INT(3);
	float x, y;
	VG_Vector vCt;
	
	if (vv->vg == NULL ||
	    AG_ExecMouseAction(vv, AG_ACTION_ON_BUTTONUP, button, xCurs, yCurs) == 1)
		return;
	
	VG_GetVGCoords(vv, xCurs,yCurs, &vCt);
	x = vCt.x;
	y = vCt.y;
	
	if (tool != NULL && tool->ops->mousebuttonup != NULL) {
		if (!(tool->ops->flags & VG_BUTTONUP_NOSNAP)) {
			VG_ApplyConstraints(vv, &vCt);
		}
		AG_Redraw(vv);
		if (tool->ops->mousebuttonup(tool, vCt, button) == 1)
			return;
	}
	if (vv->btnup_ev != NULL) {
		AG_PostEventByPtr(vv, vv->btnup_ev, "%i,%f,%f", button, x, y);
		AG_Redraw(vv);
	}
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	VG_Tool *tool = VG_CURTOOL(vv);
	const int sym = AG_INT(1);
	const int mod = AG_INT(2);
	const AG_Char ch = AG_CHAR(3);
	VG_ToolCommand *cmd;
	
	if (vv->vg == NULL)
		return;
	if (AG_ExecKeyAction(vv, AG_ACTION_ON_KEYDOWN, sym, mod))
		return;

	if (tool == NULL) {
		return;
	}
	AG_Redraw(vv);
	if (tool->ops->keydown != NULL &&
	    tool->ops->keydown(tool, sym, mod, ch) == 1) {
		return;
	}
	TAILQ_FOREACH(cmd, &tool->cmds, cmds) {
		if (cmd->kSym == sym &&
		    (cmd->kMod == AG_KEYMOD_NONE || mod & cmd->kMod)) {
			AG_PostEventByPtr(tool->vgv, cmd->fn, "%p", tool);
			AG_Redraw(vv);
		}
	}
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	VG_Tool *tool = VG_CURTOOL(vv);
	const int sym = AG_INT(1);
	const int mod = AG_INT(2);
	const AG_Char ch = AG_CHAR(3);
	
	if (vv->vg == NULL)
		return;
	if (AG_ExecKeyAction(vv, AG_ACTION_ON_KEYUP, sym, mod))
		return;
	
	AG_Redraw(vv);

	if (tool != NULL &&
	    tool->ops->keyup != NULL)
		tool->ops->keyup(tool, sym, mod, ch);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();

	vv->x = WIDTH(vv)/2.0f;
	vv->y = HEIGHT(vv)/2.0f;
}

static void
UpdateGridIntervals(VG_View *vv)
{
	int i;

	for (i = 0; i < vv->nGrids; i++) {
		VG_Grid *grid = &vv->grid[i];

		grid->ivalView = (int)VG_Rint(((float)grid->ival)/vv->wPixel);

		if (grid->ivalView < 6) {
			grid->flags |= VG_GRID_UNDERSIZE;
		} else {
			grid->flags &= ~(VG_GRID_UNDERSIZE);
		}
	}
}

static void
ZoomInOut(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	int inc = AG_INT(1);
		
	VG_ViewSetScalePreset(vv, (vv->scaleIdx += inc));
	VG_Status(vv, _("Scale: 1:%.0f"), vv->scale);
}

static void
ZoomInMax(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
		
	VG_ViewSetScalePreset(vv, nScaleFactors-1);
	VG_Status(vv, _("Scale: 1:%.0f"), vv->scale);
}

static void
ZoomOutMax(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
		
	VG_ViewSetScalePreset(vv, 0);
	VG_Status(vv, _("Scale: 1:%.0f"), vv->scale);
}

static void
SetScale(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_SELF();
	int n = AG_INT(1);
		
	VG_ViewSetScalePreset(vv, n);
	VG_Status(vv, _("Scale: 1:%.0f"), vv->scale);
}

static void
Init(void *_Nonnull obj)
{
	VG_View *vv = obj;

	WIDGET(vv)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_TEXT;

	vv->flags = 0;
	vv->scaleIdx = 0;
	vv->vg = NULL;
	vv->x = 0.0f;
	vv->y = 0.0f;
	vv->scale = scaleFactors[0];
	vv->scaleMin = 1.0f;
	vv->scaleMax = 1e6f;
	vv->wPixel = 1.0f;
	vv->snap_mode = VG_GRID;
	vv->nGrids = 0;

	vv->draw_ev = NULL;
	vv->scale_ev = NULL;
	vv->keydown_ev = NULL;
	vv->btndown_ev = NULL;
	vv->keyup_ev = NULL;
	vv->btnup_ev = NULL;
	vv->motion_ev = NULL;

	vv->mouse.x = 0.0f;
	vv->mouse.y = 0.0f;
	vv->mouse.panning = 0;

	vv->curtool = NULL;
	vv->deftool = NULL;
	vv->status[0] = '\0';
	vv->tCache = AG_TextCacheNew(vv, 64, 2);
	vv->editAreas = NULL;
	vv->nEditAreas = 0;
	vv->r.x = 0;
	vv->r.y = 0;
	vv->r.w = 0;
	vv->r.h = 0;
	TAILQ_INIT(&vv->tools);

	VG_ViewSetGrid(vv, 0, VG_GRID_POINTS, 8, VG_GetColorRGB(100,100,100));
	VG_ViewSetScale(vv, 0);
	
	AG_AddEvent(vv, "widget-shown", OnShow, NULL);
	AG_SetEvent(vv, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(vv, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(vv, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(vv, "key-down", KeyDown, NULL);
	AG_SetEvent(vv, "key-up", KeyUp, NULL);

	AG_ActionSetInt(vv, "Enable panning",	&vv->mouse.panning, 1);
	AG_ActionSetInt(vv, "Disable panning",	&vv->mouse.panning, 0);
	AG_ActionFn(vv, "Zoom in",		ZoomInOut, "%i", 1);
	AG_ActionFn(vv, "Zoom out",		ZoomInOut, "%i", -1);
	AG_ActionFn(vv, "Zoom in maximum",	ZoomInMax, NULL);
	AG_ActionFn(vv, "Zoom out maximum",	ZoomOutMax, NULL);
	AG_ActionFn(vv, "Scale 1:1",		SetScale, "%i", 0);
	AG_ActionFn(vv, "Scale 1:2",		SetScale, "%i", 1);
	AG_ActionFn(vv, "Scale 1:3",		SetScale, "%i", 2);
	AG_ActionFn(vv, "Scale 1:9",		SetScale, "%i", 8);

	AG_ActionOnButtonDown(vv, AG_MOUSE_MIDDLE,		"Enable panning");
	AG_ActionOnButtonUp(vv,	  AG_MOUSE_MIDDLE,		"Disable panning");
	AG_ActionOnButton(vv,     AG_MOUSE_WHEELUP,		"Zoom in");
	AG_ActionOnButton(vv,     AG_MOUSE_WHEELDOWN,		"Zoom out");
	AG_ActionOnKeyDown(vv,    AG_KEY_1, AG_KEYMOD_ANY,	"Scale 1:1");
	AG_ActionOnKeyDown(vv,    AG_KEY_2, AG_KEYMOD_ANY,	"Scale 1:2");
	AG_ActionOnKeyDown(vv,    AG_KEY_3, AG_KEYMOD_ANY,	"Scale 1:3");
	AG_ActionOnKeyDown(vv,    AG_KEY_9, AG_KEYMOD_ANY,	"Scale 1:9");
}

static void
Destroy(void *_Nonnull obj)
{
	VG_View *vv = obj;

	AG_TextCacheDestroy(vv->tCache);
}

/* Change the VG being displayed. */
void
VG_ViewSetVG(VG_View *vv, VG *vg)
{
	AG_ObjectLock(vv);
	if (vv->vg != vg) {
		vv->vg = vg;
		VG_ViewSelectTool(vv, NULL, NULL);
	}
	AG_ObjectUnlock(vv);
	AG_Redraw(vv);
}

/* Set the snapping constraint. */
void
VG_ViewSetSnapMode(VG_View *vv, enum vg_snap_mode mode)
{
	vv->snap_mode = mode;
}

/* Set the parameters of the specified grid. */
void
VG_ViewSetGrid(VG_View *vv, int idx, enum vg_grid_type type, int ival,
    VG_Color color)
{
	if (idx+1 > VG_GRIDS_MAX)
		AG_FatalError("Too many grids");

	AG_ObjectLock(vv);
	vv->grid[idx].type = type;
	vv->grid[idx].ival = ival;
	vv->grid[idx].ivalView = 0;
	vv->grid[idx].color = color;
	vv->grid[idx].flags = 0;
	if (idx == 0) {
		vv->pointSelRadius = vv->grid[0].ival/2;
	}
	if (idx >= vv->nGrids) {
		vv->nGrids = idx+1;
	}
	UpdateGridIntervals(vv);
	AG_ObjectUnlock(vv);
	AG_Redraw(vv);
}

/* Register a "draw" callback. */
void
VG_ViewDrawFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->draw_ev = AG_SetEvent(vv, "Draw", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->draw_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

/* Register a "resize" callback. */
void
VG_ViewScaleFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->scale_ev = AG_SetEvent(vv, "Scale", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->scale_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

/* Register a keydown callback. */
void
VG_ViewKeydownFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->keydown_ev = AG_SetEvent(vv, "key-down", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->keydown_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

/* Register a keyup callback. */
void
VG_ViewKeyupFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->keyup_ev = AG_SetEvent(vv, "key-up", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->keyup_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

/* Register a mousebuttondown callback. */
void
VG_ViewButtondownFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->btndown_ev = AG_SetEvent(vv, "ButtonDown", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->btndown_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

/* Register a mousebuttonup callback. */
void
VG_ViewButtonupFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->btnup_ev = AG_SetEvent(vv, "mouse-button-up", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->btnup_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

/* Register a mousemotion callback. */
void
VG_ViewMotionFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(vv);
	vv->motion_ev = AG_SetEvent(vv, "mouse-motion", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(vv->motion_ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(vv);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	r->w = 320;
	r->h = 240;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	VG_View *vv = obj;

	if (a->w < 16 || a->h < 16) {
		return (-1);
	}
	vv->r.w = a->w;
	vv->r.h = a->h;
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	VG_View *vv = obj;
	VG *vg = vv->vg;
	VG_Tool *curtool = vv->curtool;
	AG_Color c;
	int su, i;

	if (vg == NULL)
		return;

	if (!(vv->flags & VG_VIEW_DISABLE_BG)) {
		c = VG_MapColorRGBA(vg->fillColor);
		AG_DrawRect(vv, &vv->r, &c);
	}
	
	AG_PushClipRect(vv, &vv->r);

	if (vv->flags & VG_VIEW_GRID) {
		for (i = 0; i < vv->nGrids; i++)
			DrawGrid(vv, &vv->grid[i]);
	}

	AG_ObjectLock(vg);

	if (curtool && curtool->ops->predraw)
		curtool->ops->predraw(curtool, vv);
	if (vv->draw_ev)
		vv->draw_ev->fn(vv->draw_ev);
	if (curtool && curtool->ops->postdraw)
		curtool->ops->postdraw(curtool, vv);

	DrawNode(vg, vg->root, vv);

	AG_ObjectUnlock(vg);

	if (vv->status[0] != '\0') {
		AG_PushTextState();
		AG_PushBlendingMode(vv, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
		AG_TextColor(&WCOLOR(vv, TEXT_COLOR));
		if ((su = AG_TextCacheGet(vv->tCache, vv->status)) != -1) {
			AG_WidgetBlitSurface(vv, su,
			    0,
			    HEIGHT(vv) - WSURFACE(vv,su)->h);
		}
		AG_PopBlendingMode(vv);
		AG_PopTextState();
	}
	AG_PopClipRect(vv);
}

static void
DrawNode(VG *_Nonnull vg, VG_Node *_Nonnull vn, VG_View *_Nonnull vv)
{
	VG_Node *vnChld;
	VG_Color colorSave;

	VG_PushMatrix(vg);
	VG_MultMatrix(&vg->T[vg->nT-1], &vn->T);

	VG_FOREACH_CHLD(vnChld, vn, vg_node)
		DrawNode(vg, vnChld, vv);
#ifdef AG_DEBUG
	if (vv->flags & VG_VIEW_EXTENTS)
		DrawNodeExtent(vn, vv);
#endif
	colorSave = vn->color;
	if (vn->flags & VG_NODE_SELECTED) {
		VG_BlendColors(&vn->color, vg->selectionColor);
	}
	if (vn->flags & VG_NODE_MOUSEOVER) {
		VG_BlendColors(&vn->color, vg->mouseoverColor);
	}
	vn->ops->draw(vn, vv);
	vn->color = colorSave;

	VG_PopMatrix(vg);
}

static void
DrawGrid(VG_View *_Nonnull vv, const VG_Grid *_Nonnull grid)
{
	int x, x0, y, ival;
	int x2, y2;

	if (grid->flags & (VG_GRID_HIDE | VG_GRID_UNDERSIZE))
		return;

	ival = grid->ivalView;
#ifdef HAVE_OPENGL
	if (AGDRIVER_CLASS(WIDGET(vv)->drv)->flags & AG_DRIVER_OPENGL) {
		x0 = WIDGET(vv)->rView.x1 + (int)vv->x % ival;
		y  = WIDGET(vv)->rView.y1 + (int)vv->y % ival;
		x2 = WIDGET(vv)->rView.x2;
		y2 = WIDGET(vv)->rView.y2;

		glBegin(GL_POINTS);
		glColor3ub(grid->color.r,
		           grid->color.g,
			   grid->color.b);
		for (; y < y2; y += ival) {
			for (x = x0; x < x2; x += ival)
				glVertex2i(x, y);
		}
		glEnd();
	} else
#endif /* !HAVE_OPENGL */
	{
		AG_Color c;

		x0 = (int)vv->x % ival;
		y  = (int)vv->y % ival;
		x2 = WIDGET(vv)->rView.x2;
		y2 = WIDGET(vv)->rView.y2;

		c = VG_MapColorRGB(grid->color);

		for (; y < y2; y += ival)
			for (x = x0; x < x2; x += ival)
				AG_PutPixel(vv, x,y, &c);
	}
}

#ifdef AG_DEBUG
static void
DrawNodeExtent(VG_Node *_Nonnull vn, VG_View *_Nonnull vv)
{
	AG_Rect r;
	VG_Vector a, b;
	AG_Color c;

	if (vn->ops->extent == NULL) {
		return;
	}
	vn->ops->extent(vn, vv, &a, &b);

	VG_GetViewCoords(vv, a, &r.x, &r.y);
	r.w = (int)((b.x - a.x)*vv->scale);
	r.h = (int)((b.y - a.y)*vv->scale);

	AG_ColorRGB_8(&c, 250,0,0);
	AG_DrawRectOutline(vv, &r, &c);
}
#endif /* AG_DEBUG */

void
VG_ViewSelectToolEv(AG_Event *event)
{
	VG_View *vv = VG_VIEW_PTR(1);
	VG_Tool *tool = AG_PTR(2);
	void *p = AG_PTR(3);

	VG_ViewSelectTool(vv, tool, p);
}

/* Select a new tool to use. */
void
VG_ViewSelectTool(VG_View *vv, void *pTool, void *p)
{
	VG_Tool *ntool = pTool;
	int i;

	AG_ObjectLock(vv);

	if (ntool == NULL || !(ntool->ops->flags & VG_NOEDITCLEAR)) {
		for (i = 0; i < vv->nEditAreas; i++) {
			AG_ObjectFreeChildren(vv->editAreas[i]);
			AG_WidgetUpdate(vv->editAreas[i]);
		}
	}
	if (vv->curtool != NULL) {
		if (ntool != NULL && ntool->ops == vv->curtool->ops) {
			goto out;
		}
		if (vv->curtool->editWin != NULL) {
			AG_WindowHide(vv->curtool->editWin);
		}
		if (vv->curtool->ops->deselected != NULL) {
			vv->curtool->ops->deselected(ntool, vv);
		}
		vv->curtool->selected = 0;
	}

	vv->curtool = ntool;
	if (ntool != NULL) {
		ntool->selected = 1;
		ntool->p = p;
		ntool->vgv = vv;

		if (ntool->editWin != NULL) {
			AG_WindowShow(ntool->editWin);
		}
		if (ntool->ops->edit != NULL && vv->nEditAreas > 0) {
			AG_ObjectAttach(vv->editAreas[0],
			    ntool->ops->edit(ntool,vv));
			AG_WidgetUpdate(vv->editAreas[0]);
		}

		VG_Status(vv, _("Tool: %s"), ntool->ops->name);
		if (ntool->ops->selected != NULL)
			ntool->ops->selected(ntool, vv);
	} else {
		VG_StatusS(vv, NULL);
	}
out:
	AG_ObjectUnlock(vv);
	AG_Redraw(vv);
}

/* VG_View must be locked */
VG_Tool *
VG_ViewFindTool(VG_View *vv, const char *name)
{
	VG_Tool *tool;

	TAILQ_FOREACH(tool, &vv->tools, tools) {
		if (strcmp(tool->ops->name, name) == 0)
			return (tool);
	}
	AG_SetError("No such tool: %s", name);
	return (NULL);
}

/* VG_View must be locked */
VG_Tool *
VG_ViewFindToolByOps(VG_View *vv, const VG_ToolOps *ops)
{
	VG_Tool *tool;

	TAILQ_FOREACH(tool, &vv->tools, tools) {
		if (tool->ops == ops)
			return (tool);
	}
	AG_SetError("No such tool: %p", ops);
	return (NULL);
}

/* Register a new VG_Tool class. */
VG_Tool *
VG_ViewRegTool(VG_View *vv, const VG_ToolOps *ops, void *p)
{
	VG_Tool *t;

	t = Malloc(ops->len);
	t->ops = ops;
	t->vgv = vv;
	t->p = p;

	AG_ObjectLock(vv);
	VG_ToolInit(t);
	TAILQ_INSERT_TAIL(&vv->tools, t, tools);
	AG_ObjectUnlock(vv);
	return (t);
}

/* Set the minimum allowed display scale factor. */
void
VG_ViewSetScaleMin(VG_View *vv, float scaleMin)
{
	AG_ObjectLock(vv);
	vv->scaleMin = MAX(scaleMin,1e-6f);
	AG_ObjectUnlock(vv);
}

/* Set the maximum allowed display scale factor. */
void
VG_ViewSetScaleMax(VG_View *vv, float scaleMax)
{
	AG_ObjectLock(vv);
	vv->scaleMax = scaleMax;
	AG_ObjectUnlock(vv);
}

/* Set the display scale factor explicitely. */
void
VG_ViewSetScale(VG_View *vv, float c)
{
	float scalePrev;

	AG_ObjectLock(vv);
	scalePrev = vv->scale;

	/* Set the specified scaling factor. */
	/* vv->scaleIdx = idx; XXX */
	vv->scale = c;
	if (vv->scale < vv->scaleMin) { vv->scale = vv->scaleMin; }
	if (vv->scale > vv->scaleMax) { vv->scale = vv->scaleMax; }
	
	/* Update all values dependent on VG's representation of a pixel. */
	vv->wPixel = 1.0/vv->scale;
	vv->x *= (vv->scale/scalePrev);
	vv->y *= (vv->scale/scalePrev);
	vv->pointSelRadius = vv->grid[0].ival >> 1;
	UpdateGridIntervals(vv);

	AG_ObjectUnlock(vv);
	AG_Redraw(vv);
}

/* Set the display scale factor by preset index. */
void
VG_ViewSetScalePreset(VG_View *vv, int idx)
{
	if (idx < 0) { idx = 0; }
	else if (idx >= nScaleFactors) { idx = nScaleFactors-1; }
	VG_ViewSetScale(vv, scaleFactors[idx]);
}

/* Set the specified tool as default. */
void
VG_ViewSetDefaultTool(VG_View *vv, VG_Tool *tool)
{
	AG_ObjectLock(vv);
	vv->deftool = tool;
	AG_ObjectUnlock(vv);
}

/* Set the status line text (C string). */
void
VG_StatusS(VG_View *vv, const char *s)
{
	AG_ObjectLock(vv);
	if (s != NULL) {
		Strlcpy(vv->status, s, sizeof(vv->status));
	} else {
		vv->status[0] = '\0';
	}
	AG_ObjectUnlock(vv);
	AG_Redraw(vv);
}

/* Set the status line text (format string). */
void
VG_Status(VG_View *vv, const char *fmt, ...)
{
	va_list ap;

	AG_ObjectLock(vv);
	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(vv->status, sizeof(vv->status), fmt, ap);
		va_end(ap);
	} else {
		vv->status[0] = '\0';
	}
	AG_ObjectUnlock(vv);
	AG_Redraw(vv);
}

/* Register a new container widget to associate with the tool. */
Uint
VG_AddEditArea(VG_View *vv, void *widget)
{
	Uint name;

	AG_ObjectLock(vv);
	vv->editAreas = Realloc(vv->editAreas, (vv->nEditAreas+1) *
	                                       sizeof(AG_Widget *));
	name = vv->nEditAreas++;
	vv->editAreas[name] = widget;
	AG_ObjectUnlock(vv);
	return (name);
}

/* Destroy widgets attached to all edit areas. */
void
VG_ClearEditAreas(VG_View *vv)
{
	Uint i;

	AG_ObjectLock(vv);
	for (i = 0; i < vv->nEditAreas; i++) {
		AG_Widget *editArea = vv->editAreas[i];

		AG_ObjectFreeChildren(editArea);
		AG_WidgetUpdate(editArea);
		AG_WidgetHideAll(editArea);
	}
	AG_ObjectUnlock(vv);
}

/* Create GUI elements for editing the parameters of vn. */
void
VG_EditNode(VG_View *vv, Uint editArea, VG_Node *vn)
{
	void *wEdit;

	if (vv->nEditAreas > editArea &&
	    vn->ops->edit != NULL &&
	    (wEdit = vn->ops->edit(vn, vv)) != NULL) {
		AG_ObjectFreeChildren(vv->editAreas[editArea]);
		AG_ObjectAttach(vv->editAreas[editArea], wEdit);
		AG_WidgetUpdate(vv->editAreas[editArea]);
		AG_WidgetShowAll(vv->editAreas[editArea]);
	}
}

/* Render a mapped surface at the specified coordinates and rotation. */
void
VG_DrawSurface(VG_View *vv, int x, int y, float degs, int su)
{
	AG_PushBlendingMode(vv, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

#ifdef HAVE_OPENGL
	if (AGDRIVER_CLASS(WIDGET(vv)->drv)->flags & AG_DRIVER_OPENGL) {
		glPushMatrix();
		glTranslatef((float)(WIDGET(vv)->rView.x1 + x),
		             (float)(WIDGET(vv)->rView.y1 + y),
			     0.0f);
		if (degs != 0.0f) {
			glRotatef(degs, 0.0f, 0.0f, 1.0f);
		}
		AG_WidgetBlitSurfaceGL(vv, su, WSURFACE(vv,su)->w,
		                               WSURFACE(vv,su)->h);
		glPopMatrix();
	} else
#endif /* HAVE_OPENGL */
	{
		AG_WidgetBlitSurface(vv, su, x, y);
	}
	AG_PopBlendingMode(vv);
}

AG_WidgetClass vgViewClass = {
	{
		"AG_Widget:VG_View",
		sizeof(VG_View),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
