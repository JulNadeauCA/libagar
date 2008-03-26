/*
 * Copyright (c) 2004-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Base vector graphics object.
 */

#include <core/core.h>

#include <gui/view.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"
#include "vg_math.h"
#include "icons.h"
#include "icons_data.h"

#include <string.h>

const AG_Version vgVer = { 6, 0 };

extern const VG_ElementOps vgPointsOps;
extern const VG_ElementOps vgLinesOps;
extern const VG_ElementOps vgLineStripOps;
extern const VG_ElementOps vgLineLoopOps;
extern const VG_ElementOps vgCircleOps;
extern const VG_ElementOps vgArcOps;
extern const VG_ElementOps vgEllipseOps;
extern const VG_ElementOps vgTextOps;
extern const VG_ElementOps vgMaskOps;
extern const VG_ElementOps vgPolygonOps;

const VG_ElementOps *vgElementTypes[] = {
	&vgPointsOps,
	&vgLinesOps,
	&vgLineStripOps,
	&vgLineLoopOps,
	NULL,			/* triangles */
	NULL,			/* triangle strip */
	NULL,			/* triangle fan */
	NULL,			/* quads */
	NULL,			/* quad strip */
	&vgPolygonOps,
	&vgCircleOps,
	&vgArcOps,
	&vgEllipseOps,
	NULL,			/* Bezier curve */
	NULL,			/* Bezigon */
	&vgTextOps,
	&vgMaskOps,
};

static int vgInited = 0;

void
VG_InitSubsystem(void)
{
	AG_RegisterClass(&vgViewClass);

	vgIcon_Init();
}

void
VG_DestroySubsystem(void)
{
}

VG *
VG_New(Uint flags)
{
	VG *vg;

	vg = Malloc(sizeof(VG));
	VG_Init(vg, flags);
	return (vg);
}

void
VG_Init(VG *vg, Uint flags)
{
	int i;

	if (!vgInited) {
		VG_InitSubsystem();
		vgInited = 1;
	}

	Strlcpy(vg->name, _("Untitled"), sizeof(vg->name));
	vg->flags = flags;
	vg->fillColor = VG_GetColorRGB(0,0,0);
	vg->gridColor = VG_GetColorRGB(200,200,0);
	vg->selectionColor = VG_GetColorRGB(255,255,0);
	vg->mouseoverColor = VG_GetColorRGB(200,200,0);
	vg->origin = Malloc(sizeof(VG_Vtx)*VG_NORIGINS);
	vg->originRadius = Malloc(sizeof(float)*VG_NORIGINS);
	vg->originColor = Malloc(sizeof(Uint32)*VG_NORIGINS);
	vg->layers = Malloc(sizeof(VG_Layer));
	vg->nlayers = 0;
	vg->cur_layer = 0;
	vg->cur_block = NULL;
	vg->cur_vge = NULL;
	VG_PushLayer(vg, _("Layer 0"));
	TAILQ_INIT(&vg->vges);
	TAILQ_INIT(&vg->blocks);
	TAILQ_INIT(&vg->styles);
	AG_MutexInitRecursive(&vg->lock);

	for (i = 0; i < VG_NORIGINS; i++) {
		vg->origin[i].x = 0;
		vg->origin[i].y = 0;
		vg->originRadius[i] = 0.0625f;
		vg->originColor[i] = VG_GetColorRGB(0,0,180);
	}
	vg->originRadius[0] = 0.25f;
	vg->originRadius[1] = 0.125f;
	vg->originRadius[2] = 0.075f;
	vg->originColor[0] = VG_GetColorRGB(0,200,0);
	vg->originColor[1] = VG_GetColorRGB(0,150,0);
	vg->originColor[2] = VG_GetColorRGB(0,80,150);
	vg->norigin = VG_NORIGINS;

	vg->ints = NULL;
	vg->nints = 0;
}

void
VG_FreeElement(VG *vg, VG_Element *vge)
{
	if (vge->ops->destroy != NULL) {
		vge->ops->destroy(vg, vge);
	}
	Free(vge->vtx);
	Free(vge->trans);
	Free(vge);
}

static void
VG_DestroyElements(VG *vg)
{
	VG_Element *vge, *nvge;
	
	for (vge = TAILQ_FIRST(&vg->vges);
	     vge != TAILQ_END(&vg->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vges);
		VG_FreeElement(vg, vge);
	}
	TAILQ_INIT(&vg->vges);
}

static void
VG_DestroyBlocks(VG *vg)
{
	VG_Block *vgb, *nvgb;

	for (vgb = TAILQ_FIRST(&vg->blocks);
	     vgb != TAILQ_END(&vg->blocks);
	     vgb = nvgb) {
		nvgb = TAILQ_NEXT(vgb, vgbs);
		Free(vgb);
	}
	TAILQ_INIT(&vg->blocks);
}

