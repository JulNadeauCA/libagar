/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

#include <string.h>

const char *skDimensionTypeNames[] = {
	"None",
	"Distance",
	"Angle (at endpoint)",
	"Angle (at intersection)",
	NULL
};

SK_Dimension *
SK_DimensionNew(void *parent)
{
	SK_Dimension *dim;

	dim = Malloc(sizeof(SK_Dimension));
	SK_DimensionInit(dim, SK_GenNodeName(SKNODE(parent)->sk, "Dimension"));
	SK_NodeAttach(parent, dim);
	return (dim);
}

void
SK_DimensionInit(void *p, Uint name)
{
	SK_Dimension *dim = p;

	SK_AnnotInit(dim, name, &skDimensionOps);

	dim->unit = NULL;
	dim->n1 = NULL;
	dim->n2 = NULL;
	dim->cLbl = M_ColorRGB(0.0, 0.0, 0.5);
	dim->cLblBorder = M_ColorRGB(0.0, 0.0, 0.5);
	dim->cLineDim = M_ColorRGB(0.0, 0.0, 0.5);
	dim->vLbl = M_VecGet3(0.0, 1.0, 0.0);
	dim->wLbl = 1.0;
	dim->hLbl = 1.0;

	dim->text[0] = '\0';
	dim->xPad = 5;
	dim->yPad = 2;
	dim->type = SK_DIMENSION_NONE;
}

int
SK_DimensionLoad(SK *sk, void *p, AG_DataSource *buf)
{
	char unitKey[128];
	SK_Dimension *dim = p;

	if (SK_AnnotLoad(sk, dim, buf) == -1)
		return (-1);

	dim->type = (enum sk_dimension_type)AG_ReadUint32(buf);
	dim->flags = (Uint)AG_ReadUint32(buf);
	AG_CopyString(unitKey, buf, sizeof(unitKey));
	if (unitKey[0] == '\0') {
		dim->unit = NULL;
	} else {
		if ((dim->unit = AG_FindUnit(unitKey)) == NULL)
			return (-1);
	}
	dim->cLbl = M_ReadColor(buf);
	dim->cLblBorder = M_ReadColor(buf);
	dim->cLineDim = M_ReadColor(buf);
	dim->xPad = (int)AG_ReadUint8(buf);
	dim->yPad = (int)AG_ReadUint8(buf);
	dim->vLbl = M_ReadVector3(buf);
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
SK_DimensionSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Dimension *dim = p;

	if (dim->n1 == NULL || dim->n2 == NULL) {
		AG_SetErrorS("Missing n1/n2");
		return (-1);
	}

	SK_AnnotSave(sk, dim, buf);
	AG_WriteUint32(buf, (Uint32)dim->type);
	AG_WriteUint32(buf, (Uint32)dim->flags);
	AG_WriteString(buf, dim->unit != NULL ? dim->unit->key : "");
	M_WriteColor(buf, &dim->cLbl);
	M_WriteColor(buf, &dim->cLblBorder);
	M_WriteColor(buf, &dim->cLineDim);
	AG_WriteUint8(buf, (Uint8)dim->xPad);
	AG_WriteUint8(buf, (Uint8)dim->yPad);
	M_WriteVector3(buf, &dim->vLbl);
	SK_WriteRef(buf, dim->n1);
	SK_WriteRef(buf, dim->n2);
	return (0);
}

/*
 * Compute the minimal distance between L and v, and return the
 * closest point on L into pvC.
 */
