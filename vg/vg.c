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

const VG_NodeOps *vgNodeTypes[] = {
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
	NULL,			/* Bezier curve */
	NULL,			/* Bezigon */
	&vgTextOps
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
	if (!vgInited) {
		VG_InitSubsystem();
		vgInited = 1;
	}

	vg->flags = flags;
	vg->fillColor = VG_GetColorRGB(0,0,0);
	vg->gridColor = VG_GetColorRGB(200,200,0);
	vg->selectionColor = VG_GetColorRGB(255,255,0);
	vg->mouseoverColor = VG_GetColorRGB(200,200,0);
	vg->layers = NULL;
	vg->nlayers = 0;
	vg->curLayer = 0;
	vg->curBlock = NULL;
	vg->curNode = NULL;
	vg->ints = NULL;
	vg->nints = 0;
	TAILQ_INIT(&vg->nodes);
	TAILQ_INIT(&vg->blocks);
	TAILQ_INIT(&vg->styles);
	AG_MutexInitRecursive(&vg->lock);
	
	VG_PushLayer(vg, _("Layer 0"));
}

void
VG_FreeNode(VG *vg, VG_Node *vge)
{
	if (vge->ops->destroy != NULL) {
		vge->ops->destroy(vg, vge);
	}
	Free(vge->vtx);
	Free(vge);
}

static void
FreeNodes(VG *vg)
{
	VG_Node *vge, *nvge;
	
	for (vge = TAILQ_FIRST(&vg->nodes);
	     vge != TAILQ_END(&vg->nodes);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, nodes);
		VG_FreeNode(vg, vge);
	}
	TAILQ_INIT(&vg->nodes);
}

static void
FreeBlocks(VG *vg)
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
FreeStyles(VG *vg)
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
	FreeBlocks(vg);
	FreeNodes(vg);
	FreeStyles(vg);
}

void
VG_Destroy(VG *vg)
{
	VG_Reinit(vg);
	Free(vg->layers);
	Free(vg->ints);
	AG_MutexDestroy(&vg->lock);
}

void
VG_DestroyNode(VG *vg, VG_Node *vge)
{
	if (vge->block != NULL)
		TAILQ_REMOVE(&vge->block->nodes, vge, vgbmbs);

	if (vg->curNode == vge)
		vg->curNode = NULL;

	TAILQ_REMOVE(&vg->nodes, vge, nodes);
	VG_FreeNode(vg, vge);
}

void VG_SetBackgroundColor(VG *vg, VG_Color c) { vg->fillColor = c; }
void VG_SetGridColor(VG *vg, VG_Color c) { vg->gridColor = c; }
void VG_SetSelectionColor(VG *vg, VG_Color c) { vg->selectionColor = c; }
void VG_SetMouseOverColor(VG *vg, VG_Color c) { vg->mouseoverColor = c; }

/*
 * Allocate the given type of element and begin its parametrization.
 * If a block is selected, associate the element with it.
 */
VG_Node *
VG_Begin(VG *vg, enum vg_node_type nType)
{
	VG_Node *vge;

	vge = Malloc(sizeof(VG_Node));
	vge->flags = 0;
	vge->type = nType;
	vge->style = NULL;
	vge->layer = vg->curLayer;
	vge->vtx = NULL;
	vge->nvtx = 0;
	vge->color = VG_GetColorRGB(250,250,250);

	vge->line_st.style = VG_CONTINUOUS;
	vge->line_st.endpoint_style = VG_SQUARE;
	vge->line_st.stipple = 0x0;
	vge->line_st.thickness = 1;
	vge->line_st.miter_len = 0;
	vge->fill_st.style = VG_SOLID;
	vge->fill_st.texture[0] = '\0';
	vge->fill_st.texture_alpha = 255;
	vge->text_st.size = agDefaultFont->size;
	vge->text_st.flags = agDefaultFont->flags;
	Strlcpy(vge->text_st.face, OBJECT(agDefaultFont)->name,
	    sizeof(vge->text_st.face));

	VG_Lock(vg);

	TAILQ_INSERT_TAIL(&vg->nodes, vge, nodes);
	if (vg->curBlock != NULL) {
		TAILQ_INSERT_TAIL(&vg->curBlock->nodes, vge, vgbmbs);
		vge->block = vg->curBlock;
		if (vge->block->flags & VG_BLOCK_NOSAVE)
			vge->flags |= VG_NODE_NOSAVE;
	} else {
		vge->block = NULL;
	}
	vge->ops = vgNodeTypes[nType];
	if (vge->ops->init != NULL) {
		vge->ops->init(vg, vge);
	}
	vg->curNode = vge;
	VG_LoadIdentity(vg);
	return (vge);
}