static void
VG_DestroyStyles(VG *vg)
{
	VG_Style *st, *nst;

	for (st = TAILQ_FIRST(&vg->styles);
	     st != TAILQ_END(&vg->styles);
	     st = nst) {
		nst = TAILQ_NEXT(st, styles);
		Free(st);
	}
	TAILQ_INIT(&vg->styles);
}

void
VG_Reinit(VG *vg)
{
	VG_DestroyBlocks(vg);
	VG_DestroyElements(vg);
	VG_DestroyStyles(vg);
}

void
VG_Destroy(VG *vg)
{
	Free(vg->origin);
	Free(vg->originRadius);
	Free(vg->originColor);
	Free(vg->layers);

	VG_DestroyBlocks(vg);
	VG_DestroyElements(vg);
	VG_DestroyStyles(vg);
	AG_MutexDestroy(&vg->lock);

	Free(vg->ints);
}

void
VG_DestroyElement(VG *vg, VG_Element *vge)
{
	if (vge->block != NULL)
		TAILQ_REMOVE(&vge->block->vges, vge, vgbmbs);

	if (vg->cur_vge == vge)
		vg->cur_vge = NULL;

	TAILQ_REMOVE(&vg->vges, vge, vges);
	VG_FreeElement(vg, vge);
}

void VG_SetBackgroundColor(VG *vg, VG_Color c) { vg->fillColor = c; }
void VG_SetGridColor(VG *vg, VG_Color c) { vg->gridColor = c; }
void VG_SetSelectionColor(VG *vg, VG_Color c) { vg->selectionColor = c; }
void VG_SetMouseOverColor(VG *vg, VG_Color c) { vg->mouseoverColor = c; }

/*
 * Allocate the given type of element and begin its parametrization.
 * If a block is selected, associate the element with it.
 */
VG_Element *
VG_Begin(VG *vg, enum vg_element_type eltype)
{
	VG_Element *vge;

	vge = Malloc(sizeof(VG_Element));
	vge->flags = 0;
	vge->type = eltype;
	vge->style = NULL;
	vge->layer = vg->cur_layer;
	vge->selected = 0;
	vge->mouseover = 0;
	vge->vtx = NULL;
	vge->nvtx = 0;
	vge->trans = NULL;
	vge->ntrans = 0;
	vge->color = VG_GetColorRGB(250,250,250);

	vge->line_st.style = VG_CONTINUOUS;
	vge->line_st.endpoint_style = VG_SQUARE;
	vge->line_st.stipple = 0x0;
	vge->line_st.thickness = 1;
	vge->line_st.miter_len = 0;

	Strlcpy(vge->text_st.face, OBJECT(agDefaultFont)->name,
	    sizeof(vge->text_st.face));
	vge->text_st.size = agDefaultFont->size;
	vge->text_st.flags = agDefaultFont->flags;
	vge->fill_st.style = VG_SOLID;
	vge->fill_st.texture[0] = '\0';
	vge->fill_st.texture_alpha = 255;

	AG_MutexLock(&vg->lock);
	TAILQ_INSERT_TAIL(&vg->vges, vge, vges);

	if (vg->cur_block != NULL) {
		TAILQ_INSERT_TAIL(&vg->cur_block->vges, vge, vgbmbs);
		vge->block = vg->cur_block;
		if (vge->block->flags & VG_BLOCK_NOSAVE)
			vge->flags |= VG_ELEMENT_NOSAVE;
	} else {
		vge->block = NULL;
	}

	vge->ops = vgElementTypes[eltype];
	if (vge->ops->init != NULL)
		vge->ops->init(vg, vge);

	vg->cur_vge = vge;
	AG_MutexUnlock(&vg->lock);
	return (vge);
}

/*
 * End the parametrization of the current element.
 * The vg must be locked.
 */
void
VG_End(VG *vg)
{
	vg->cur_vge = NULL;
}

/*
 * Select the given element for edition.
 * The vg must be locked.
 */
void
VG_Select(VG *vg, VG_Element *vge)
{
	vg->cur_vge = vge;
}

#ifdef DEBUG
void
VG_DrawExtents(VG_View *vv)
{
	VG *vg = vv->vg;
	VG_Rect bbox;
	VG_Element *vge;
	VG_Block *vgb;
	int x, y, w, h;
	Uint32 cGreen;
	Uint32 cGrid;

	cGreen = SDL_MapRGB(agVideoFmt, 0,250,0);
	cGrid = VG_MapColorRGB(vg->gridColor);

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->ops->bbox != NULL) {
			vge->ops->bbox(vg, vge, &bbox);
		} else {
			continue;
		}
		VG_GetViewCoords(vv, bbox.x, bbox.y, &x, &y);
		w = (int)(bbox.w*vv->scale);
		h = (int)(bbox.h*vv->scale);
		AG_DrawRectOutline(vv, AG_RECT(x-1, y-1, w+2, h+2), cGrid);
	}
	
	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		VG_BlockExtent(vg, vgb, &bbox);
		VG_GetViewCoords(vv, bbox.x, bbox.y, &x, &y);
		w = (int)(bbox.w*vv->scale);
		h = (int)(bbox.h*vv->scale);
		AG_DrawRectOutline(vv, AG_RECT(x-1, y-1, w+2, h+2), cGreen);
	}
}
#endif /* DEBUG */

