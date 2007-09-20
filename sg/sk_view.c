/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Visualization/edition widget for SK(3) sketches.
 */ 

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>

#include <stdarg.h>
#include <string.h>

#include "sk.h"
#include "sk_view.h"

#include <gui/window.h>
#include <gui/primitive.h>

#define SK_VIEW_X(skv,px) ((SG_Real)(px - WIDGET(skv)->w/2)) / ((SG_Real)WIDGET(skv)->w/2.0)
#define SK_VIEW_Y(skv,py) ((SG_Real)(py - WIDGET(skv)->h/2)) / ((SG_Real)WIDGET(skv)->h/2.0)
#define SK_VIEW_X_SNAP(skv,px) (px)
#define SK_VIEW_Y_SNAP(skv,py) (py)
#define SK_VIEW_SCALE_X(skv) (skv)->mView.m[0][0]
#define SK_VIEW_SCALE_Y(skv) (skv)->mView.m[1][1]

SK_View *
SK_ViewNew(void *parent, SK *sk, Uint flags)
{
	SK_View *skv;

	skv = Malloc(sizeof(SK_View), M_OBJECT);
	SK_ViewInit(skv, sk, flags);
	AG_ObjectAttach(parent, skv);
	return (skv);
}

static void
mousemotion(AG_Event *event)
{
	SK_View *skv = AG_SELF();
	SK_Tool *tool = skv->curtool;
	SG_Vector vPos, vRel;
	SG_Matrix Ti;
	int x = AG_INT(1);
	int y = AG_INT(2);

	vPos.x = SK_VIEW_X(skv, AG_INT(1));
	vPos.y = SK_VIEW_Y(skv, WIDGET(skv)->h - AG_INT(2));
	vPos.z = 0.0;
	vRel.x = (SG_Real)AG_INT(3) * skv->wPixel;
	vRel.y = -(SG_Real)AG_INT(4) * skv->hPixel;
	vRel.z = 0.0;

	if (SG_MatrixInvert(&skv->mView, &Ti) == -1) {
		fprintf(stderr, "mView: %s\n", AG_GetError());
		return;
	}
	SG_MatrixMultVectorv(&vPos, &Ti);

	AG_MutexLock(&skv->sk->lock);

	if (skv->mouse.panning) {
		SG_MatrixTranslate2(&skv->mView, vRel.x, vRel.y);
		goto out;
	}
	if (tool != NULL && tool->ops->mousemotion != NULL) {
		if ((tool->ops->flags & SK_MOUSEMOTION_NOSNAP) == 0) {
			vPos.x = SK_VIEW_X_SNAP(skv,vPos.x);
			vPos.y = SK_VIEW_Y_SNAP(skv,vPos.y);
		}
		tool->ops->mousemotion(tool, vPos, vRel, AG_INT(5));
	}
out:
	skv->mouse.last.x = vPos.x;
	skv->mouse.last.y = vPos.y;
	AG_MutexUnlock(&skv->sk->lock);
}