static M_Real
PointLineDistance(const M_Vector3 *_Nonnull v, const SK_Line *_Nonnull L,
    M_Vector3 *_Nullable pvC)
{
	M_Vector3 p1 = SK_Pos(L->p1);
	M_Vector3 p2 = SK_Pos(L->p2);
	M_Vector3 vC;
	M_Real mag = M_VecDistance3(p1,p2);
	M_Real u;

	u = ( ((v->x - p1.x)*(p2.x - p1.x)) +
              ((v->y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);
	vC = M_VecAdd3(p1, M_VecScale3(M_VecSub3p(&p2,&p1), u));
	if (pvC != NULL) {
		M_VecCopy3(pvC, &vC);
	}
	return M_VecDistance3p(v,&vC);
}

/*
 * For a given dimension, return a transformation matrix to take us from the
 * sketch's coordinate system to a coordinate system where the x axis is the
 * line between the two dimensioned entities and the origin is this line's
 * midpoint.
 *
 * The sketch coordinates of the two measurement points are returned into
 * vr1 and vr2. In the case of point-line distances, not only the vertices
 * but any point on the line could be returned.
 */
static M_Matrix44
MapToAnnotation(SK_Dimension *_Nonnull dim, M_Vector3 *_Nullable vr1,
    M_Vector3 *_Nullable vr2, M_Vector3 *_Nullable vShared)
{
	M_Matrix44 T;
	M_Vector3 v1, v2;
	M_Vector3 v;
	M_Real theta;
	
	M_MatIdentity44v(&T);

	switch (dim->type) {
	case SK_DIMENSION_DISTANCE:
		/*
		 * Find the positions v1,v2 of the two entities. For point-line
		 * distances, we use the closest point on the line.
		 *
		 * Line-line distances entered by the user are expanded to two
		 * point-point distances so that case is not considered.
		 */
		if (SK_NodeOfClass(dim->n1, "Line:*") &&
		    SK_NodeOfClass(dim->n2, "Point:*")) {
			v2 = SK_Pos(dim->n2);
			PointLineDistance(&v2, SKLINE(dim->n1), &v1);
		} else
		if (SK_NodeOfClass(dim->n1, "Point:*") &&
		    SK_NodeOfClass(dim->n2, "Line:*")) {
			v1 = SK_Pos(dim->n1);
			PointLineDistance(&v1, SKLINE(dim->n2), &v2);
		} else {
			v1 = SK_Pos(dim->n1);
			v2 = SK_Pos(dim->n2);
		}
		v = M_VecSub3p(&v1, &v2);
		theta = Atan2(v.y, v.x);
		M_MatTranslate44v(&T, M_VecLERP3p(&v1, &v2, 0.5));
		M_MatRotate44K(&T, theta);
		if (vr1 != NULL) { M_VecCopy3(vr1, &v1); }
		if (vr2 != NULL) { M_VecCopy3(vr2, &v2); }
		break;
	case SK_DIMENSION_ANGLE_ENDPOINT:
		if (SK_LineSharedEndpoint(SKLINE(dim->n1), SKLINE(dim->n2),
		    &v, &v1, &v2) == -1) {
			AG_FatalError("Bogus ANGLE_ENDPOINT");
		}
		M_MatTranslate44v(&T, v);
		M_MatRotate44K(&T, M_PI);
		if (vr1 != NULL) { M_VecCopy3(vr1, &v1); }
		if (vr2 != NULL) { M_VecCopy3(vr2, &v2); }
		if (vShared != NULL) { M_VecCopy3(vShared, &v); }
		break;
	case SK_DIMENSION_ANGLE_INTERSECT:
		break;
	default:
		break;
	}
	return (T);
}

static M_Real
GetDimensionVal(SK_Dimension *_Nonnull dim)
{
	SK_Node *n1 = dim->n1;
	SK_Node *n2 = dim->n2;
	M_Vector3 v1, v2;
	M_Real rv;

	if (SK_NodeOfClass(n1, "Point:*") &&
	    SK_NodeOfClass(n2, "Point:*")) {
		v1 = SK_Pos(n1);
		v2 = SK_Pos(n2);
		return M_VecDistance3p(&v1,&v2);
	} else if (SK_NodeOfClass(n1, "Point:*") &&
	           SK_NodeOfClass(n2, "Line:*")) {
		v1 = SK_Pos(n1);
		return PointLineDistance(&v1, SKLINE(n2), NULL);
	} else if (SK_NodeOfClass(n1, "Line:*") &&
	           SK_NodeOfClass(n2, "Point:*")) {
		v2 = SK_Pos(n2);
		return PointLineDistance(&v2, SKLINE(n1), NULL);
	} else if (SK_NodeOfClass(n1, "Line:*") &&
	           SK_NodeOfClass(n2, "Line:*")) {
		rv = SK_LineLineAngleCCW(SKLINE(n1), SKLINE(n2));
		if (dim->flags & SK_DIMENSION_CONJ_ANGLE) {
			return (M_PI*2.0 - rv);
		} else {
			return (rv);
		}
	}
	return (M_INFINITY);
}

static void
GetLabelText(SK_Dimension *_Nonnull dim, char *_Nonnull text, AG_Size text_len)
{
	SK *sk = SKNODE(dim)->sk;

	switch (dim->type) {
	case SK_DIMENSION_DISTANCE:
		AG_Snprintf(text, text_len, "%.02f",
		    AG_Base2Unit(GetDimensionVal(dim), sk->uLen));
		break;
	case SK_DIMENSION_ANGLE_ENDPOINT:
	case SK_DIMENSION_ANGLE_INTERSECT:
		AG_Snprintf(text, text_len, "%.02f\xc2\xb0",
		    Degrees(GetDimensionVal(dim)));
		break;
	default:
		text[0] = '\0';
		break;
	}
	if (SK_ConstrainedNodes(&sk->ctGraph, dim->n1, dim->n2) == NULL)
		Strlcat(text, _(" (REF)"), text_len);
}

static __inline__ void
DrawLinearDistance(SK_Dimension *_Nonnull dim, M_Real xPad,
    M_Vector3 *_Nonnull v1, M_Vector3 *_Nonnull v2)
{
	M_Color cLineDim = SK_NodeColor(dim, &dim->cLineDim);
	M_Real y, x1, x2, e1, e2;
	M_Real d;

	y = dim->vLbl.y + dim->hLbl/2.0;
	x1 = dim->vLbl.x - xPad;
	x2 = dim->vLbl.x + dim->wLbl + xPad;
	d = M_VecDistance3p(v1,v2);
	e1 = -d/2.0;
	e2 = +d/2.0;
		
	GL_Begin(GL_LINES);
	GL_Color3v(&cLineDim);
	if ((x1 - e1) > 0.0) {
		GL_Vertex2(e1, y);
		GL_Vertex2(x1, y);
	}
	if ((e2 - x2) > 0.0) {
		GL_Vertex2(x2, y);
		GL_Vertex2(e2, y);
	}
	GL_Vertex2(e1, 0.0);
	GL_Vertex2(e1, y);
	GL_Vertex2(e2, 0.0);
	GL_Vertex2(e2, y);
	GL_End();
}

static __inline__ void
DrawAngle(SK_Dimension *_Nonnull dim, M_Vector3 *_Nonnull vShared,
    M_Vector3 *_Nonnull v1, M_Vector3 *_Nonnull v2, SK_View *_Nonnull skv)
{
	M_Color cLineDim = SK_NodeColor(dim, &dim->cLineDim);
	M_Real r = 10.0*skv->wPixel;
	M_Real incr = (2.0*M_PI)/30.0;
	M_Real a1, a2, i;
	
	if (dim->flags & SK_DIMENSION_CONJ_ANGLE) {
		a1 = Atan2(v1->y - vShared->y, v1->x - vShared->x) - M_PI;
		a2 = Atan2(v2->y - vShared->y, v2->x - vShared->x) - M_PI;
	} else {
		a1 = Atan2(v2->y - vShared->y, v2->x - vShared->x) - M_PI;
		a2 = Atan2(v1->y - vShared->y, v1->x - vShared->x) - M_PI;
	}
	while (a1 > a2)
		a2 += M_PI*2.0;

	GL_Begin(GL_LINE_STRIP);
	GL_Color3v(&cLineDim);
	GL_Vertex2(Cos(a1)*r, Sin(a1)*r);
	for (i = a1+incr; i < a2-incr; i += incr) {
		GL_Vertex2(Cos(i)*r, Sin(i)*r);
	}
	GL_Vertex2(Cos(a2)*r, Sin(a2)*r);
	GL_End();
}

static SK_DimensionView *_Nullable
GetNodeData(SK_View *_Nonnull skv, SK_Dimension *_Nonnull dim)
{
	SK_DimensionView *dimv;

	if ((dimv = SKNODE(dim)->userData) == NULL) {
		dimv = Malloc(sizeof(SK_DimensionView));
		dimv->lbl = -1;
		SKNODE(dim)->userData = dimv;
	}
	return (dimv);
}

void
SK_DimensionDraw(void *p, SK_View *skv)
{
	char text[SK_DIMENSION_TEXT_MAX];
	SK_Dimension *dim = p;
	M_Color cLblBorder = SK_NodeColor(dim, &dim->cLblBorder);
	M_Vector3 v1, v2, vShared;
	M_Matrix44 Ta;
	int wText, hText;
	M_Real xPad = ((M_Real)dim->xPad)*skv->wPixel; 
	M_Real yPad = ((M_Real)dim->yPad)*skv->hPixel;
	SK_DimensionView *dimv;

	if (dim->n1 == NULL || dim->n2 == NULL)
		return;

	dimv = GetNodeData(skv, dim);
	GetLabelText(dim, text, sizeof(text));
	AG_TextSize(text, &wText, &hText);
	dim->wLbl = (M_Real)wText * skv->wPixel;
	dim->hLbl = (M_Real)hText * skv->hPixel;
	if (strcmp(text, dim->text) != 0) {
		Strlcpy(dim->text, text, sizeof(dim->text));
		if (dimv->lbl != -1) {
			AG_WidgetUnmapSurface(skv, dimv->lbl);
			dimv->lbl = -1;
		}
	}
	if (dimv->lbl == -1)
		SK_DimensionRedraw(dim, skv);

	GL_PushMatrix();
	Ta = MapToAnnotation(dim, &v1, &v2, &vShared);
	M_MatTranspose44v(&Ta);
	GL_MultMatrixv(&Ta);

	switch (dim->type) {
	case SK_DIMENSION_DISTANCE:
		DrawLinearDistance(dim, xPad, &v1, &v2);
		break;
	case SK_DIMENSION_ANGLE_ENDPOINT:
		DrawAngle(dim, &vShared, &v1, &v2, skv);
		break;
	default:
		break;
	}

	AG_PushBlendingMode(skv, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	GL_Translate(dim->vLbl);
	AG_WidgetBlitSurfaceFlippedGL(skv, dimv->lbl,
	    (float)dim->wLbl, (float)dim->hLbl);
	GL_Begin(GL_LINE_LOOP);
	{
		GL_Color3v(&cLblBorder);
		GL_Vertex2(-xPad,		-yPad);
		GL_Vertex2(dim->wLbl+xPad,	-yPad);
		GL_Vertex2(dim->wLbl+xPad,	dim->hLbl+yPad);
		GL_Vertex2(-xPad,		dim->hLbl+yPad);
	}
	GL_End();
	GL_PopMatrix();

	AG_PopBlendingMode(skv);
}

void
SK_DimensionRedraw(void *p, SK_View *skv)
{
	SK_Dimension *dim = p;
	SK_DimensionView *dimv = GetNodeData(skv, dim);
	M_Color cLbl = SK_NodeColor(dim, &dim->cLbl);
	
	AG_PushTextState();
	AG_TextColorRGB((Uint8)(cLbl.r*255.0),
	                (Uint8)(cLbl.g*255.0),
	                (Uint8)(cLbl.b*255.0));
	if (dimv->lbl == -1) {
		dimv->lbl = AG_WidgetMapSurface(skv,
		    AG_TextRender(dim->text));
	} else {
		AG_WidgetReplaceSurface(skv, dimv->lbl,
		    AG_TextRender(dim->text));
	}
	AG_PopTextState();
}

void
SK_DimensionEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Dimension *dim = p;
	AG_MSpinbutton *msb;
	AG_MFSpinbutton *mfsb;

	AG_LabelNew(box, 0, "Type: %s", skDimensionTypeNames[dim->type]);
	AG_LabelNew(box, 0, "Units: %s", dim->unit != NULL ? dim->unit->abbr :
	                                 "(default)");
	msb = AG_MSpinbuttonNew(box, 0, ",", "Padding: ");
	AG_BindInt(msb, "xvalue", &dim->xPad);
	AG_BindInt(msb, "yvalue", &dim->yPad);
	mfsb = AG_MFSpinbuttonNew(box, 0, ",", "Position: ");
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	AG_BindFloat(mfsb, "xvalue", &dim->vLbl.x);
	AG_BindFloat(mfsb, "yvalue", &dim->vLbl.y);
#else
	M_BindReal(mfsb, "xvalue", &dim->vLbl.x);
	M_BindReal(mfsb, "yvalue", &dim->vLbl.y);
#endif
}

M_Real
SK_DimensionProximity(void *p, const M_Vector3 *v, M_Vector3 *vC)
{
	SK_Dimension *dim = p;
	M_Vector3 p1, p2;
	M_Real r, u;
	M_Matrix44 M;

	if (dim->n1 == NULL || dim->n2 == NULL)
		return (M_INFINITY);

	M = MapToAnnotation(dim, NULL, NULL, NULL);
	p1 = dim->vLbl;
	p1.y += dim->hLbl/2.0;
	p1 = M_VecFromProj3(M_MatMultVector44(M, M_VecToProj3(p1,1)));

	p2 = dim->vLbl;
	p2.x += dim->wLbl;
	p2.y += dim->hLbl/2.0;
	p2 = M_VecFromProj3(M_MatMultVector44(M, M_VecToProj3(p2,1)));

	r = M_VecDistance3(p2,p1);
	u = ( ((v->x - p1.x)*(p2.x - p1.x)) +
	      ((v->y - p1.y)*(p2.y - p1.y)) ) / (r*r);
	      
	if (u < 0.0) {
		*vC = p1;
	} else if (u > 1.0) {
		*vC = p2;
	} else {
		*vC = M_VecAdd3(p1, M_VecScale3(M_VecSub3(p2,p1), u));
	}
	return M_VecDistance3p(v,vC);
}

int
SK_DimensionDelete(void *p)
{
	SK_Dimension *dim = p;

	return SK_AnnotDelete(dim);
}

int
SK_DimensionMove(void *p, const M_Vector3 *pos, const M_Vector3 *vel)
{
	SK_Dimension *dim = p;
	M_Vector3 v;
	M_Matrix44 Ta, TaInv;

	Ta = MapToAnnotation(dim, NULL, NULL, NULL);
	v = M_VecFromProj3(M_MatMultVector44(Ta, M_VecToProj3(dim->vLbl,1)));
	M_VecAdd3v(&v, vel);
	TaInv = M_MatInvert44(Ta);
	dim->vLbl = M_VecFromProj3(M_MatMultVector44(TaInv, M_VecToProj3(v,1)));
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
	SK_DimensionDraw,
	SK_DimensionRedraw,
	SK_DimensionEdit,
	SK_DimensionProximity,
	SK_DimensionDelete,
	SK_DimensionMove,
	NULL,			/* constrained */
};

struct sk_dimension_tool {
	SK_Tool tool;
	SK_Dimension *curDim;
};

static void
AddDimConstraint(AG_Event *_Nonnull event)
{
	AG_Window *pWin = AG_PTR(1);
	struct sk_dimension_tool *t = AG_PTR(2);
	SK_Dimension *dim = AG_PTR(3);
	AG_Numerical *num = AG_PTR(4);
	SK *sk = SKTOOL(t)->skv->sk;

	switch (dim->type) {
	case SK_DIMENSION_DISTANCE:
		SK_AddConstraint(&sk->ctGraph, dim->n1, dim->n2,
		    SK_DISTANCE, M_NumericalGetReal(num));
		SK_Update(sk);
		break;
	case SK_DIMENSION_ANGLE_ENDPOINT:
	case SK_DIMENSION_ANGLE_INTERSECT:
		SK_AddConstraint(&sk->ctGraph, dim->n1, dim->n2,
		    SK_ANGLE, M_NumericalGetReal(num));
		SK_Update(sk);
		break;
	default:
		break;
	}
	t->curDim = NULL;
	AG_ObjectDetach(pWin);
}

static void
UndoDimConstraint(AG_Event *_Nonnull event)
{
	AG_Window *pWin = AG_PTR(1);
	struct sk_dimension_tool *t = AG_PTR(2);

	SK_DimensionDelete(t->curDim);
	t->curDim = NULL;
	AG_ObjectDetach(pWin);
}

static int
AddDimConstraintDlg(struct sk_dimension_tool *_Nonnull t, SK *_Nonnull sk,
    SK_Dimension *_Nonnull dim)
{
	AG_Window *win;
	AG_Numerical *num;
	AG_Box *hb, *vb;
	const char *unit = NULL;

#if 0
	if (SK_FindConstraint(&sk->ctGraph, SK_CONSTRAINT_ANY, dim->n1, dim->n2)
	    != NULL) {
		AG_TextMsg(AG_MSG_ERROR,
		    _("Existing constraint; new %s constraint would "
		      "overconstraint sketch."),
		      skConstraintNames[SK_DISTANCE]);
		return (-1);
	}
#endif
	win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_NOVRESIZE |
	                   AG_WINDOW_NOCLOSE);

	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, AG_BOX_HFILL);
	{
		switch (dim->type) {
		case SK_DIMENSION_DISTANCE:
			AG_WindowSetCaptionS(win, _("Distance constraint"));
			AG_LabelNew(vb, 0,
			    _("Distance between %s and %s:"),
			    dim->n1->name, dim->n2->name);
			unit = "mm";
			break;
		case SK_DIMENSION_ANGLE_ENDPOINT:
		case SK_DIMENSION_ANGLE_INTERSECT:
			AG_WindowSetCaptionS(win, _("Angle constraint"));
			AG_LabelNew(vb, 0,
			    _("Angle between %s and %s:"),
			    dim->n1->name, dim->n2->name);
			unit = "deg";
			break;
		default:
			AG_SetError("Bad dimension type");
			return (-1);
		}
	}

	vb = AG_BoxNewVert(win, AG_BOX_HFILL);
	{
		static double v;

		/* XXX */
		num = AG_NumericalNewS(vb, AG_NUMERICAL_HFILL, unit, NULL);
		AG_WidgetFocus(num);
		v = GetDimensionVal(dim);
		AG_BindDouble(num, "value", &v);
		AG_SetDouble(num, "inc", AG_Unit2Base(1.0, AG_FindUnit(unit)));

		AG_SetEvent(num, "numerical-return",
		    AddDimConstraint, "%p,%p,%p,%p", win, t, dim, num);
	}

	hb = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
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
ToolInit(void *_Nonnull p)
{
	struct sk_dimension_tool *t = p;

	t->curDim = NULL;
}

static int
ToolMouseMotion(void *_Nonnull self, M_Vector3 pos, M_Vector3 vel, int btn)
{
	struct sk_dimension_tool *t = self;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	M_Vector3 vC;
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->flags &= ~(SK_NODE_MOUSEOVER);
		SK_NodeRedraw(node, skv);
	}
	if ((node = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) != NULL &&
	    M_VecDistance3p(&pos,&vC) < skv->rSnap) {
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
		M_Matrix44 Ta, TaInv;

		Ta = MapToAnnotation(t->curDim, NULL, NULL, NULL);
		TaInv = M_MatInvert44(Ta);
		t->curDim->vLbl = M_VecFromProj3(M_MatMultVector44(TaInv,
		                                 M_VecToProj3(pos,1)));
	}
	return (0);
}