VG_Vtx *
VG_AllocVertex(VG_Element *vge)
{
	if (vge->vtx == NULL) {
		vge->vtx = Malloc(sizeof(VG_Vtx));
	} else {
		vge->vtx = Realloc(vge->vtx, (vge->nvtx+1)*sizeof(VG_Vtx));
	}
	return (&vge->vtx[vge->nvtx++]);
}

VG_Matrix *
VG_AllocMatrix(VG_Element *vge)
{
	if (vge->trans == NULL) {
		vge->trans = Malloc(sizeof(VG_Matrix));
	} else {
		vge->trans = Realloc(vge->trans,
		    (vge->ntrans+1)*sizeof(VG_Matrix));
	}
	return (&vge->trans[vge->ntrans++]);
}

VG_Vtx *
VG_PopVertex(VG *vg)
{
	VG_Element *vge = vg->cur_vge;

	if (vge->vtx == NULL)
		return (NULL);
#ifdef DEBUG
	if (vge->nvtx-1 < 0)
		AG_FatalError("VG_PopVertex: Negative vertex count");
#endif
	vge->vtx = Realloc(vge->vtx, (--vge->nvtx)*sizeof(VG_Vtx));
	return ((vge->nvtx > 0) ? &vge->vtx[vge->nvtx-1] : NULL);
}

VG_Matrix *
VG_PopMatrix(VG *vg)
{
	VG_Element *vge = vg->cur_vge;

	if (vge->trans == NULL)
		return (NULL);
#ifdef DEBUG
	if (vge->ntrans-1 < 0)
		AG_FatalError("VG_PopMatrix: Negative matrix count");
#endif
	vge->trans = Realloc(vge->trans, (--vge->ntrans)*sizeof(VG_Matrix));
	return ((vge->ntrans > 0) ? &vge->trans[vge->ntrans-1] : NULL);
}

/* Push a 2D vertex onto the vertex array. */
VG_Vtx *
VG_Vertex2(VG *vg, float x, float y)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	VG_BlockOffset(vg, vtx);
	return (vtx);
}

/*
 * Push a vertex at the intersection point between a given line (x1,y1,x2,y2)
 * and a vertical line at x.
 */
VG_Vtx *
VG_VertexVint2(VG *vg, float x, float px1, float py1, float px2, float py2)
{
	float x1 = px1, x2 = px2;
	float y1 = py1, y2 = py2;
	float m, x3 = x1;
	VG_Vtx *vtx;

	if (y1 < y2) { m = y1; y1 = y2; y2 = m; x3 = x2; }
	if (x1 < x2) { m = x1; x1 = x2; x2 = m; }
	m = Fabs(y2-y1)/Fabs(x2-x1);

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = m*(x - x3);
	VG_BlockOffset(vg, vtx);
	return (vtx);
}

/*
 * Push a vertical line onto the vertex array with endpoints at (x,y) and
 * the intersection point with a given line (x1,y1,x2,y2).
 */
void
VG_VintVLine2(VG *vg, float x, float y, float x1, float y1, float x2, float y2)
{
	VG_VertexVint2(vg, x, x1, y1, x2, y2);
	VG_Vertex2(vg, x, y);
}

/* Push a series of vertices onto the vertex array. */
void
VG_VertexV(VG *vg, const VG_Vtx *svtx, Uint nsvtx)
{
	VG_Element *vge = vg->cur_vge;
	Uint i;
	
	for (i = 0; i < nsvtx; i++) {
		VG_Vtx *vtx;

		vtx = VG_AllocVertex(vge);
		memcpy(vtx, &svtx[i], sizeof(VG_Vtx));
		VG_BlockOffset(vg, vtx);
	}
}

/* Push two vertices onto the vertex array. */
void
VG_Line(VG *vg, float x1, float y1, float x2, float y2)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x1;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x2;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
}

/* Push the two endpoints of a vertical line onto the vertex array. */
void
VG_VLine(VG *vg, float x, float y1, float y2)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
}

/* Push the two endpoints of a horizontal line onto the vertex array. */
void
VG_HLine(VG *vg, float x1, float x2, float y)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x1;
	vtx->y = y;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x2;
	vtx->y = y;
	VG_BlockOffset(vg, vtx);
}

void
VG_Rectangle(VG *vg, float x1, float y1, float x2, float y2)
{
	VG_Vtx *vtx;

	VG_Begin(vg, VG_LINE_LOOP);

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x1;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x2;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x2;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x1;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
	
	VG_End(vg);
}