static void
mousebuttondown(AG_Event *event)
{
	SK_View *skv = AG_SELF();
	SK_Tool *tool = SK_CURTOOL(skv);
	int button = AG_INT(1);
	SG_Vector vPos;
	SG_Matrix Ti;

	vPos.x = SK_VIEW_X(skv, AG_INT(2));
	vPos.y = SK_VIEW_Y(skv, WIDGET(skv)->h - AG_INT(3));
	vPos.z = 0.0;

	if (SG_MatrixInvert(&skv->mView, &Ti) == -1) {
		fprintf(stderr, "mView: %s\n", AG_GetError());
		return;
	}
	SG_MatrixMultVectorv(&vPos, &Ti);

	AG_WidgetFocus(skv);
	AG_MutexLock(&skv->sk->lock);
	skv->mouse.last = vPos;
	
	switch (button) {
	case SDL_BUTTON_RIGHT:
		SK_ViewPopupMenu(skv);
		break;
	case SDL_BUTTON_MIDDLE:
		skv->mouse.panning = 1;
		break;
	case SDL_BUTTON_WHEELDOWN:
		SK_ViewZoom(skv, SK_VIEW_SCALE_X(skv) -
		    logf(1.0+SK_VIEW_SCALE_X(skv))/3.0);
		goto out;
	case SDL_BUTTON_WHEELUP:
		SK_ViewZoom(skv, SK_VIEW_SCALE_X(skv) +
		    logf(1.0+SK_VIEW_SCALE_X(skv))/3.0);
		goto out;
	default:
		break;
	}
	if (tool != NULL && tool->ops->mousebuttondown != NULL) {
		if ((tool->ops->flags & SK_BUTTONDOWN_NOSNAP) == 0) {
			vPos.x = SK_VIEW_X_SNAP(skv, vPos.x);
			vPos.y = SK_VIEW_Y_SNAP(skv, vPos.y);
		}
		if (tool->ops->mousebuttondown(tool, vPos, button) == 1)
			goto out;
	}
	TAILQ_FOREACH(tool, &skv->tools, tools) {
		SK_ToolMouseBinding *mb;

		SLIST_FOREACH(mb, &tool->mbindings, mbindings) {
			if (mb->button != button) {
				continue;
			}
			tool->skv = skv;
			if (mb->func(tool, button, 1, vPos, mb->arg) == 1)
				goto out;
		}
	}
	if (skv->btndown_ev != NULL)
		AG_PostEvent(NULL, skv, skv->btndown_ev->name,
		    "%i,%f,%f", button, vPos.x, vPos.y);
out:
	AG_MutexUnlock(&skv->sk->lock);
}

static void
mousebuttonup(AG_Event *event)
{
	SK_View *skv = AG_SELF();
	SK_Tool *tool = SK_CURTOOL(skv);
	int button = AG_INT(1);
	SG_Vector vPos;
	SG_Matrix Ti;

	vPos.x = SK_VIEW_X(skv, AG_INT(2));
	vPos.y = SK_VIEW_Y(skv, WIDGET(skv)->h - AG_INT(3));
	vPos.z = 0.0;

	if (SG_MatrixInvert(&skv->mView, &Ti) == -1) {
		fprintf(stderr, "mView: %s\n", AG_GetError());
		return;
	}
	SG_MatrixMultVectorv(&vPos, &Ti);

	AG_MutexLock(&skv->sk->lock);

	if (tool != NULL && tool->ops->mousebuttonup != NULL) {
		if ((tool->ops->flags & SK_BUTTONUP_NOSNAP) == 0) {
			vPos.x = SK_VIEW_X_SNAP(skv, vPos.x);
			vPos.y = SK_VIEW_Y_SNAP(skv, vPos.y);
		}
		if (tool->ops->mousebuttonup(tool, vPos, button) == 1)
			goto out;
	}
	TAILQ_FOREACH(tool, &skv->tools, tools) {
		SK_ToolMouseBinding *mb;

		SLIST_FOREACH(mb, &tool->mbindings, mbindings) {
			if (mb->button != button) {
				continue;
			}
			tool->skv = skv;
			if (mb->func(tool, button, 0, vPos, mb->arg) == 1)
				goto out;
		}
	}
	switch (button) {
	case SDL_BUTTON_MIDDLE:
		skv->mouse.panning = 0;
		goto out;
	default:
		break;
	}
	if (skv->btnup_ev != NULL)
		AG_PostEvent(NULL, skv, skv->btnup_ev->name, "%i,%f,%f", button,
		    vPos.x, vPos.y);
out:
	AG_MutexUnlock(&skv->sk->lock);
}

