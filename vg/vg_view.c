/*	$Csoft: vgview.c,v 1.2 2005/10/06 10:41:50 vedge Exp $	*/

/*
 * Copyright (c) 2005-2006 CubeSoft Communications, Inc.
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
#include <core/config.h>
#include <core/view.h>

#include <stdarg.h>
#include <string.h>

#include "vg.h"
#include "vg_view.h"

#include <gui/window.h>
#include <gui/primitive.h>

#define VG_VIEW_X(vv,px) VG_VECXF_NOSNAP((vv)->vg,(px)-(vv)->x)
#define VG_VIEW_Y(vv,py) VG_VECYF_NOSNAP((vv)->vg,(py)-(vv)->y)
#define VG_VIEW_X_SNAP(vv,px) VG_VECXF((vv)->vg,(px)-(vv)->x)
#define VG_VIEW_Y_SNAP(vv,py) VG_VECYF((vv)->vg,(py)-(vv)->y)
#define VG_VIEW_LEN(vv,len) VG_VECLENF((vv)->vg,(len))

VG_View *
VG_ViewNew(void *parent, VG *vg, Uint flags)
{
	VG_View *vgv;

	vgv = Malloc(sizeof(VG_View), M_OBJECT);
	VG_ViewInit(vgv, vg, flags);
	AG_ObjectAttach(parent, vgv);
	if (flags & VG_VIEW_FOCUS) {
		AG_WidgetFocus(vgv);
	}
	return (vgv);
}

static void
VG_ViewMotion(AG_Event *event)
{
	VG_View *vv = AG_SELF();
	VG_Tool *tool = VG_CURTOOL(vv);
	VG *vg = vv->vg;
	float x, y;
	float xrel = AG_INT(3);
	float yrel = AG_INT(4);
	int state = AG_INT(5);
	
	AG_MutexLock(&vg->lock);
	if (vv->mouse.panning) {
		vv->x += xrel;
		vv->y += yrel;
		goto out;
	}
	if (tool != NULL && tool->ops->mousemotion != NULL) {
		if (tool->ops->flags & VG_MOUSEMOTION_NOSNAP) {
			x = VG_VIEW_X(vv,AG_INT(1));
			y = VG_VIEW_Y(vv,AG_INT(2));
		} else {
			x = VG_VIEW_X_SNAP(vv,AG_INT(1));
			y = VG_VIEW_Y_SNAP(vv,AG_INT(2));
		}
		if (tool->ops->mousemotion(tool, x, y,
		    VG_VIEW_LEN(vv,xrel), VG_VIEW_LEN(vv,yrel), state) == 1) {
			vv->mouse.x = x;
			vv->mouse.y = y;
			goto out;
		}
		vv->mouse.x = x;
		vv->mouse.y = y;
	} else {
		vv->mouse.x = VG_VIEW_X(vv,AG_INT(1));
		vv->mouse.y = VG_VIEW_Y(vv,AG_INT(2));
	}
out:
	AG_MutexUnlock(&vg->lock);
}

static void
VG_ViewButtonDown(AG_Event *event)
{
	VG_View *vv = AG_SELF();
	VG *vg = vv->vg;
	VG_Tool *tool = VG_CURTOOL(vv);
	int button = AG_INT(1);
	VG_ToolMouseBinding *mb;
	float x = VG_VIEW_X(vv,AG_INT(2));
	float y = VG_VIEW_Y(vv,AG_INT(3));
	float xSn, ySn;
	
	AG_WidgetFocus(vv);
	AG_MutexLock(&vg->lock);
	vv->mouse.x = x;
	vv->mouse.y = y;
	switch (button) {
	case SDL_BUTTON_MIDDLE:
		vv->mouse.panning = 1;
		break;
	case SDL_BUTTON_WHEELDOWN:
		VG_Scale(vg, vg->rDst.w, vg->rDst.h, vg->scale-1.0);
		goto out;
	case SDL_BUTTON_WHEELUP:
		VG_Scale(vg, vg->rDst.w, vg->rDst.h, vg->scale+1.0);
		goto out;
	default:
		break;
	}
	if (tool != NULL && tool->ops->mousebuttondown != NULL) {
		if (tool->ops->flags & VG_BUTTONDOWN_NOSNAP) {
			xSn = x;
			ySn = y;
		} else {
			xSn = VG_VIEW_X_SNAP(vv,AG_INT(2));
			ySn = VG_VIEW_Y_SNAP(vv,AG_INT(3));
		}
		if (tool->ops->mousebuttondown(tool, xSn, ySn, button) == 1)
			goto out;
	}
	TAILQ_FOREACH(tool, &vv->tools, tools) {
		SLIST_FOREACH(mb, &tool->mbindings, mbindings) {
			if (mb->button != button) {
				continue;
			}
			tool->vgv = vv;
			if (mb->func(tool, button, 1, x, y, mb->arg) == 1)
				goto out;
		}
	}
	if (vv->btndown_ev != NULL)
		AG_PostEvent(NULL, vv, vv->btndown_ev->name, "%i,%f,%f", button,
		    x, y);
out:
	AG_MutexUnlock(&vg->lock);
}

static void
VG_ViewButtonUp(AG_Event *event)
{
	VG_View *vv = AG_SELF();
	VG *vg = vv->vg;
	VG_Tool *tool = VG_CURTOOL(vv);
	int button = AG_INT(1);
	float x = VG_VIEW_X(vv,AG_INT(2));
	float y = VG_VIEW_Y(vv,AG_INT(3));
	float xSn, ySn;
	VG_ToolMouseBinding *mb;

	AG_MutexLock(&vg->lock);
	if (tool != NULL && tool->ops->mousebuttonup != NULL) {
		if (tool->ops->flags & VG_BUTTONUP_NOSNAP) {
			xSn = x;
			ySn = y;
		} else {
			xSn = VG_VIEW_X_SNAP(vv,AG_INT(2));
			ySn = VG_VIEW_Y_SNAP(vv,AG_INT(3));
		}
		if (tool->ops->mousebuttonup(tool, xSn, ySn, button) == 1)
			goto out;
	}
	TAILQ_FOREACH(tool, &vv->tools, tools) {
		SLIST_FOREACH(mb, &tool->mbindings, mbindings) {
			if (mb->button != button) {
				continue;
			}
			tool->vgv = vv;
			if (mb->func(tool, button, 0, x, y, mb->arg) == 1)
				goto out;
		}
	}
	switch (button) {
	case SDL_BUTTON_MIDDLE:
		vv->mouse.panning = 0;
		goto out;
	default:
		break;
	}
	if (vv->btnup_ev != NULL)
		AG_PostEvent(NULL, vv, vv->btnup_ev->name, "%i,%f,%f", button,
		    x, y);
out:
	AG_MutexUnlock(&vg->lock);
}

void
VG_ViewInit(VG_View *vv, VG *vg, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING;

	if (flags & VG_VIEW_HFILL) wflags |= AG_WIDGET_HFILL;
	if (flags & VG_VIEW_VFILL) wflags |= AG_WIDGET_VFILL;

	AG_WidgetInit(vv, &vgViewOps, wflags);

	vv->vg = vg;
	vv->draw_ev = NULL;
	vv->scale_ev = NULL;
	vv->keydown_ev = NULL;
	vv->btndown_ev = NULL;
	vv->keyup_ev = NULL;
	vv->btnup_ev = NULL;
	vv->motion_ev = NULL;
	vv->x = 0;
	vv->y = 0;
	vv->status[0] = '\0';

	vv->mouse.x = 0;
	vv->mouse.y = 0;
	vv->mouse.panning = 0;
	vv->curtool = NULL;
	vv->deftool = NULL;
	TAILQ_INIT(&vv->tools);

	AG_SetEvent(vv, "window-mousemotion", VG_ViewMotion, NULL);
	AG_SetEvent(vv, "window-mousebuttondown", VG_ViewButtonDown, NULL);
	AG_SetEvent(vv, "window-mousebuttonup", VG_ViewButtonUp, NULL);
}

void
VG_ViewDrawFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->draw_ev = AG_SetEvent(vv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(vv->draw_ev, fmt);
}

void
VG_ViewScaleFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->scale_ev = AG_SetEvent(vv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(vv->scale_ev, fmt);
}

void
VG_ViewKeydownFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->keydown_ev = AG_SetEvent(vv, "window-keydown", fn, NULL);
	AG_EVENT_GET_ARGS(vv->keydown_ev, fmt);
}

void
VG_ViewKeyupFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->keyup_ev = AG_SetEvent(vv, "window-keyup", fn, NULL);
	AG_EVENT_GET_ARGS(vv->keyup_ev, fmt);
}

void
VG_ViewButtondownFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->btndown_ev = AG_SetEvent(vv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(vv->btndown_ev, fmt);
}

void
VG_ViewButtonupFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->btnup_ev = AG_SetEvent(vv, "window-mousebuttonup", fn, NULL);
	AG_EVENT_GET_ARGS(vv->btnup_ev, fmt);
}

void
VG_ViewMotionFn(VG_View *vv, AG_EventFn fn, const char *fmt, ...)
{
	vv->motion_ev = AG_SetEvent(vv, "window-mousemotion", fn, NULL);
	AG_EVENT_GET_ARGS(vv->motion_ev, fmt);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	r->w = 32;				/* XXX */
	r->h = 32;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	VG_View *vv = p;

	VG_Scale(vv->vg, a->w, a->h, vv->vg->scale);
	return (0);
}