void
VG_MultMatrixByVector(VG_Vtx *c, const VG_Vtx *a, const VG_Matrix *T)
{
	float ax = a->x;
	float ay = a->y;

	c->x = ax*T->m[0][0] + ay*T->m[1][0] + T->m[0][2];
	c->y = ax*T->m[0][1] + ay*T->m[1][1] + T->m[1][2];
}

void
VG_CopyMatrix(VG_Matrix *B, const VG_Matrix *A)
{
	int m, n;

	for (m = 0; m < 3; m++)
		for (n = 0; n < 3; n++)
			B->m[m][n] = A->m[m][n];
}

void
VG_MultMatrixByMatrix(VG_Matrix *C, const VG_Matrix *B, const VG_Matrix *A)
{
	VG_Matrix R;
	int m, n;

	for (m = 0; m < 3; m++) {
		for (n = 0; n < 3; n++)
			R.m[m][n] = B->m[m][0]*A->m[0][n] +
			            B->m[m][1]*A->m[1][n] +
			            B->m[m][2]*A->m[2][n];
	}
	VG_CopyMatrix(C, &R);
}

void
VG_LoadIdentity(VG_Matrix *m)
{
	m->m[0][0] = 1.0; m->m[0][1] = 0.0; m->m[0][2] = 0.0;
	m->m[1][0] = 0.0; m->m[1][1] = 1.0; m->m[1][2] = 0.0;
	m->m[2][0] = 0.0; m->m[2][1] = 0.0; m->m[2][2] = 1.0;
}

VG_Matrix *
VG_PushIdentity(VG *vg)
{
	VG_Matrix *m;

	m = VG_AllocMatrix(vg->cur_vge);
	VG_LoadIdentity(m);
	return (m);
}

void
VG_LoadTranslate(VG_Matrix *m, float x, float y)
{
	m->m[0][0] = 1.0; m->m[0][1] = 0.0; m->m[0][2] = x;
	m->m[1][0] = 0.0; m->m[1][1] = 1.0; m->m[1][2] = y;
	m->m[2][0] = 0.0; m->m[2][1] = 0.0; m->m[2][2] = 1.0;
}

VG_Matrix *
VG_Translate(VG *vg, float x, float y)
{
	VG_Matrix *m;

	m = VG_AllocMatrix(vg->cur_vge);
	VG_LoadTranslate(m, x, y);
	return (m);
}

void
VG_LoadRotate(VG_Matrix *m, float tdeg)
{
	float theta = Radians(tdeg);
	float rcos = Cos(theta);
	float rsin = Sin(theta);

	m->m[0][0] = +rcos;
	m->m[0][1] = -rsin;
	m->m[0][2] = 0.0;
	m->m[1][0] = +rsin;
	m->m[1][1] = +rcos;
	m->m[1][2] = 0.0;
	m->m[2][0] = 0.0;
	m->m[2][1] = 0.0;
	m->m[2][2] = 1.0;
}

VG_Matrix *
VG_Rotate(VG *vg, float tdeg)
{
	VG_Matrix *m;

	m = VG_AllocMatrix(vg->cur_vge);
	VG_LoadRotate(m, tdeg);
	return (m);
}

/* Create a new global style. */
VG_Style *
VG_CreateStyle(VG *vg, enum vg_style_type type, const char *name)
{
	VG_Style *vgs;

	vgs = Malloc(sizeof(VG_Style));
	Strlcpy(vgs->name, name, sizeof(vgs->name));
	vgs->type = type;
	vgs->color = VG_GetColorRGB(250,250,250);
	switch (vgs->type) {
	case VG_LINE_STYLE:
		vgs->vg_line_st.style = VG_CONTINUOUS;
		vgs->vg_line_st.endpoint_style = VG_SQUARE;
		vgs->vg_line_st.stipple = 0x0;
		vgs->vg_line_st.thickness = 1;
		vgs->vg_line_st.miter_len = 0;
		break;
	case VG_FILL_STYLE:
		vgs->vg_fill_st.style = VG_SOLID;
		vgs->vg_fill_st.texture[0] = '\0';
		vgs->vg_fill_st.texture_alpha = 255;
		break;
	case VG_TEXT_STYLE:
		Strlcpy(vgs->vg_text_st.face, OBJECT(agDefaultFont)->name,
		    sizeof(vgs->vg_text_st.face));
		vgs->vg_text_st.size = agDefaultFont->size;
		vgs->vg_text_st.flags = agDefaultFont->flags;
		break;
	}
	TAILQ_INSERT_TAIL(&vg->styles, vgs, styles);
	return (vgs);
}

