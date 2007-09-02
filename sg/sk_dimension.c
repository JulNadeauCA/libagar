/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Dimension annotation. Creates distance and angle constraints, or
 * display effective distances, angles and radii.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"
#include "sk_view.h"
#include "sg_gui.h"

#include <gui/hsvpal.h>
#include <gui/hbox.h>
#include <gui/vbox.h>
#include <gui/numerical.h>
#include <gui/button.h>

SK_Dimension *
SK_DimensionNew(void *parent)
{
	SK_Dimension *dim;

	dim = Malloc(sizeof(SK_Dimension), M_SG);
	SK_DimensionInit(dim, SK_GenNodeName(SKNODE(parent)->sk, "Dimension"));
	SK_NodeAttach(parent, dim);
	return (dim);
}

void
SK_DimensionInit(void *p, Uint32 name)
{
	SK_Dimension *dim = p;

	SK_AnnotInit(dim, name, &skDimensionOps);

	dim->unit = NULL;
	dim->n1 = NULL;
	dim->n2 = NULL;
	dim->cLbl = SG_ColorRGB(0.0, 0.0, 0.5);
	dim->cLblBorder = SG_ColorRGB(0.0, 0.0, 0.5);
	dim->cLineDim = SG_ColorRGB(0.0, 0.0, 0.5);
	dim->vLbl = SG_VECTOR(0.0, 1.0, 0.0);
	dim->lbl = -1;
	dim->text[0] = '\0';
	dim->xPad = 5;
	dim->yPad = 2;
}

int
SK_DimensionLoad(SK *sk, void *p, AG_Netbuf *buf)
{
	char unitKey[AG_UNIT_KEY_MAX];
	SK_Dimension *dim = p;

	if (SK_AnnotLoad(sk, dim, buf) == -1)
		return (-1);

	AG_CopyString(unitKey, buf, sizeof(unitKey));
	if (unitKey[0] == '\0') {
		dim->unit = NULL;
	} else {
		if ((dim->unit = AG_FindUnit(unitKey)) == NULL)
			return (-1);
	}
	dim->cLbl = SG_ReadColor(buf);
	dim->cLblBorder = SG_ReadColor(buf);
	dim->cLineDim = SG_ReadColor(buf);
	dim->xPad = (int)AG_ReadUint8(buf);
	dim->yPad = (int)AG_ReadUint8(buf);
	dim->vLbl = SG_ReadVector(buf);
	dim->n1 = SK_ReadRef(buf, sk, NULL);
	dim->n2 = SK_ReadRef(buf, sk, NULL);
	if (dim->n1 == NULL || dim->n2 == NULL) {
		AG_SetError("Missing dimension nodes (%s)", AG_GetError());
		return (-1);
	}
	dim->text[0] = '\0';
	return (0);
}

int
SK_DimensionSave(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Dimension *dim = p;
	
	SK_AnnotSave(sk, dim, buf);
	AG_WriteString(buf, dim->unit != NULL ? dim->unit->key : "");
	SG_WriteColor(buf, &dim->cLbl);
	SG_WriteColor(buf, &dim->cLblBorder);
	SG_WriteColor(buf, &dim->cLineDim);
	AG_WriteUint8(buf, (Uint8)dim->xPad);
	AG_WriteUint8(buf, (Uint8)dim->yPad);
	SG_WriteVector(buf, &dim->vLbl);
	SK_WriteRef(buf, dim->n1);
	SK_WriteRef(buf, dim->n2);
	return (0);
}

/*
 * Compute the minimal distance between L and v, and return the
 * closest point on L into pvC.
 */
