/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

#include <stdarg.h>
#include <string.h>

SK_View *
SK_ViewNew(void *parent, SK *sk, Uint flags)
{
	SK_View *skv;

	skv = Malloc(sizeof(SK_View));
	AG_ObjectInit(skv, &skViewClass);
	skv->flags |= flags;
	skv->sk = sk;
	
	if (flags & SK_VIEW_HFILL) WIDGET(skv)->flags |= AG_WIDGET_HFILL;
	if (flags & SK_VIEW_VFILL) WIDGET(skv)->flags |= AG_WIDGET_VFILL;

	AG_ObjectAttach(parent, skv);
	return (skv);
}

void
SK_ViewZoom(SK_View *skv, M_Real zoom)
{
	if (zoom != 0.0) {
		SK_VIEW_SCALE_X(skv) = zoom >= 0.01 ? zoom : 0.01;
		SK_VIEW_SCALE_Y(skv) = zoom >= 0.01 ? zoom : 0.01;
	}
	skv->wPixel = 1.0 / ((M_Real)WIDTH(skv))*2.0 / SK_VIEW_SCALE_X(skv);
	skv->hPixel = 1.0 / ((M_Real)HEIGHT(skv))*2.0 / SK_VIEW_SCALE_Y(skv);
	skv->rSnap = 16.0*skv->wPixel;
	Debug(skv, "Zoom %f (1px = %f x %f)\n", zoom, skv->wPixel, skv->hPixel);
	AG_Redraw(skv);
}

static void
Draw(void *_Nonnull obj)
{
	SK_View *skv = obj;
	SK *sk = skv->sk;
	M_Matrix44 T;
	SK_Node *node;

	GL_PushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT);

	GL_Disable(GL_DEPTH_TEST);
#if 0
	GL_Enable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT, GL_FILL);
	glShadeModel(GL_FLAT);
#endif
	GL_MatrixMode(GL_PROJECTION);
	GL_PushMatrix();
	GL_LoadIdentity();
	
	GL_MatrixMode(GL_MODELVIEW);
	GL_PushMatrix();
	T = M_MatTranspose44p(&skv->mView);	/* OpenGL is column-major */
	GL_LoadMatrixv(&T);
	
	/* Render the nodes */
	TAILQ_FOREACH(node, &sk->root->cnodes, sknodes) {
		if (node->ops->draw != NULL)
			node->ops->draw(node, skv);
	}

	GL_PopMatrix();	/* GL_MODELVIEW */

	GL_MatrixMode(GL_PROJECTION);
	GL_PopMatrix();

	GL_PopAttrib();
}

static void
ViewMotion(AG_Event *_Nonnull event)
{
	SK_View *skv = SK_VIEW_SELF();
	SK_Tool *tool = skv->curtool;
	SK *sk = skv->sk;
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	const int xRel = AG_INT(3);
	const int yRel = AG_INT(4);
	const int state = AG_INT(5);
	M_Vector3 vPos, vRel;
	M_Matrix44 Tinv;

	vPos.x = SK_VIEW_X(skv, x);
	vPos.y = SK_VIEW_Y(skv, HEIGHT(skv) - y);
	vPos.z = 0.0;
	vRel.x =  (M_Real)xRel * skv->wPixel;
	vRel.y = -(M_Real)yRel * skv->hPixel;
	vRel.z = 0.0;

	Tinv = M_MatInvert44(skv->mView);
	vPos = M_VecFromProj3(M_MatMultVector44(Tinv, M_VecToProj3(vPos,1)));

	AG_MutexLock(&sk->lock);

	if (skv->flags & SK_VIEW_PANNING) {
		SK_Translate2(sk->root, vRel.x, vRel.y);
		AG_Redraw(skv);
		goto out;
	}
	if (tool != NULL && tool->ops->mousemotion != NULL) {
		if ((tool->ops->flags & SK_MOUSEMOTION_NOSNAP) == 0) {
			vPos.x = SK_VIEW_X_SNAP(skv,vPos.x);
			vPos.y = SK_VIEW_Y_SNAP(skv,vPos.y);
		}
		tool->ops->mousemotion(tool, vPos, vRel, state);
		AG_Redraw(skv);
	}
out:
	skv->mouseLast.x = vPos.x;
	skv->mouseLast.y = vPos.y;
	AG_MutexUnlock(&sk->lock);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	SK_View *skv = SK_VIEW_SELF();
	SK_Tool *tool = SK_CURTOOL(skv);
	SK *sk = skv->sk;
	const int button = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);
	M_Vector3 vPos;
	M_Matrix44 Tinv;

	if (!AG_WidgetIsFocused(skv))
		AG_WidgetFocus(skv);

	vPos.x = SK_VIEW_X(skv, x);
	vPos.y = SK_VIEW_Y(skv, HEIGHT(skv) - y);
	vPos.z = 0.0;

	Tinv = M_MatInvert44(skv->mView);
	vPos = M_VecFromProj3(M_MatMultVector44(Tinv, M_VecToProj3(vPos,1)));

	AG_MutexLock(&sk->lock);
	skv->mouseLast = vPos;
	
	switch (button) {
	case AG_MOUSE_RIGHT:
		SK_ViewPopupMenu(skv, x, y);
		break;
	case AG_MOUSE_MIDDLE:
		skv->flags |= SK_VIEW_PANNING;
		break;
	case AG_MOUSE_WHEELDOWN:
		SK_ViewZoom(skv, SK_VIEW_SCALE_X(skv) -
		    M_Log(1.0+SK_VIEW_SCALE_X(skv))/3.0);
		goto out;
	case AG_MOUSE_WHEELUP:
		SK_ViewZoom(skv, SK_VIEW_SCALE_X(skv) +
		    M_Log(1.0+SK_VIEW_SCALE_X(skv))/3.0);
		goto out;
	default:
		break;
	}
	if (tool != NULL && tool->ops->mousebuttondown != NULL) {
		if ((tool->ops->flags & SK_BUTTONDOWN_NOSNAP) == 0) {
			vPos.x = SK_VIEW_X_SNAP(skv, vPos.x);
			vPos.y = SK_VIEW_Y_SNAP(skv, vPos.y);
		}
		if (tool->ops->mousebuttondown(tool, vPos, button) == 1) {
			AG_Redraw(skv);
			goto out;
		}
	}
	TAILQ_FOREACH(tool, &skv->tools, tools) {
		SK_ToolMouseBinding *mb;

		SLIST_FOREACH(mb, &tool->mbindings, mbindings) {
			if (mb->button != button) {
				continue;
			}
			tool->skv = skv;
			if (mb->func(tool, button, 1, vPos, mb->arg) == 1) {
				AG_Redraw(skv);
				goto out;
			}
		}
	}
	if (skv->btndown_ev != NULL) {
		AG_PostEventByPtr(skv, skv->btndown_ev, "%i,%f,%f",
		    button, vPos.x, vPos.y);
		AG_Redraw(skv);
	}