/* Associate the given style with the current element. */
int
VG_SetStyle(VG *vg, const char *name)
{
	VG_Element *vge = vg->cur_vge;
	VG_Style *st;

	TAILQ_FOREACH(st, &vg->styles, styles) {
		if (strcmp(st->name, name) == 0)
			break;
	}
	if (st == NULL) {
		return (-1);
	}
	switch (st->type) {
	case VG_LINE_STYLE:
		memcpy(&vge->line_st, &st->vg_line_st,
		    sizeof(VG_LineStyle));
		break;
	case VG_TEXT_STYLE:
		memcpy(&vge->text_st, &st->vg_text_st,
		    sizeof(VG_TextStyle));
		break;
	case VG_FILL_STYLE:
		memcpy(&vge->fill_st, &st->vg_fill_st,
		    sizeof(VG_FillingStyle));
		break;
	}
	return (0);
}

/* Specify the layer# to associate with the current element. */
void
VG_SetLayer(VG *vg, int layer)
{
	vg->cur_vge->layer = layer;
}

/* Specify the color of the current element (VG color format). */
void
VG_Colorv(VG *vg, const VG_Color *c)
{
	vg->cur_vge->color.r = c->r;
	vg->cur_vge->color.g = c->g;
	vg->cur_vge->color.b = c->b;
	vg->cur_vge->color.a = c->a;
}

/* Specify the color of the current element (RGB triplet). */
void
VG_ColorRGB(VG *vg, Uint8 r, Uint8 g, Uint8 b)
{
	vg->cur_vge->color.r = r;
	vg->cur_vge->color.g = g;
	vg->cur_vge->color.b = b;
}

/* Specify the color of the current element (RGB triplet + alpha). */
void
VG_ColorRGBA(VG *vg, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	vg->cur_vge->color.r = r;
	vg->cur_vge->color.g = g;
	vg->cur_vge->color.b = b;
	vg->cur_vge->color.a = a;
}

/* Push a new layer onto the layer stack. */
VG_Layer *
VG_PushLayer(VG *vg, const char *name)
{
	VG_Layer *vgl;

	vg->layers = Realloc(vg->layers, (vg->nlayers+1) *
	                                 sizeof(VG_Layer));
	vgl = &vg->layers[vg->nlayers];
	vg->nlayers++;
	Strlcpy(vgl->name, name, sizeof(vgl->name));
	vgl->visible = 1;
	vgl->alpha = 255;
	vgl->color = VG_GetColorRGB(255,255,255);
	return (vgl);
}

/* Pop the highest layer off the layer stack. */
void
VG_PopLayer(VG *vg)
{
	if (--vg->nlayers < 1)
		vg->nlayers = 1;
}

static void
VG_SaveMatrix(VG_Matrix *A, AG_DataSource *buf)
{
	int m, n;

	for (m = 0; m < 3; m++) {
		for (n = 0; n < 3; n++)
			AG_WriteFloat(buf, A->m[m][n]);
	}
}

static void
VG_LoadMatrix(VG_Matrix *A, AG_DataSource *buf)
{
	int m, n;

	for (m = 0; m < 3; m++) {
		for (n = 0; n < 3; n++)
			A->m[m][n] = AG_ReadFloat(buf);
	}
}