/* End parametrization of the current element. */
void
VG_End(VG *vg)
{
	vg->curNode = NULL;
	VG_Unlock(vg);
}

/* Select the given element for edition. */
void
VG_Select(VG *vg, VG_Node *vge)
{
	VG_Lock(vg);
	vg->curNode = vge;
}

VG_Vtx *
VG_PopVertex(VG *vg)
{
	VG_Node *vge = vg->curNode;

	if (vge->vtx == NULL)
		return (NULL);
#ifdef DEBUG
	if (vge->nvtx-1 < 0)
		AG_FatalError("VG_PopVertex: Negative vertex count");
#endif
	vge->vtx = Realloc(vge->vtx, (--vge->nvtx)*sizeof(VG_Vtx));
	return ((vge->nvtx > 0) ? &vge->vtx[vge->nvtx-1] : NULL);
}

/* Push a 2D vertex onto the vertex array. */
VG_Vtx *
VG_Vertex2(VG *vg, float x, float y)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->curNode);
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

	vtx = VG_AllocVertex(vg->curNode);
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
	VG_Node *vge = vg->curNode;
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

	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x1;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);

	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x2;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
}

/* Push the two endpoints of a vertical line onto the vertex array. */
void
VG_VLine(VG *vg, float x, float y1, float y2)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
}

/* Push the two endpoints of a horizontal line onto the vertex array. */
void
VG_HLine(VG *vg, float x1, float x2, float y)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x1;
	vtx->y = y;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x2;
	vtx->y = y;
	VG_BlockOffset(vg, vtx);
}