void
SK_ViewInit(SK_View *skv, SK *sk, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if (flags & SK_VIEW_HFILL) wflags |= AG_WIDGET_HFILL;
	if (flags & SK_VIEW_VFILL) wflags |= AG_WIDGET_VFILL;

	AG_WidgetInit(skv, &skViewOps, wflags);
	
	if (!AG_Bool(agConfig, "view.opengl"))
		fatal("widget requires OpenGL mode");

	skv->sk = sk;
	skv->predraw_ev = NULL;
	skv->postdraw_ev = NULL;
	skv->scale_ev = NULL;
	skv->keydown_ev = NULL;
	skv->btndown_ev = NULL;
	skv->keyup_ev = NULL;
	skv->btnup_ev = NULL;
	skv->motion_ev = NULL;
	skv->status[0] = '\0';
	skv->mouse.last.x = 0;
	skv->mouse.last.y = 0;
	skv->mouse.panning = 0;
	skv->curtool = NULL;
	skv->deftool = NULL;
	skv->wPixel = 1.0;
	skv->hPixel = 1.0;
	skv->editPane = NULL;
	skv->popup = NULL;
	skv->rSnap = 1.0;
	SG_MatrixIdentityv(&skv->mView);
	SG_MatrixIdentityv(&skv->mProj);
	TAILQ_INIT(&skv->tools);

	AG_SetEvent(skv, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(skv, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(skv, "window-mousebuttonup", mousebuttonup, NULL);
	
	SK_ViewZoom(skv, 1.0/10.0);
}

static void
Destroy(void *p)
{
	SK_View *skv = p;
	SK_Tool *tool, *toolNext;

	for (tool = TAILQ_FIRST(&skv->tools);
	     tool != TAILQ_END(&skv->tools);
	     tool = toolNext) {
		toolNext = TAILQ_NEXT(tool, tools);
		SK_ToolDestroy(tool);
		Free(tool, M_SG);
	}
	AG_WidgetDestroy(skv);
}

void
SK_ViewPreDrawFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->predraw_ev = AG_SetEvent(skv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(skv->predraw_ev, fmt);
}

void
SK_ViewPostDrawFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->postdraw_ev = AG_SetEvent(skv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(skv->postdraw_ev, fmt);
}

void
SK_ViewScaleFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->scale_ev = AG_SetEvent(skv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(skv->scale_ev, fmt);
}

void
SK_ViewKeydownFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->keydown_ev = AG_SetEvent(skv, "window-keydown", fn, NULL);
	AG_EVENT_GET_ARGS(skv->keydown_ev, fmt);
}

void
SK_ViewKeyupFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->keyup_ev = AG_SetEvent(skv, "window-keyup", fn, NULL);
	AG_EVENT_GET_ARGS(skv->keyup_ev, fmt);
}

void
SK_ViewButtondownFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->btndown_ev = AG_SetEvent(skv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(skv->btndown_ev, fmt);
}

void
SK_ViewButtonupFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->btnup_ev = AG_SetEvent(skv, "window-mousebuttonup", fn, NULL);
	AG_EVENT_GET_ARGS(skv->btnup_ev, fmt);
}

void
SK_ViewMotionFn(SK_View *skv, AG_EventFn fn, const char *fmt, ...)
{
	skv->motion_ev = AG_SetEvent(skv, "window-mousemotion", fn, NULL);
	AG_EVENT_GET_ARGS(skv->motion_ev, fmt);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	r->w = 32;	/* XXX */
	r->h = 32;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	SK_View *skv = p;

	if (a->w < 1 || a->h < 1)
		return (-1);

	SG_MatrixIdentityv(&skv->mProj);
	SK_ViewZoom(skv, 0.0);
	return (0);
}