void
VG_Save(VG *vg, AG_DataSource *buf)
{
	off_t nblocks_offs, nvges_offs, nstyles_offs;
	Uint32 nblocks = 0, nvges = 0, nstyles = 0;
	VG_Block *vgb;
	VG_Style *vgs;
	VG_Element *vge;
	int i;

	AG_WriteVersion(buf, "AG_VG", &vgVer);

	AG_MutexLock(&vg->lock);

	AG_WriteString(buf, vg->name);
	AG_WriteUint32(buf, (Uint32)vg->flags);
	VG_WriteColor(buf, &vg->fillColor);
	VG_WriteColor(buf, &vg->gridColor);
	VG_WriteColor(buf, &vg->selectionColor);
	VG_WriteColor(buf, &vg->mouseoverColor);
	AG_WriteFloat(buf, 0.0f);				/* gridIval */
	AG_WriteUint32(buf, (Uint32)vg->cur_layer);

	/* Save the origin points. */
	AG_WriteUint32(buf, vg->norigin);
	for (i = 0; i < vg->norigin; i++) {
		VG_WriteVertex(buf, &vg->origin[i]);
		AG_WriteFloat(buf, vg->originRadius[i]);
		VG_WriteColor(buf, &vg->originColor[i]);
	}

	/* Save the layer information. */
	AG_WriteUint32(buf, vg->nlayers);
	for (i = 0; i < vg->nlayers; i++) {
		VG_Layer *layer = &vg->layers[i];

		AG_WriteString(buf, layer->name);
		AG_WriteUint8(buf, (Uint8)layer->visible);
		VG_WriteColor(buf, &layer->color);
		AG_WriteUint8(buf, layer->alpha);
	}

	/* Save the block information. */
	nblocks_offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		if (vgb->flags & VG_BLOCK_NOSAVE)
			continue;

		AG_WriteString(buf, vgb->name);
		AG_WriteUint32(buf, (Uint32)vgb->flags);
		VG_WriteVertex(buf, &vgb->pos);
		VG_WriteVertex(buf, &vgb->origin);
		AG_WriteFloat(buf, vgb->theta);
		nblocks++;
	}
	AG_WriteUint32At(buf, nblocks, nblocks_offs);

	/* Save the global style information. */
	nstyles_offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vgs, &vg->styles, styles) {
		AG_WriteString(buf, vgs->name);
		AG_WriteUint8(buf, (Uint8)vgs->type);
		VG_WriteColor(buf, &vgs->color);

		switch (vgs->type) {
		case VG_LINE_STYLE:
			AG_WriteUint8(buf, (Uint8)vgs->vg_line_st.style);
			AG_WriteUint8(buf,
			    (Uint8)vgs->vg_line_st.endpoint_style);
			AG_WriteUint16(buf, vgs->vg_line_st.stipple);
			AG_WriteUint8(buf, vgs->vg_line_st.thickness);
			AG_WriteUint8(buf, vgs->vg_line_st.miter_len);
			break;
		case VG_FILL_STYLE:
			AG_WriteUint8(buf, (Uint8)vgs->vg_fill_st.style);
			AG_WriteString(buf, vgs->vg_fill_st.texture);
			AG_WriteUint8(buf, vgs->vg_fill_st.texture_alpha);
			break;
		case VG_TEXT_STYLE:
			AG_WriteString(buf, vgs->vg_text_st.face);
			AG_WriteUint8(buf, (Uint8)vgs->vg_text_st.size);
			AG_WriteUint32(buf, (Uint32)vgs->vg_text_st.flags);
			break;
		}
		nstyles++;
	}
	AG_WriteUint32At(buf, nstyles, nstyles_offs);

	/* Save the vg elements. */
	nvges_offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->flags & VG_ELEMENT_NOSAVE)
			continue;

		AG_WriteUint32(buf, (Uint32)vge->type);
		AG_WriteString(buf, vge->block != NULL ?
		    vge->block->name : NULL);
		AG_WriteUint32(buf, (Uint32)vge->layer);
		VG_WriteColor(buf, &vge->color);

		/* Save the line style information. */
		AG_WriteUint8(buf, (Uint8)vge->line_st.style);
		AG_WriteUint8(buf, (Uint8)vge->line_st.endpoint_style);
		AG_WriteUint16(buf, vge->line_st.stipple);
		AG_WriteUint8(buf, vge->line_st.thickness);
		AG_WriteUint8(buf, vge->line_st.miter_len);

		/* Save the filling style information. */
		AG_WriteUint8(buf, (Uint8)vge->fill_st.style);
		AG_WriteString(buf, vge->fill_st.texture);
		AG_WriteUint8(buf, vge->fill_st.texture_alpha);
		
		/* Save the text style information. */
		AG_WriteString(buf, vge->text_st.face);
		AG_WriteUint8(buf, (Uint8)vge->text_st.size);
		AG_WriteUint32(buf, (Uint32)vge->text_st.flags);

		/* Save the vertices. */
		AG_WriteUint32(buf, (Uint32)vge->nvtx);
		for (i = 0; i < vge->nvtx; i++)
			VG_WriteVertex(buf, &vge->vtx[i]);
		
		/* Save the transformation matrices. */
		AG_WriteUint32(buf, (Uint32)vge->ntrans);
		for (i = 0; i < vge->ntrans; i++)
			VG_SaveMatrix(&vge->trans[i], buf);

		/* Save element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			AG_WriteFloat(buf, vge->vg_circle.radius);
			break;
		case VG_ARC:
		case VG_ELLIPSE:
			AG_WriteFloat(buf, vge->vg_arc.w);
			AG_WriteFloat(buf, vge->vg_arc.h);
			AG_WriteFloat(buf, vge->vg_arc.s);
			AG_WriteFloat(buf, vge->vg_arc.e);
			break;
		case VG_TEXT:
			if (vge->vg_text.su != NULL) {
				SDL_FreeSurface(vge->vg_text.su);
				vge->vg_text.su = NULL;
			}
			AG_WriteString(buf, vge->vg_text.text);
			AG_WriteFloat(buf, vge->vg_text.angle);
			AG_WriteUint8(buf, (Uint8)vge->vg_text.align);
			break;
		case VG_MASK:
			AG_WriteFloat(buf, vge->vg_mask.scale);
			AG_WriteUint8(buf, (Uint8)vge->vg_mask.visible);
			AG_WriteString(buf, NULL);		       /* Pad */
			break;
		default:
			break;
		}
		nvges++;
	}
	AG_WriteUint32At(buf, nvges, nvges_offs);
	AG_MutexUnlock(&vg->lock);
}