static void
Draw(void *p)
{
	VG_View *vv = p;
	VG *vg = vv->vg;
	SDL_Surface *status;
	
	if (vv->draw_ev != NULL)
		vv->draw_ev->handler(vv->draw_ev);

	if (vg->flags & VG_DIRECT) {
		vg->rDst.x = WIDGET(vv)->cx+vv->x;
		vg->rDst.y = WIDGET(vv)->cy+vv->y;
		VG_Rasterize(vg);
		if (vg->flags & VG_VISGRID)
			VG_DrawGrid(vg);
	} else {
		VG_Rasterize(vg);
		AG_WidgetBlit(vv, vg->su, vv->x, vv->y);
	}

	AG_TextColor(TEXT_COLOR);
	status = AG_TextRender(vv->status);
	AG_WidgetBlit(vv, status, 0, WIDGET(vv)->h - status->h);
	SDL_FreeSurface(status);
}

void
VG_ViewSelectTool(VG_View *vv, VG_Tool *ntool, void *p)
{
	AG_Window *pwin;

	if (vv->curtool != NULL) {
		if (vv->curtool->trigger != NULL) {
			AG_WidgetSetBool(vv->curtool->trigger, "state", 0);
		}
		if (vv->curtool->win != NULL) {
			AG_WindowHide(vv->curtool->win);
		}
		if (vv->curtool->pane != NULL) {
			AG_Widget *wt;
			AG_Window *pwin;

			OBJECT_FOREACH_CHILD(wt, vv->curtool->pane,
			    ag_widget) {
				AG_ObjectDetach(wt);
				AG_ObjectDestroy(wt);
				Free(wt, M_OBJECT);
			}
			if ((pwin = AG_WidgetParentWindow(vv->curtool->pane))
			    != NULL) {
				AG_WindowUpdate(pwin);
			}
		}
		vv->curtool->vgv = NULL;
	}
	vv->curtool = ntool;

	if (ntool != NULL) {
		ntool->p = p;
		ntool->vgv = vv;

		if (ntool->trigger != NULL) {
			AG_WidgetSetBool(ntool->trigger, "state", 1);
		}
		if (ntool->win != NULL) {
			AG_WindowShow(ntool->win);
		}
#if 0
		if (ntool->pane != NULL && ntool->ops->edit != NULL) {
			AG_Window *pwin;

			ntool->ops->edit(ntool, ntool->pane);
			if ((pwin = AG_WidgetParentWindow(vv->curtool->pane))
			    != NULL) {
				AG_WindowUpdate(pwin);
			}
		}
#endif
		snprintf(vv->status, sizeof(vv->status), "Tool: %s",
		    ntool->ops->name);
	} else {
		vv->status[0] = '\0';
	}

//	if ((pwin = AG_WidgetParentWindow(vv)) != NULL) {
//		agView->winToFocus = pwin;
//		AG_WidgetFocus(vv);
//	}
}

VG_Tool *
VG_ViewFindTool(VG_View *vv, const char *name)
{
	VG_Tool *tool;

	TAILQ_FOREACH(tool, &vv->tools, tools) {
		if (strcmp(tool->ops->name, name) == 0)
			return (tool);
	}
	return (NULL);
}

VG_Tool *
VG_ViewFindToolByOps(VG_View *vv, const VG_ToolOps *ops)
{
	VG_Tool *tool;

	TAILQ_FOREACH(tool, &vv->tools, tools) {
		if (tool->ops == ops)
			return (tool);
	}
	return (NULL);
}

VG_Tool *
VG_ViewRegTool(VG_View *vv, const VG_ToolOps *ops, void *p)
{
	VG_Tool *t;

	t = Malloc(ops->len, M_MAPEDIT);
	t->ops = ops;
	t->vgv = vv;
	t->p = p;
	VG_ToolInit(t);
	TAILQ_INSERT_TAIL(&vv->tools, t, tools);
	return (t);
}

void
VG_ViewSetDefaultTool(VG_View *vv, VG_Tool *tool)
{
	vv->deftool = tool;
}

const AG_WidgetOps vgViewOps = {
	{
		"AG_Widget:VG_View",
		sizeof(VG_View),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