void
SK_ViewZoom(SK_View *skv, SG_Real zoom)
{
	SG_Vector v;

	if (zoom != 0.0) {
		SK_VIEW_SCALE_X(skv) = zoom >= 0.01 ? zoom : 0.01;
		SK_VIEW_SCALE_Y(skv) = zoom >= 0.01 ? zoom : 0.01;
	}
	skv->wPixel = 1.0/((SG_Real)WIDGET(skv)->w)*2.0/SK_VIEW_SCALE_X(skv);
	skv->hPixel = 1.0/((SG_Real)WIDGET(skv)->h)*2.0/SK_VIEW_SCALE_Y(skv);
	skv->rSnap = 16.0*skv->wPixel;
}

static void
Draw(void *p)
{
	SK_View *skv = p;
	SK *sk = skv->sk;
	SDL_Surface *status;

	glViewport(
	    WIDGET(skv)->cx, agView->h - WIDGET(skv)->cy2,
	    WIDGET(skv)->w, WIDGET(skv)->h);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	SG_LoadMatrixGL(&skv->mProj);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	SG_LoadMatrixGL(&skv->mView);

	glPushAttrib(GL_ENABLE_BIT|GL_POLYGON_BIT);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);
	glShadeModel(GL_FLAT);
	
	if (skv->predraw_ev != NULL)
		skv->predraw_ev->handler(skv->predraw_ev);

	SK_RenderNode(sk, (SK_Node *)sk->root, skv);
	SK_RenderAbsolute(sk, skv);
	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glViewport(0, 0, agView->w, agView->h);

	if (skv->postdraw_ev != NULL)
		skv->postdraw_ev->handler(skv->postdraw_ev);
#if 0
	AG_TextColor(TEXT_COLOR);
	status = AG_TextRender(skv->status);
	AG_WidgetBlit(skv, status, 0, WIDGET(skv)->h - status->h);
	SDL_FreeSurface(status);
#endif
}

void
SK_ViewSelectTool(SK_View *skv, SK_Tool *ntool, void *p)
{
	AG_Window *wParent;

	if (skv->curtool != NULL) {
		if (skv->curtool->trigger != NULL) {
			AG_WidgetSetBool(skv->curtool->trigger, "state", 0);
		}
		if (skv->curtool->win != NULL) {
			AG_WindowHide(skv->curtool->win);
		}

		if (skv->curtool->pane != NULL) {
			AG_Widget *wt;

			OBJECT_FOREACH_CHILD(wt, skv->curtool->pane,
			    ag_widget) {
				AG_ObjectDetach(wt);
				AG_ObjectDestroy(wt);
				Free(wt, M_OBJECT);
			}
			wParent = AG_WidgetParentWindow(skv->curtool->pane);
			if (wParent != NULL)
				AG_WindowUpdate(wParent);
		}
		skv->curtool->skv = NULL;
	}
	skv->curtool = ntool;

	if (ntool != NULL) {
		ntool->p = p;
		ntool->skv = skv;

		if (ntool->trigger != NULL) {
			AG_WidgetSetBool(ntool->trigger, "state", 1);
		}
		if (ntool->win != NULL) {
			AG_WindowShow(ntool->win);
		}
#if 0
		if (skv->curtool->pane != NULL &&
		    ntool->ops->edit != NULL) {
			if (ntool->pane != NULL) {
				ntool->ops->edit(ntool, ntool->pane);
			}
			wParent = AG_WidgetParentWindow(skv->curtool->pane);
			if (wParent != NULL)
				AG_WindowUpdate(wParent);
		}
#endif
		snprintf(skv->status, sizeof(skv->status), _("Tool: %s"),
		    ntool->ops->name);
	} else {
		skv->status[0] = '\0';
	}
}

SK_Tool *
SK_ViewFindTool(SK_View *skv, const char *name)
{
	SK_Tool *tool;

	TAILQ_FOREACH(tool, &skv->tools, tools) {
		if (strcmp(tool->ops->name, name) == 0)
			return (tool);
	}
	return (NULL);
}