int
VG_Load(VG *vg, AG_DataSource *buf)
{
	Uint32 norigin, nlayers, nstyles, nelements, nblocks;
	Uint32 i;

	if (AG_ReadVersion(buf, "AG_VG", &vgVer, NULL) != 0)
		return (-1);

	AG_MutexLock(&vg->lock);
	AG_CopyString(vg->name, buf, sizeof(vg->name));
	vg->flags = AG_ReadUint32(buf);
	vg->fillColor = VG_ReadColor(buf);
	vg->gridColor = VG_ReadColor(buf);
	vg->selectionColor = VG_ReadColor(buf);
	vg->mouseoverColor = VG_ReadColor(buf);
	(void)AG_ReadFloat(buf);				/* gridIval */
	vg->cur_layer = (int)AG_ReadUint32(buf);
	vg->cur_block = NULL;
	vg->cur_vge = NULL;

	/* Read the origin points. */
	if ((norigin = AG_ReadUint32(buf)) < 1) {
		AG_SetError("norigin < 1");
		goto fail;
	}
	vg->origin = Realloc(vg->origin, norigin*sizeof(VG_Vtx));
	vg->originRadius = Realloc(vg->originRadius, norigin*sizeof(float));
	vg->originColor = Realloc(vg->originColor, norigin*sizeof(Uint32));
	vg->norigin = norigin;
	for (i = 0; i < vg->norigin; i++) {
		vg->origin[i] = VG_ReadVertex(buf);
		vg->originRadius[i] = AG_ReadFloat(buf);
		vg->originColor[i] = VG_ReadColor(buf);
	}

	/* Read the layer information. */
	if ((nlayers = AG_ReadUint32(buf)) < 1) {
		AG_SetError("missing vg layer 0");
		goto fail;
	}
	vg->layers = Realloc(vg->layers, nlayers*sizeof(VG_Layer));
	for (i = 0; i < nlayers; i++) {
		VG_Layer *layer = &vg->layers[i];

		AG_CopyString(layer->name, buf, sizeof(layer->name));
		layer->visible = (int)AG_ReadUint8(buf);
		layer->color = VG_ReadColor(buf);
		layer->alpha = AG_ReadUint8(buf);
	}
	vg->nlayers = nlayers;

	/* Read the block information. */
	VG_DestroyBlocks(vg);
	nblocks = AG_ReadUint32(buf);
	for (i = 0; i < nblocks; i++) {
		VG_Block *vgb;

		vgb = Malloc(sizeof(VG_Block));
		AG_CopyString(vgb->name, buf, sizeof(vgb->name));
		vgb->flags = (int)AG_ReadUint32(buf);
		vgb->pos = VG_ReadVertex(buf);
		vgb->origin = VG_ReadVertex(buf);
		vgb->theta = AG_ReadFloat(buf);
		TAILQ_INIT(&vgb->vges);
		TAILQ_INSERT_TAIL(&vg->blocks, vgb, vgbs);
	}

	/* Read the global style information. */
	VG_DestroyStyles(vg);
	nstyles = AG_ReadUint32(buf);
	for (i = 0; i < nstyles; i++) {
		char sname[VG_STYLE_NAME_MAX];
		enum vg_style_type type;
		VG_Style *vgs;

		AG_CopyString(sname, buf, sizeof(sname));
		type = (enum vg_style_type)AG_ReadUint8(buf);
		vgs = VG_CreateStyle(vg, type, sname);
		vgs->color = VG_ReadColor(buf);

		switch (type) {
		case VG_LINE_STYLE:
			vgs->vg_line_st.style = AG_ReadUint8(buf);
			vgs->vg_line_st.endpoint_style = AG_ReadUint8(buf);
			vgs->vg_line_st.stipple = AG_ReadUint16(buf);
			vgs->vg_line_st.thickness = AG_ReadUint8(buf);
			vgs->vg_line_st.miter_len = AG_ReadUint8(buf);
			break;
		case VG_FILL_STYLE:
			vgs->vg_fill_st.style = AG_ReadUint8(buf);
			AG_CopyString(vgs->vg_fill_st.texture, buf,
			    sizeof(vgs->vg_fill_st.texture));
			vgs->vg_fill_st.texture_alpha = AG_ReadUint8(buf);
			break;
		case VG_TEXT_STYLE:
			AG_CopyString(vgs->vg_text_st.face, buf,
			    sizeof(vgs->vg_text_st.face));
			vgs->vg_text_st.size = (int)AG_ReadUint8(buf);
			vgs->vg_text_st.flags = (int)AG_ReadUint32(buf);
			break;
		}
	}

	/* Read the vg elements. */
	VG_DestroyElements(vg);
	nelements = AG_ReadUint32(buf);
	for (i = 0; i < nelements; i++) {
		char block_id[VG_BLOCK_NAME_MAX];
		enum vg_element_type type;
		VG_Element *vge;
		VG_Block *block;
		Uint32 nlayer;
		int j;
	
		type = (enum vg_element_type)AG_ReadUint32(buf);
		AG_CopyString(block_id, buf, sizeof(block_id));
		nlayer = (int)AG_ReadUint32(buf);

		vge = VG_Begin(vg, type);
		vge->color = VG_ReadColor(buf);

		/* Load the line style information. */
		vge->line_st.style = AG_ReadUint8(buf);
		vge->line_st.endpoint_style = AG_ReadUint8(buf);
		vge->line_st.stipple = AG_ReadUint16(buf);
		vge->line_st.thickness = AG_ReadUint8(buf);
		vge->line_st.miter_len = AG_ReadUint8(buf);

		/* Load the filling style information. */
		vge->fill_st.style = AG_ReadUint8(buf);
		AG_CopyString(vge->fill_st.texture, buf,
		    sizeof(vge->fill_st.texture));
		vge->fill_st.texture_alpha = AG_ReadUint8(buf);

		/* Load the text style information. */
		AG_CopyString(vge->text_st.face, buf,
		    sizeof(vge->text_st.face));
		vge->text_st.size = (int)AG_ReadUint8(buf);
		vge->text_st.flags = (int)AG_ReadUint32(buf);

		/* Load the vertices. */
		vge->nvtx = (Uint)AG_ReadUint32(buf);
		vge->vtx = Malloc(vge->nvtx*sizeof(VG_Vtx));
		for (j = 0; j < vge->nvtx; j++)
			vge->vtx[j] = VG_ReadVertex(buf);

		/* Load the matrices. */
		vge->ntrans = (Uint)AG_ReadUint32(buf);
		vge->trans = Malloc(vge->ntrans*sizeof(VG_Matrix));
		for (j = 0; j < vge->ntrans; j++)
			VG_LoadMatrix(&vge->trans[j], buf);
			
		/* Associate the element with a block if necessary. */
		if (block_id[0] != '\0') {
			TAILQ_FOREACH(block, &vg->blocks, vgbs) {
				if (strcmp(block->name, block_id) == 0)
					break;
			}
			if (block == NULL) {
				AG_SetError("unexisting vg block: %s",
				    block_id);
				goto fail;
			}
		} else {
			block = NULL;
		}
		if (block != NULL) {
			TAILQ_INSERT_TAIL(&block->vges, vge, vgbmbs);
			vge->block = block;
		}

		/* Load element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			if (vge->nvtx < 1) {
				AG_SetError("circle nvtx < 1");
				VG_DestroyElement(vg, vge);
				goto fail;
			}
			vge->vg_circle.radius = AG_ReadFloat(buf);
			break;
		case VG_ARC:
		case VG_ELLIPSE:
			if (vge->nvtx < 1) {
				AG_SetError("arc nvtx < 1");
				VG_DestroyElement(vg, vge);
				goto fail;
			}
			vge->vg_arc.w = AG_ReadFloat(buf);
			vge->vg_arc.h = AG_ReadFloat(buf);
			vge->vg_arc.s = AG_ReadFloat(buf);
			vge->vg_arc.e = AG_ReadFloat(buf);
			break;
		case VG_TEXT:
			if (vge->nvtx < 1) {
				AG_SetError("text nvtx < 1");
				VG_DestroyElement(vg, vge);
				goto fail;
			}
			AG_CopyString(vge->vg_text.text, buf,
			    sizeof(vge->vg_text.text));
			vge->vg_text.angle = AG_ReadFloat(buf);
			vge->vg_text.align = AG_ReadUint8(buf);
			break;
		case VG_MASK:
			vge->vg_mask.scale = AG_ReadFloat(buf);
			vge->vg_mask.visible = (int)AG_ReadUint8(buf);
			AG_ReadString(buf);			       /* Pad */
			break;
		default:
			break;
		}
		VG_End(vg);
	}
	AG_MutexUnlock(&vg->lock);
	return (0);
fail:
	AG_MutexUnlock(&vg->lock);
	return (-1);
}

void
VG_WriteVertex(AG_DataSource *buf, const VG_Vtx *vtx)
{
	AG_WriteFloat(buf, vtx->x);
	AG_WriteFloat(buf, vtx->y);
}

void
VG_WriteColor(AG_DataSource *buf, const VG_Color *c)
{
	AG_WriteUint8(buf, c->r);
	AG_WriteUint8(buf, c->g);
	AG_WriteUint8(buf, c->b);
	AG_WriteUint8(buf, c->a);
}

VG_Vtx
VG_ReadVertex(AG_DataSource *buf)
{
	VG_Vtx v;

	v.x = AG_ReadFloat(buf);
	v.y = AG_ReadFloat(buf);
	return (v);
}

VG_Color
VG_ReadColor(AG_DataSource *buf)
{
	VG_Color c;

	c.r = AG_ReadUint8(buf);
	c.g = AG_ReadUint8(buf);
	c.b = AG_ReadUint8(buf);
	c.a = AG_ReadUint8(buf);
	return (c);
}