/* Generate a rectangle. */
void
VG_Rectangle(VG *vg, float x1, float y1, float x2, float y2)
{
	VG_Vtx *vtx;

	VG_Begin(vg, VG_LINE_LOOP);

	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x1;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x2;
	vtx->y = y1;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x2;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
	vtx = VG_AllocVertex(vg->curNode);
	vtx->x = x1;
	vtx->y = y2;
	VG_BlockOffset(vg, vtx);
	
	VG_End(vg);
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
VG_LoadIdentity(VG *vg)
{
	VG_Matrix *T = &vg->curNode->T;
	
	T->m[0][0] = 1.0; T->m[0][1] = 0.0; T->m[0][2] = 0.0;
	T->m[1][0] = 0.0; T->m[1][1] = 1.0; T->m[1][2] = 0.0;
	T->m[2][0] = 0.0; T->m[2][1] = 0.0; T->m[2][2] = 1.0;
}

void
VG_Translate(VG *vg, float x, float y)
{
	VG_Matrix *T = &vg->curNode->T;

	T->m[0][0] = 1.0; T->m[0][1] = 0.0; T->m[0][2] = x;
	T->m[1][0] = 0.0; T->m[1][1] = 1.0; T->m[1][2] = y;
	T->m[2][0] = 0.0; T->m[2][1] = 0.0; T->m[2][2] = 1.0;
}

void
VG_Rotate(VG *vg, float degs)
{
	VG_Matrix *T = &vg->curNode->T;
	float theta = Radians(degs);
	float rCos = Cos(theta);
	float rSin = Sin(theta);

	T->m[0][0] = +rCos;
	T->m[0][1] = -rSin;
	T->m[0][2] = 0.0;
	T->m[1][0] = +rSin;
	T->m[1][1] = +rCos;
	T->m[1][2] = 0.0;
	T->m[2][0] = 0.0;
	T->m[2][1] = 0.0;
	T->m[2][2] = 1.0;
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
	VG_Lock(vg);
	TAILQ_INSERT_TAIL(&vg->styles, vgs, styles);
	VG_Unlock(vg);
	return (vgs);
}

/* Associate the given style with the current element. */
int
VG_SetStyle(VG *vg, const char *name)
{
	VG_Node *vge = vg->curNode;
	VG_Style *st;

	VG_Lock(vg);
	TAILQ_FOREACH(st, &vg->styles, styles) {
		if (strcmp(st->name, name) == 0)
			break;
	}
	if (st == NULL) {
		AG_SetError(_("No such style: %s"), name);
		VG_Unlock(vg);
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
	VG_Unlock(vg);
	return (0);
}

/* Specify the layer# to associate with the current element. */
void
VG_SetLayer(VG *vg, int layer)
{
	vg->curNode->layer = layer;
}

/* Specify the color of the current element (VG color format). */
void
VG_Colorv(VG *vg, const VG_Color *c)
{
	vg->curNode->color.r = c->r;
	vg->curNode->color.g = c->g;
	vg->curNode->color.b = c->b;
	vg->curNode->color.a = c->a;
}

/* Specify the color of the current element (RGB triplet). */
void
VG_ColorRGB(VG *vg, Uint8 r, Uint8 g, Uint8 b)
{
	vg->curNode->color.r = r;
	vg->curNode->color.g = g;
	vg->curNode->color.b = b;
}

/* Specify the color of the current element (RGB triplet + alpha). */
void
VG_ColorRGBA(VG *vg, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	vg->curNode->color.r = r;
	vg->curNode->color.g = g;
	vg->curNode->color.b = b;
	vg->curNode->color.a = a;
}

/* Push a new layer onto the layer stack. */
VG_Layer *
VG_PushLayer(VG *vg, const char *name)
{
	VG_Layer *vgl;

	VG_Lock(vg);
	vg->layers = Realloc(vg->layers, (vg->nlayers+1) *
	                                 sizeof(VG_Layer));
	vgl = &vg->layers[vg->nlayers];
	vg->nlayers++;
	Strlcpy(vgl->name, name, sizeof(vgl->name));
	vgl->visible = 1;
	vgl->alpha = 255;
	vgl->color = VG_GetColorRGB(255,255,255);
	VG_Unlock(vg);
	return (vgl);
}

/* Pop the highest layer off the layer stack. */
void
VG_PopLayer(VG *vg)
{
	VG_Lock(vg);
	if (--vg->nlayers < 1) {
		vg->nlayers = 1;
	}
	VG_Unlock(vg);
}

static void
SaveMatrix(VG_Matrix *A, AG_DataSource *buf)
{
	int m, n;

	for (m = 0; m < 3; m++)
		for (n = 0; n < 3; n++)
			AG_WriteFloat(buf, A->m[m][n]);
}

static void
LoadMatrix(VG_Matrix *A, AG_DataSource *buf)
{
	int m, n;

	for (m = 0; m < 3; m++)
		for (n = 0; n < 3; n++)
			A->m[m][n] = AG_ReadFloat(buf);
}

void
VG_Save(VG *vg, AG_DataSource *buf)
{
	off_t nblocks_offs, nnodes_offs, nstyles_offs;
	Uint32 nblocks = 0, nnodes = 0, nstyles = 0;
	VG_Block *vgb;
	VG_Style *vgs;
	VG_Node *vge;
	int i;

	AG_WriteVersion(buf, "AG_VG", &vgVer);
	AG_WriteString(buf, "VG");

	VG_Lock(vg);

	AG_WriteUint32(buf, (Uint32)vg->flags);
	VG_WriteColor(buf, &vg->fillColor);
	VG_WriteColor(buf, &vg->gridColor);
	VG_WriteColor(buf, &vg->selectionColor);
	VG_WriteColor(buf, &vg->mouseoverColor);
	AG_WriteFloat(buf, 0.0f);				/* gridIval */
	AG_WriteUint32(buf, (Uint32)vg->curLayer);

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
	nnodes_offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vge, &vg->nodes, nodes) {
		if (vge->flags & VG_NODE_NOSAVE)
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
		
		/* Save the transformation matrix. */
		AG_WriteUint32(buf, 1);
		SaveMatrix(&vge->T, buf);

		/* Save element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			AG_WriteFloat(buf, vge->vg_circle.radius);
			break;
		case VG_ARC:
			AG_WriteFloat(buf, vge->vg_arc.w);
			AG_WriteFloat(buf, vge->vg_arc.h);
			AG_WriteFloat(buf, vge->vg_arc.s);
			AG_WriteFloat(buf, vge->vg_arc.e);
			break;
		case VG_TEXT:
			AG_WriteString(buf, vge->vg_text.text);
			AG_WriteFloat(buf, vge->vg_text.angle);
			AG_WriteUint8(buf, (Uint8)vge->vg_text.align);
			break;
		default:
			break;
		}
		nnodes++;
	}
	AG_WriteUint32At(buf, nnodes, nnodes_offs);
	VG_Unlock(vg);
}

int
VG_Load(VG *vg, AG_DataSource *buf)
{
	char name[VG_NAME_MAX];
	Uint32 nlayers, nstyles, nelements, nblocks;
	Uint32 i;

	if (AG_ReadVersion(buf, "AG_VG", &vgVer, NULL) != 0) {
		return (-1);
	}
	AG_CopyString(name, buf, sizeof(name));			/* Ignore */
	
	VG_Lock(vg);
	vg->flags = AG_ReadUint32(buf);
	vg->fillColor = VG_ReadColor(buf);
	vg->gridColor = VG_ReadColor(buf);
	vg->selectionColor = VG_ReadColor(buf);
	vg->mouseoverColor = VG_ReadColor(buf);
	(void)AG_ReadFloat(buf);				/* gridIval */
	vg->curLayer = (int)AG_ReadUint32(buf);
	vg->curBlock = NULL;
	vg->curNode = NULL;

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
	FreeBlocks(vg);
	nblocks = AG_ReadUint32(buf);
	for (i = 0; i < nblocks; i++) {
		VG_Block *vgb;

		vgb = Malloc(sizeof(VG_Block));
		AG_CopyString(vgb->name, buf, sizeof(vgb->name));
		vgb->flags = (int)AG_ReadUint32(buf);
		vgb->pos = VG_ReadVertex(buf);
		vgb->theta = AG_ReadFloat(buf);
		TAILQ_INIT(&vgb->nodes);
		TAILQ_INSERT_TAIL(&vg->blocks, vgb, vgbs);
	}

	/* Read the global style information. */
	FreeStyles(vg);
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
	FreeNodes(vg);
	nelements = AG_ReadUint32(buf);
	for (i = 0; i < nelements; i++) {
		char block_id[VG_BLOCK_NAME_MAX];
		enum vg_node_type type;
		VG_Node *vge;
		VG_Block *block;
		Uint32 nlayer;
		int j;
	
		type = (enum vg_node_type)AG_ReadUint32(buf);
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
		
		/* Load the transformation matrix. */
		(void)AG_ReadUint32(buf);				/* 1 */
		LoadMatrix(&vge->T, buf);
		
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
			TAILQ_INSERT_TAIL(&block->nodes, vge, vgbmbs);
			vge->block = block;
		}

		/* Load element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			if (vge->nvtx < 1) {
				AG_SetError("circle nvtx < 1");
				VG_DestroyNode(vg, vge);
				goto fail;
			}
			vge->vg_circle.radius = AG_ReadFloat(buf);
			break;
		case VG_ARC:
			if (vge->nvtx < 1) {
				AG_SetError("arc nvtx < 1");
				VG_DestroyNode(vg, vge);
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
				VG_DestroyNode(vg, vge);
				goto fail;
			}
			AG_CopyString(vge->vg_text.text, buf,
			    sizeof(vge->vg_text.text));
			vge->vg_text.angle = AG_ReadFloat(buf);
			vge->vg_text.align = AG_ReadUint8(buf);
			break;
		default:
			break;
		}
		VG_End(vg);
	}
	VG_Unlock(vg);
	return (0);
fail:
	VG_Unlock(vg);
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