out:
	AG_MutexUnlock(&sk->lock);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	SK_View *skv = SK_VIEW_SELF();
	SK_Tool *tool = SK_CURTOOL(skv);
	SK *sk = skv->sk;
	M_Matrix44 Tinv;
	M_Vector3 vPos;
	const int button = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	vPos.x = SK_VIEW_X(skv, x);
	vPos.y = SK_VIEW_Y(skv, HEIGHT(skv) - y);
	vPos.z = 0.0;

	Tinv = M_MatInvert44(skv->mView);
	vPos = M_VecFromProj3(M_MatMultVector44(Tinv, M_VecToProj3(vPos,1)));

	AG_MutexLock(&sk->lock);

	if (tool != NULL && tool->ops->mousebuttonup != NULL) {
		if ((tool->ops->flags & SK_BUTTONUP_NOSNAP) == 0) {
			vPos.x = SK_VIEW_X_SNAP(skv, vPos.x);
			vPos.y = SK_VIEW_Y_SNAP(skv, vPos.y);
		}
		if (tool->ops->mousebuttonup(tool, vPos, button) == 1) {
			AG_Redraw(skv);
			goto out;
		}
	}
	TAILQ_FOREACH(tool, &skv->tools, tools) {
		SK_ToolMouseBinding *mb;

		SLIST_FOREACH(mb, &tool->mbindings, mbindings) {
			if (mb->button != button) {
				continue;
			}
			tool->skv = skv;
			if (mb->func(tool, button, 0, vPos, mb->arg) == 1) {
				AG_Redraw(skv);
				goto out;
			}
		}
	}
	switch (button) {
	case AG_MOUSE_MIDDLE:
		skv->flags &= ~(SK_VIEW_PANNING);
		goto out;
	default:
		break;
	}
	if (skv->btnup_ev != NULL) {
		AG_PostEventByPtr(skv, skv->btnup_ev, "%i,%f,%f",
		    button, vPos.x, vPos.y);
		AG_Redraw(skv);
	}
out:
	AG_MutexUnlock(&sk->lock);
}

static void
Shown(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_SELF();

	SK_ViewZoom(skv, 1.0/10.0);
}