static SG_Real
PointLineDistance(const SG_Vector *v, const SK_Line *L, SG_Vector *pvC)
{
	SG_Vector p1 = SK_NodeCoords(L->p1);
	SG_Vector p2 = SK_NodeCoords(L->p2);
	SG_Vector vC;
	SG_Real mag = SG_VectorDistance(p1,p2);
	SG_Real u;

	u = ( ((v->x - p1.x)*(p2.x - p1.x)) +
              ((v->y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);
	vC = SG_VectorAdd(p1, SG_VectorScale(SG_VectorSubp(&p2,&p1), u));
	if (pvC != NULL) {
		SG_VectorCopy(pvC, &vC);
	}
	return (SG_VectorDistancep(v, &vC));
}

/*
 * Map to the dimension's reference frame. Also return the distance between
 * the two dimensioned entities into d if not NULL.
 */
static SG_Matrix
TransformToAnnotFrame(SK_Dimension *dim, SG_Vector *vr1, SG_Vector *vr2)
{
	SG_Matrix T;
	SG_Vector v1, v2;
	SG_Vector vd;
	
	if (SK_NodeOfClass(dim->n1, "Line:*") &&
	    SK_NodeOfClass(dim->n2, "Point:*")) {
		v2 = SK_NodeCoords(dim->n2);
		PointLineDistance(&v2, SKLINE(dim->n1), &v1);
	} else
	if (SK_NodeOfClass(dim->n1, "Point:*") &&
	    SK_NodeOfClass(dim->n2, "Line:*")) {
		v1 = SK_NodeCoords(dim->n1);
		PointLineDistance(&v1, SKLINE(dim->n2), &v2);
	} else {
		v1 = SK_NodeCoords(dim->n1);
		v2 = SK_NodeCoords(dim->n2);
	}

	vd = SG_VectorSubp(&v1, &v2);
	SG_MatrixIdentityv(&T);
	SG_MatrixTranslatev(&T, SG_VectorLERPp(&v1, &v2, 0.5));
	SG_MatrixRotateZv(&T, Atan2(vd.y, vd.x));
	if (vr1 != NULL) { SG_VectorCopy(vr1, &v1); }
	if (vr2 != NULL) { SG_VectorCopy(vr2, &v2); }
	return (T);
}

static SG_Real
NodeNodeDistance(SK_Node *n1, SK_Node *n2)
{
	SG_Vector v1, v2;

	if (SK_NodeOfClass(n1, "Point:*") &&
	    SK_NodeOfClass(n2, "Point:*")) {
		v1 = SK_NodeCoords(n1);
		v2 = SK_NodeCoords(n2);
		return SG_VectorDistancep(&v1, &v2);
	} else if (SK_NodeOfClass(n1, "Point:*") &&
	           SK_NodeOfClass(n2, "Line:*")) {
		v1 = SK_NodeCoords(n1);
		return PointLineDistance(&v1, SKLINE(n2), NULL);
	} else if (SK_NodeOfClass(n1, "Line:*") &&
	           SK_NodeOfClass(n2, "Point:*")) {
		v2 = SK_NodeCoords(n2);
		return PointLineDistance(&v2, SKLINE(n1), NULL);
	}
	return (HUGE_VAL);
}

static void
GetLabelText(SK_Dimension *dim, char *text, size_t text_len)
{
	SK *sk = SKNODE(dim)->sk;

	snprintf(text, text_len, "%.02f", NodeNodeDistance(dim->n1, dim->n2));
	if (SK_ConstrainedNodes(&sk->ctGraph, dim->n1, dim->n2) == NULL)
		strlcat(text, _(" (REF)"), text_len);
}

void
SK_DimensionDraw(void *p, SK_View *skv)
{
	char text[SK_DIMENSION_TEXT_MAX];
	SK_Dimension *dim = p;
	SG_Color cLblBorder = SK_NodeColor(dim, &dim->cLblBorder);
	SG_Color cLineDim = SK_NodeColor(dim, &dim->cLineDim);
	SG_Vector vC, v1, v2;
	SG_Real d, wLbl, hLbl;
	SG_Matrix Ta;
	int wText, hText;
	SG_Real y, x1, x2, e1, e2;
	SG_Real xPad = ((SG_Real)dim->xPad)*skv->wPixel; 
	SG_Real yPad = ((SG_Real)dim->yPad)*skv->hPixel;

	if (dim->n1 == NULL || dim->n2 == NULL)
		return;

	glPushMatrix();

	Ta = TransformToAnnotFrame(dim, &v1, &v2);
	d = SG_VectorDistancep(&v1, &v2);

	SG_MatrixTransposev(&Ta);
	SG_MultMatrixGL(&Ta);

	GetLabelText(dim, text, sizeof(text));
	AG_TextSize(text, &wText, &hText);
	wLbl = (SG_Real)wText * skv->wPixel;
	hLbl = (SG_Real)hText * skv->hPixel;

	if (strcmp(text, dim->text) != 0) {
		strlcpy(dim->text, text, sizeof(dim->text));
		SK_DimensionRedraw(dim, skv);
	}

	SG_Begin(SG_LINES);
	SG_Color3v(&cLineDim);
	y = dim->vLbl.y + hLbl/2.0;
	x1 = dim->vLbl.x - xPad;
	x2 = dim->vLbl.x + wLbl + xPad;
	e1 = -d/2.0;
	e2 = +d/2.0;
	if ((x1 - e1) > 0.0) {
		SG_Vertex2(e1, y);
		SG_Vertex2(x1, y);
	}
	if ((e2 - x2) > 0.0) {
		SG_Vertex2(x2, y);
		SG_Vertex2(e2, y);
	}
	SG_Vertex2(e1, 0.0);
	SG_Vertex2(e1, y);
	SG_Vertex2(e2, 0.0);
	SG_Vertex2(e2, y);
	SG_End();

	glPopAttrib();

	SG_TranslateVecGL(dim->vLbl);
	AG_WidgetBlitSurfaceGL(skv, dim->lbl, (float)wLbl, (float)hLbl);
	SG_Begin(SG_LINE_LOOP);
	SG_Color3v(&cLblBorder);
	SG_Vertex2(-xPad,	-yPad);
	SG_Vertex2(wLbl+xPad,	-yPad);
	SG_Vertex2(wLbl+xPad,	hLbl+yPad);
	SG_Vertex2(-xPad,	hLbl+yPad);
	SG_End();
	glPopMatrix();
}

void
SK_DimensionRedraw(void *p, SK_View *skv)
{
	SK_Dimension *dim = p;
	SG_Color cLbl = SK_NodeColor(dim, &dim->cLbl);

	AG_TextColorRGB((Uint8)(cLbl.r*255.0),
	                (Uint8)(cLbl.g*255.0),
	                (Uint8)(cLbl.b*255.0));
	if (dim->lbl == -1) {
		dim->lbl = AG_WidgetMapSurface(skv,
		    AG_TextRender(dim->text));
	} else {
		AG_WidgetReplaceSurface(skv, dim->lbl,
		    AG_TextRender(dim->text));
	}
}

void
SK_DimensionEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Dimension *dim = p;
}

SG_Real
SK_DimensionProximity(void *p, const SG_Vector *v, SG_Vector *vC)
{
	SK_Dimension *dim = p;
	SG_Matrix Ta;
	SG_Vector p1, p2;
	SG_Real mag, u;

	if (dim->n1 == NULL || dim->n2 == NULL)
		return (HUGE_VAL);

	Ta = TransformToAnnotFrame(dim, &p1, &p2);

//	SG_VectorSubv(&p1, &dim->vLbl);
//	SG_VectorSubv(&p2, &dim->vLbl);

	mag = SG_VectorDistance(p2, p1);
	u = ( ((v->x - p1.x)*(p2.x - p1.x)) +
              ((v->y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);
	if (u < 0.0 || u > 1.0) {
		return (HUGE_VAL);
	}
	*vC = SG_VectorAdd(p1, SG_VectorScale(SG_VectorSubp(&p2,&p1), u));
	return (SG_VectorDistancep(v, vC));
}

int
SK_DimensionDelete(void *p)
{
	SK_Dimension *dim = p;

	return (SK_AnnotDelete(dim));
}

int
SK_DimensionMove(void *p, const SG_Vector *pos, const SG_Vector *vel)
{
	SK_Dimension *dim = p;
	SG_Vector v;
	SG_Matrix Ta, TaInv;

	Ta = TransformToAnnotFrame(dim, NULL, NULL);
	v = SG_MatrixMultVectorp(&Ta, &dim->vLbl);
	SG_VectorAddv(&v, vel);
	TaInv = SG_MatrixInvertCramerp(&Ta);
	dim->vLbl = SG_MatrixMultVectorp(&TaInv, &v);
	return (0);
}

void
SK_DimensionSetUnit(SK_Dimension *dim, const AG_Unit *unit)
{
	dim->unit = unit;
}

SK_NodeOps skDimensionOps = {
	"Annot:Dimension",
	sizeof(SK_Dimension),
	0,
	SK_DimensionInit,
	NULL,			/* destroy */
	SK_DimensionLoad,
	SK_DimensionSave,
	NULL,			/* draw_relative */
	SK_DimensionDraw,
	SK_DimensionRedraw,
	SK_DimensionEdit,
	SK_DimensionProximity,
	SK_DimensionDelete,
	SK_DimensionMove,
	NULL,			/* constrained */
};

#ifdef EDITION

struct sk_dimension_tool {
	SK_Tool tool;
	SK_Dimension *curDim;
};

static void
AddDimConstraint(AG_Event *event)
{
	AG_Window *pWin = AG_PTR(1);
	struct sk_dimension_tool *t = AG_PTR(2);
	SK_Dimension *dim = AG_PTR(3);
	AG_Numerical *num = AG_PTR(4);
	SK *sk = SKTOOL(t)->skv->sk;
	SK_Constraint *ct;
	
	ct = SK_AddConstraint(&sk->ctGraph, dim->n1, dim->n2, SK_DISTANCE);
	if (ct == NULL) {
		AG_TextMsgFromError();
		goto out;
	}
	ct->ct_distance = (SG_Real)AG_Base2Unit(AG_NumericalGetDouble(num),
	                                        AG_FindUnit("mm"));
	SK_Update(sk);
out:
	t->curDim = NULL;
	AG_ViewDetach(pWin);
}

static void
UndoDimConstraint(AG_Event *event)
{
	AG_Window *pWin = AG_PTR(1);
	struct sk_dimension_tool *t = AG_PTR(2);

	SK_DimensionDelete(t->curDim);
	t->curDim = NULL;
	AG_ViewDetach(pWin);
}

static int
AddDimConstraintDlg(struct sk_dimension_tool *t, SK *sk, SK_Dimension *dim)
{
	char name1[SK_NODE_NAME_MAX];
	char name2[SK_NODE_NAME_MAX];
	SG_Vector v1, v2;
	SG_Real d;
	AG_Window *win;
	AG_Numerical *num;
	AG_VBox *vb;
	AG_HBox *hb;
	
	if (SK_FindConstraint(&sk->ctGraph, SK_CONSTRAINT_ANY, dim->n1, dim->n2)
	    != NULL) {
		AG_TextMsg(AG_MSG_ERROR,
		    _("Existing constraint; new %s constraint would "
		      "overconstraint sketch."),
		      skConstraintNames[SK_DISTANCE]);
		return (-1);
	}
	
	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE|
	                   AG_WINDOW_NOCLOSE);
	AG_WindowSetCaption(win, _("Distance constraint"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		AG_LabelNewStatic(vb, 0, _("Enter distance between %s and %s:"),
		    SK_NodeNameCopy(dim->n1, name1, sizeof(name1)),
		    SK_NodeNameCopy(dim->n2, name2, sizeof(name2)));
	}
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		static double v;

		num = AG_NumericalNew(vb, AG_NUMERICAL_HFILL, "mm", "");
		AG_NumericalSetIncrement(num, 
		    AG_Unit2Base(1.0, AG_FindUnit("mm")));
		AG_WidgetFocus(num);
		v = AG_Unit2Base(NodeNodeDistance(dim->n1, dim->n2),
		                 AG_FindUnit("mm"));
		AG_WidgetBindDouble(num, "value", &v);

		AG_SetEvent(num, "numerical-return",
		    AddDimConstraint, "%p,%p,%p,%p", win, t, dim, num);
	}
	hb = AG_HBoxNew(win, AG_HBOX_HFILL|AG_HBOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(hb, 0, _("OK"),
		    AddDimConstraint, "%p,%p,%p,%p", win, t, dim, num);
		AG_ButtonNewFn(hb, 0, _("Cancel"),
		    UndoDimConstraint, "%p,%p", win, t);
	}

	AG_WindowShow(win);
	return (0);
}

static void
ToolInit(void *p)
{
	struct sk_dimension_tool *t = p;

	t->curDim = NULL;
}

static int
ToolMouseMotion(void *self, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_dimension_tool *t = self;
	SK_View *skv = SKTOOL(t)->skv;
	SK_Dimension *dim;
	SK *sk = skv->sk;
	SG_Vector vC;
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->flags &= ~(SK_NODE_MOUSEOVER);
		SK_NodeRedraw(node, skv);
	}
	if ((node = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) != NULL &&
	    SG_VectorDistancep(&pos, &vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		SK_NodeRedraw(node, skv);
	} else {
		if ((node = SK_ProximitySearch(sk, NULL, &pos, &vC, NULL))
		    != NULL) {
			node->flags |= SK_NODE_MOUSEOVER;
			SK_NodeRedraw(node, skv);
		}
	}
	if (t->curDim != NULL &&
	    t->curDim->n1 != NULL &&
	    t->curDim->n2 != NULL) {
		SG_Vector vDim;
		SG_Matrix Ta, TaInv;

		Ta = TransformToAnnotFrame(t->curDim, NULL, NULL);
		TaInv = SG_MatrixInvertCramerp(&Ta);
		vDim = SG_MatrixMultVectorp(&TaInv, &pos);
	   	t->curDim->vLbl = vDim;
	}
	return (0);
}

static int
ToolMouseButtonDown(void *self, SG_Vector pos, int btn)
{
	struct sk_dimension_tool *t = self;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Node *n, *nPrev = NULL;
	SG_Vector vC;
	
	if (btn == SDL_BUTTON_RIGHT) {
		if (t->curDim != NULL) {
			goto undo;
		}
		return (0);
	} else if (btn != SDL_BUTTON_LEFT) {
		return (0);
	}

	if ((n = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) == NULL ||
	    SG_VectorDistancep(&pos, &vC) >= skv->rSnap) {
		if ((n = SK_ProximitySearch(sk, "Line", &pos, &vC, NULL))
		    == NULL)
			n = SK_ProximitySearch(sk, NULL, &pos, &vC, NULL);
	}
	if (t->curDim == NULL) {
		if (n != NULL) {
			t->curDim = SK_DimensionNew(sk->root);
			t->curDim->n1 = n;
		} else {
			goto undo;
		}
	} else if (t->curDim->n2 == NULL) {
		if (n != NULL) {
			t->curDim->n2 = n;
		} else {
			goto undo;
		}
	} else {
		if (AddDimConstraintDlg(t, sk, t->curDim) == -1)
			goto undo;
	}
	return (0);
undo:
	SK_DimensionDelete(t->curDim);
	t->curDim = NULL;
	return (0);
}

SK_ToolOps skDimensionToolOps = {
	N_("Dimension"),
	N_("Specify distances, angles and radii"),
	VGLINES_ICON,
	sizeof(struct sk_dimension_tool),
	0,
	ToolInit,
	NULL,		/* destroy */
	NULL,		/* edit */
	ToolMouseMotion,
	ToolMouseButtonDown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* EDITION */
#endif /* HAVE_OPENGL */
