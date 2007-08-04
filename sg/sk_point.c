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
 * Point in sketch.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"
#include "sk_view.h"
#include "sg_gui.h"

#include <gui/hsvpal.h>

SK_Point *
SK_PointNew(void *pnode)
{
	SK_Point *pt;

	pt = Malloc(sizeof(SK_Point), M_SG);
	SK_PointInit(pt, SK_GenNodeName(SKNODE(pnode)->sk, "Point"));
	SK_NodeAttach(pnode, pt);
	return (pt);
}

void
SK_PointInit(void *p, Uint32 name)
{
	SK_Point *pt = p;

	SK_NodeInit(pt, &skPointOps, name, 0);
	pt->size = 3.0;
	pt->color = SG_ColorRGB(0.0, 0.0, 0.0);
	pt->flags = 0;
}

int
SK_PointLoad(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Point *pt = p;

	pt->size = SG_ReadReal(buf);
	pt->color = SG_ReadColor(buf);
	dprintf("%s: size=%f\n", SK_NodeName(pt), pt->size);
	return (0);
}

int
SK_PointSave(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Point *pt = p;

	SG_WriteReal(buf, pt->size);
	SG_WriteColor(buf, &pt->color);
	return (0);
}

void
SK_PointDraw(void *p, SK_View *skv)
{
	SK_Point *pt = p;
	SG_Color color = SK_NodeColor(pt, &pt->color);
	SG_Real i;

	SG_Begin(SG_LINES);
	SG_Color3v(&color);
	SG_Vertex2(0.0, 0.0);	SG_Vertex2(-pt->size*skv->wPixel, 0.0);
	SG_Vertex2(0.0, 0.0);	SG_Vertex2(+pt->size*skv->wPixel, 0.0);
	SG_Vertex2(0.0, 0.0);	SG_Vertex2(0.0, -pt->size*skv->hPixel);
	SG_Vertex2(0.0, 0.0);	SG_Vertex2(0.0, +pt->size*skv->hPixel);
	SG_End();

	if (SKNODE(pt)->flags & SK_NODE_MOUSEOVER) {
		SG_Begin(SG_LINE_LOOP);
		SG_Color3v(&color);
//		SG_Color3f(0.0, 0.0, 1.0);
		for (i = 0.0; i < M_PI*2.0; i += (2.0*M_PI)/6.0) {
			SG_Vertex2(SG_Cos(i)*skv->wPixel*4.0,
			           SG_Sin(i)*skv->hPixel*4.0);
		}
		SG_End();
	}
}

void
SK_PointSize(SK_Point *pt, SG_Real size)
{
	pt->size = size;
}

void
SK_PointEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Point *pt = p;
//	AG_HSVPal *pal;

	SG_SpinReal(box, _("Size: "), &pt->size);

//	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
//	SG_WidgetBindReal(pal, "RGBAv", (void *)&pt->color);
}

void
SK_PointColor(SK_Point *pt, SG_Color c)
{
	pt->color = c;
}

SG_Real
SK_PointProximity(void *p, const SG_Vector *v, SG_Vector *vC)
{
	SG_Vector pv = SK_NodeCoords(p);

	SG_CopyVector(vC, &pv);
	return (SG_VectorDistancep(v, &pv));
}

int
SK_PointDelete(void *p)
{
	SK_Point *pt = p;

	return (SK_NodeDel(pt));
}

void
SK_PointMove(void *p, const SG_Vector *pos, const SG_Vector *vel)
{
	SK_Translatev(p, vel);
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
	NULL,		/* draw_absolute */
	NULL,		/* redraw */
	SK_PointEdit,
	SK_PointProximity,
	SK_PointDelete,
	SK_PointMove
};

#ifdef EDITION

static int
mousebuttondown(void *pTool, SG_Vector pos, int btn)
{
	SK_Tool *tool = pTool;
	SK *sk = tool->skv->sk;
	SK_Point *pt;
	
	if (btn != SDL_BUTTON_LEFT)
		return (0);

	pt = Malloc(sizeof(SK_Point), M_OBJECT);
	SK_PointInit(pt, SK_GenNodeName(sk, "Point"));
	SK_NodeAttach(sk->root, pt);
	SG_MatrixTranslate2(&SKNODE(pt)->T, pos.x, pos.y);
	
	SK_Update(sk);
	return (0);
}

SK_ToolOps skPointToolOps = {
	N_("Point"),
	N_("Insert a point into the sketch"),
	VGPOINTS_ICON,
	sizeof(SK_Tool),
	0,
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* edit */
	NULL,		/* motion */
	mousebuttondown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* EDITION */
#endif /* HAVE_OPENGL */