static void
Init(void *_Nonnull obj)
{
	SK_View *skv = obj;

	WIDGET(skv)->flags |= (AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_OPENGL);

	AG_SetEvent(skv, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(skv, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(skv, "mouse-motion", ViewMotion, NULL);

	skv->flags = 0;
	skv->sk = NULL;
	skv->scale_ev = NULL;
	skv->keydown_ev = NULL;
	skv->btndown_ev = NULL;
	skv->keyup_ev = NULL;
	skv->btnup_ev = NULL;
	skv->motion_ev = NULL;
	skv->mouseLast.x = 0;
	skv->mouseLast.y = 0;
	skv->curtool = NULL;
	skv->deftool = NULL;
	skv->wPixel = 1.0;
	skv->hPixel = 1.0;
	skv->editPane = NULL;
	skv->viewPane = NULL;
	skv->editBox = NULL;
	skv->popup = NULL;
	skv->rSnap = 1.0;
	skv->mView = M_MatIdentity44();
	skv->mProj = M_MatIdentity44();
	skv->pmView = NULL;
	TAILQ_INIT(&skv->tools);
	skv->status[0] = '\0';

	AG_AddEvent(skv, "widget-shown", Shown, NULL);
}

static void
Destroy(void *_Nonnull obj)
{
	SK_View *skv = obj;
	SK_Tool *tool, *toolNext;

	if (skv->pmView != NULL)
		AG_PopupDestroy(skv->pmView);

	for (tool = TAILQ_FIRST(&skv->tools);
	     tool != TAILQ_END(&skv->tools);
	     tool = toolNext) {
		toolNext = TAILQ_NEXT(tool, tools);
		SK_ToolDestroy(tool);
		Free(tool);
	}
}

/* Select an SK tool */
void
SK_ViewSelectTool(SK_View *skv, SK_Tool *ntool, void *p)
{
	if (skv->curtool != NULL) {
		if (skv->curtool->trigger != NULL) {
			AG_SetBool(skv->curtool->trigger, "state", 0);
		}
		if (skv->curtool->win != NULL) {
			AG_WindowHide(skv->curtool->win);
		}
		SK_ViewClearEditPane(skv);
		skv->curtool->skv = NULL;
	}
	skv->curtool = ntool;

	if (ntool != NULL) {
		ntool->p = p;
		ntool->skv = skv;

		if (ntool->trigger != NULL) {
			AG_SetBool(ntool->trigger, "state", 1);
		}
		if (ntool->win != NULL) {
			AG_WindowShow(ntool->win);
		}
		if (skv->editPane != NULL) {
			SK_ViewClearEditPane(skv);
			if (ntool->ops->edit != NULL) {
				skv->editBox = (AG_Widget *)
				    AG_BoxNew(skv->editPane->div[1],
				    AG_BOX_VERT,AG_BOX_EXPAND);
				ntool->ops->edit(ntool, skv->editBox);
				SK_ViewResizePanes(skv);
			}
		}
		Debug(skv->sk, "Selected tool: %s\n", ntool->ops->name);
		AG_Snprintf(skv->status, sizeof(skv->status), _("Tool: %s"),
		    ntool->ops->name);
	} else {
		skv->status[0] = '\0';
	}
}

/* Lookup a SK tool by name. */
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

/* Lookup a SK tool by class. */
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

/* Register a new SK tool class. */
SK_Tool *
SK_ViewRegTool(SK_View *skv, const SK_ToolOps *ops, void *p)
{
	SK_Tool *t;

	t = Malloc(ops->len);
	t->ops = ops;
	t->skv = skv;
	t->p = p;
	SK_ToolInit(t);
	TAILQ_INSERT_TAIL(&skv->tools, t, tools);
	return (t);
}

/* Configure the default SK tool. */
void
SK_ViewSetDefaultTool(SK_View *skv, SK_Tool *tool)
{
	skv->deftool = tool;
}

static void
SetLengthUnit(AG_Event *_Nonnull event)
{
	SK *sk = SK_PTR(1);
	const char *uname = AG_STRING(2);
	const AG_Unit *unit;

	if ((unit = AG_FindUnit(uname)) == NULL) {
		AG_TextError(_("No such unit \"%s\""), uname);
		return;
	}
	SK_SetLengthUnit(sk, unit);
	AG_TextTmsg(AG_MSG_INFO, 1250, _("Set unit to %s"), uname);
}

static void
AddConstraint(AG_Event *_Nonnull event)
{
	SK *sk = SK_PTR(1);
	const enum sk_constraint_type type = (enum sk_constraint_type)AG_INT(2);
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
	Debug(sk, "Added %s constraint between %s and %s\n",
	    skConstraintNames[type],
	    nodes[0]->name, nodes[1]->name);

	SK_Update(sk);
}

/*
 * Compute all intersections between selected entities and create entities
 * for them.
 */
static void
ComputeIntersections(AG_Event *_Nonnull event)
{
	SK *sk = SK_PTR(1);
	SK_Node *n1, *n2;
	SK_Group *g;
	SK_NodePair *tested;
	Uint i, nTested = 0;

	g = SK_GroupNew(sk->root);
	tested = Malloc(sizeof(SK_NodePair));

	TAILQ_FOREACH(n1, &sk->nodes, nodes) {
		if (!SKNODE_SELECTED(n1)) {
			continue;
		}
		TAILQ_FOREACH(n2, &sk->nodes, nodes) {
			if (!SKNODE_SELECTED(n2) || n2 == n1) {
				continue;
			}
			for (i = 0; i < nTested; i++) {
				if (SK_CompareNodePair(&tested[i], n1, n2))
					break;
			}
			if (i < nTested) {
				continue;
			}
			Debug(sk, "Computing %s-%s intersections\n",
			    n1->name, n2->name);
			SK_ComputeIntersections(g, n1, n2);
			tested = Realloc(tested, (nTested+1)*
			                         sizeof(SK_NodePair));
			tested[nTested].n1 = n1;
			tested[nTested].n2 = n2;
			nTested++;
		}
	}
	Free(tested);
}

/* Expand the SK_View popup menu. */
void
SK_ViewPopupMenu(SK_View *skv, int x, int y)
{
	SK *sk = skv->sk;
	AG_PopupMenu *pm;
	AG_MenuItem *node;
	int i;

	if (skv->pmView != NULL) {
		AG_PopupShowAt(skv->pmView, x, y);
		return;
	}
	if ((pm = skv->pmView = AG_PopupNew(skv)) == NULL)
		return;

	node = AG_MenuNode(pm->root, _("Add constraint"), NULL);
	for (i = 0; i < SK_CONSTRAINT_LAST; i++) {
		AG_MenuAction(node, _(skConstraintNames[i]), NULL,
		    AddConstraint, "%p,%i", sk, i);
	}
	AG_MenuAction(pm->root, _("Compute intersections"), NULL,
	    ComputeIntersections, "%p", sk);

	AG_MenuSeparator(pm->root);

	node = AG_MenuNode(pm->root, _("Set Unit system"), NULL);
	{
		AG_MenuAction(node, _("Inches"),      NULL, SetLengthUnit, "%p,%s", sk, "in");
		AG_MenuAction(node, _("Meters"),      NULL, SetLengthUnit, "%p,%s", sk, "m");
		AG_MenuAction(node, _("Centimeters"), NULL, SetLengthUnit, "%p,%s", sk, "cm");
		AG_MenuAction(node, _("Millimeters"), NULL, SetLengthUnit, "%p,%s", sk, "mm");
		AG_MenuAction(node, _("Microns"),     NULL, SetLengthUnit, "%p,%s", sk, "um");
	}
	AG_PopupShowAt(pm, x, y);
}

/* Configure a widget edit box. */
void
SK_ViewSetEditPane(SK_View *skv, AG_Pane *pane)
{
	AG_ObjectLock(skv);
	skv->editPane = pane;
	AG_ObjectUnlock(skv);
}

/* Configure a SK view pane. */
void
SK_ViewSetViewPane(SK_View *skv, AG_Pane *pane)
{
	AG_ObjectLock(skv);
	skv->viewPane = pane;
	AG_ObjectUnlock(skv);
}

/* Clear the contents of the edit widget box. */
void
SK_ViewClearEditPane(SK_View *skv)
{
	if (skv->editBox == NULL) {
		return;
	}
	AG_ObjectDetach(skv->editBox);
	AG_ObjectDestroy(skv->editBox);
	skv->editBox = NULL;
}

/* Resize attached SK view panes. */
void
SK_ViewResizePanes(SK_View *skv)
{
	AG_Pane *ePane = skv->editPane;
	AG_Pane *vPane = skv->viewPane;
	AG_SizeReq r;

	if (ePane == NULL || vPane == NULL) {
		return;
	}
	AG_WidgetSizeReq(ePane->div[1], &r);
	AG_PaneSetDivisionMin(ePane, 1, -1, r.h + ePane->wDiv);
	AG_PaneSetDivisionMin(vPane, 0, r.w + ePane->wDiv, -1);
	AG_PaneMoveDivider(ePane, HEIGHT(ePane) - r.h);
	AG_WidgetUpdate(skv);
}

/*
 * Issue a proximity query for the given position with respect to all
 * Point entities in the sketch.
 */
SK_Point *
SK_ViewOverPoint(SK_View *skv, M_Vector3 *pos, M_Vector3 *vC, void *ignore)
{
	SK *sk = skv->sk;
	SK_Node *node;
	
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->flags &= ~(SK_NODE_MOUSEOVER);
	}
	if ((node = SK_ProximitySearch(sk, "Point", pos, vC, ignore)) != NULL &&
	    M_VecDistance3p(pos, vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		return ((SK_Point *)node);
	}
	return (NULL);
}

AG_WidgetClass skViewClass = {
	{
		"AG_Widget:SK_View",
		sizeof(SK_View),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	NULL,		/* sizeReq */
	NULL		/* sizeAlloc */
};
