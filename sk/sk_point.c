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
 * Point in sketch.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Point *
SK_PointNew(void *pnode)
{
	SK_Point *pt;

	pt = Malloc(sizeof(SK_Point));
	SK_PointInit(pt, SK_GenNodeName(SKNODE(pnode)->sk, "Point"));
	SK_NodeAttach(pnode, pt);
	return (pt);
}

void
SK_PointInit(void *p, Uint name)
{
	SK_Point *pt = p;

	SK_NodeInit(pt, &skPointOps, name, 0);
	pt->size = 3.0;
	pt->color = M_ColorRGB(0.0, 0.0, 0.0);
	pt->flags = 0;
}

int
SK_PointLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Point *pt = p;

	pt->size = M_ReadReal(buf);
	pt->color = M_ReadColor(buf);
	return (0);
}

int
SK_PointSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Point *pt = p;

	M_WriteReal(buf, pt->size);
	M_WriteColor(buf, &pt->color);
	return (0);
}

void
SK_PointDraw(void *p, SK_View *skv)
{
	SK_Point *pt = p;
	M_Vector3 v = SK_Pos(pt);
	M_Color color;
	M_Real i, wPixel,hPixel, size;

	color = SK_NodeColor(pt, &pt->color);
	wPixel = skv->wPixel;
	hPixel = skv->hPixel;
	size = pt->size;

	/* Render scale-invariant crosshairs */
	GL_PushMatrix();
	GL_Translatev(&v);
	GL_Begin(GL_LINES);
	GL_Color3v(&color);
	GL_Vertex2(0.0, 0.0);	GL_Vertex2(-size*wPixel, 0.0);
	GL_Vertex2(0.0, 0.0);	GL_Vertex2( size*wPixel, 0.0);
	GL_Vertex2(0.0, 0.0);	GL_Vertex2(0.0, -size*hPixel);
	GL_Vertex2(0.0, 0.0);	GL_Vertex2(0.0, +size*hPixel);
	GL_End();

	if (SKNODE(pt)->flags & SK_NODE_MOUSEOVER) {
		GL_Begin(GL_LINE_LOOP);
		GL_Color3v(&color);
		for (i = 0.0; i < M_PI*2.0; i += (2.0*M_PI)/6.0) {
			GL_Vertex2(Cos(i)*wPixel*4.0,
			           Sin(i)*hPixel*4.0);
		}
		GL_End();
	}
	GL_PopMatrix();
}

void
SK_PointSize(SK_Point *pt, M_Real size)
{
	pt->size = size;
}

void
SK_PointEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Point *pt = p;
	const char *unit = skv->sk->uLen->abbr;
//	AG_HSVPal *pal;

	M_NumericalNewReal(box, 0, unit, _("Size: "), &pt->size);

//	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
//	M_BindReal(pal, "RGBAv", (void *)&pt->color);
}

void
SK_PointColor(SK_Point *pt, M_Color c)
{
	pt->color = c;
}

M_Real
SK_PointProximity(void *p, const M_Vector3 *v, M_Vector3 *vC)
{
	M_Vector3 pv = SK_Pos(p);

	M_VecCopy3(vC, &pv);
	return M_VecDistance3p(v, &pv);
}

int
SK_PointDelete(void *p)
{
	SK_Point *pt = p;
	SK *sk = SKNODE(pt)->sk;
	int rv;

	rv = SK_NodeDel(pt);
	SK_Update(sk);
	return (rv);
}

int
SK_PointMove(void *p, const M_Vector3 *pos, const M_Vector3 *vel)
{
	SK_Translatev(p, vel);
	return (1);
}

/* Points in 2D require two constraints. */
SK_Status
SK_PointConstrained(void *p)
{
	SK_Point *pt = p;

	if (SKNODE(pt)->nEdges == 2) {
		return (SK_WELL_CONSTRAINED);
	} else if (SKNODE(pt)->nEdges < 2) {
		return (SK_UNDER_CONSTRAINED);
	} else {
		return (SK_OVER_CONSTRAINED);
	}
}

SK_NodeOps skPointOps = {
	"Point",
	sizeof(SK_Point),
	0,
	SK_PointInit,
	NULL,		/* destroy */
	SK_PointLoad,
	SK_PointSave,
	SK_PointDraw,
	NULL,		/* redraw */
	SK_PointEdit,
	SK_PointProximity,
	SK_PointDelete,
	SK_PointMove,
	SK_PointConstrained
};

static int
ToolMouseButtonDown(void *_Nonnull pTool, M_Vector3 pos, int btn)
{
	SK_Tool *tool = pTool;
	SK *sk = tool->skv->sk;
	SK_Point *pt;
	
	if (btn != AG_MOUSE_LEFT)
		return (0);

	pt = Malloc(sizeof(SK_Point));
	SK_PointInit(pt, SK_GenNodeName(sk, "Point"));
	SK_NodeAttach(sk->root, pt);
	M_MatTranslate44(&SKNODE(pt)->T, pos.x, pos.y, 0.0);
	
	SK_Update(sk);
	return (0);
}

SK_ToolOps skPointToolOps = {
	N_("Point"),
	N_("Insert a point into the sketch"),
	NULL,
	sizeof(SK_Tool),
	0,
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* motion */
	ToolMouseButtonDown,
	NULL,			/* buttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