SK_Tool *
SK_ViewFindToolByOps(SK_View *skv, const SK_ToolOps *ops)
{
	SK_Tool *tool;

	TAILQ_FOREACH(tool, &skv->tools, tools) {
		if (tool->ops == ops)
			return (tool);
	}
	return (NULL);
}

SK_Tool *
SK_ViewRegTool(SK_View *skv, const SK_ToolOps *ops, void *p)
{
	SK_Tool *t;

	t = Malloc(ops->len, M_SG);
	t->ops = ops;
	t->skv = skv;
	t->p = p;
	SK_ToolInit(t);
	TAILQ_INSERT_TAIL(&skv->tools, t, tools);
	return (t);
}

void
SK_ViewSetDefaultTool(SK_View *skv, SK_Tool *tool)
{
	skv->deftool = tool;
}

static void
SetLengthUnit(AG_Event *event)
{
	SK *sk = AG_PTR(1);
	char *uname = AG_STRING(2);
	const AG_Unit *unit;

	dprintf("%s: setting unit to %s\n", OBJECT(sk)->name, uname);
	if ((unit = AG_FindUnit(uname)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "Unknown unit: %s", uname);
		return;
	}
	SK_SetLengthUnit(sk, unit);
}

static void
AddConstraint(AG_Event *event)
{
	SK *sk = AG_PTR(1);
	enum sk_constraint_type type = (enum sk_constraint_type)AG_INT(2);
	SK_Node *node, *nodes[2];
	int count = 0;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (!SKNODE_SELECTED(node)) {
			continue;
		}
		if (++count > 2) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("Please select only 2 nodes"));
			return;
		}
		nodes[count-1] = node;
	}
	if (count < 2) {
		AG_TextMsg(AG_MSG_ERROR, _("Please select 2 nodes"));
		return;
	}
	if (SK_AddConstraint(&sk->ctGraph, nodes[0], nodes[1], type) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	dprintf("%s: added %s constraint between %s and %s\n",
	    OBJECT(sk)->name, skConstraintNames[type],
	    nodes[0]->name, nodes[1]->name);

	SK_Update(sk);
}

void
SK_ViewPopupMenu(SK_View *skv)
{
	SK *sk = skv->sk;
	AG_MenuItem *node;
	int i;

	if (skv->popup != NULL) {
		AG_PopupDestroy(skv, skv->popup);
	}
	skv->popup = AG_PopupNew(skv);
	node = AG_MenuNode(skv->popup->item, _("Add constraint"), -1);
	for (i = 0; i < SK_CONSTRAINT_LAST; i++) {
		AG_MenuAction(node, _(skConstraintNames[i]), -1,
		    AddConstraint, "%p,%i", sk, i);
	}
	AG_MenuSeparator(skv->popup->item);
	node = AG_MenuNode(skv->popup->item, _("Set Unit system"), -1);
	{
		AG_MenuAction(node, _("Inches"), -1,
		    SetLengthUnit, "%p,%s", sk, "in");
		AG_MenuAction(node, _("Meters"), -1,
		    SetLengthUnit, "%p,%s", sk, "m");
		AG_MenuAction(node, _("Centimeters"), -1,
		    SetLengthUnit, "%p,%s", sk, "cm");
		AG_MenuAction(node, _("Millimeters"), -1,
		    SetLengthUnit, "%p,%s", sk, "mm");
		AG_MenuAction(node, _("Microns"), -1,
		    SetLengthUnit, "%p,%s", sk, "um");
	}
	AG_PopupShow(skv->popup);
}

void
SK_ViewCloseEditPane(SK_View *skv)
{
	if (skv->editPane == NULL) {
		return;
	}
	AG_ObjectDetach(skv->editPane);
	AG_ObjectDestroy(skv->editPane);
	Free(skv->editPane,0);
	skv->editPane = NULL;
}

const AG_WidgetOps skViewOps = {
	{
		"AG_Widget:SK_View",
		sizeof(SK_View),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* HAVE_OPENGL */