static int
ToolMouseButtonDown(void *_Nonnull self, M_Vector3 pos, int btn)
{
	struct sk_dimension_tool *t = self;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Node *n;
	M_Vector3 vC;
	
	if (btn == AG_MOUSE_RIGHT) {
		if (t->curDim != NULL) {
			goto undo;
		}
		return (0);
	} else if (btn != AG_MOUSE_LEFT) {
		return (0);
	}

	if ((n = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) == NULL ||
	    M_VecDistance3p(&pos,&vC) >= skv->rSnap) {
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
		
			if (SK_NodeOfClass(t->curDim->n1, "Line:*") &&
			    SK_NodeOfClass(t->curDim->n2, "Line:*")) {
				SK_Line *L1 = SKLINE(t->curDim->n1);
				SK_Line *L2 = SKLINE(t->curDim->n2);

				if (L1->p1 == L2->p1 || L1->p1 == L2->p2 ||
				    L1->p2 == L2->p1 || L1->p2 == L2->p2) {
					t->curDim->type =
					    SK_DIMENSION_ANGLE_ENDPOINT;
					if (SK_LineLineAngleCCW(L1, L2) > M_PI)
					{
						t->curDim->flags |=
						    SK_DIMENSION_CONJ_ANGLE;
					}
				} else {
					t->curDim->type =
					    SK_DIMENSION_ANGLE_INTERSECT;
				}
			} else {
				M_Vector3 v1, v2;
				SK_Node *nTmp;

				t->curDim->type = SK_DIMENSION_DISTANCE;

				/*
				 * The order of the points determine the
				 * handedness of the annotation's coordinate
				 * system, so we arrange for the text to
				 * display left to right initially.
				 */
				v1 = SK_Pos(t->curDim->n1);
				v2 = SK_Pos(t->curDim->n2);
				if (v1.x > v2.x) {
					nTmp = t->curDim->n1;
					t->curDim->n1 = t->curDim->n2;
					t->curDim->n2 = nTmp;
				}
			}
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
	NULL,
	sizeof(struct sk_dimension_tool),
	0,
	ToolInit,
	NULL,			/* destroy */
	NULL,			/* edit */
	ToolMouseMotion,
	ToolMouseButtonDown,
	NULL,			/* buttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
