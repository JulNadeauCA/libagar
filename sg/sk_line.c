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
 * Basic line segment.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"
#include "sk_view.h"
#include "sg_gui.h"

#include <gui/hsvpal.h>

SK_Line *
SK_LineNew(void *pnode)
{
	SK_Line *line;

	line = Malloc(sizeof(SK_Line), M_SG);
	SK_LineInit(line, SK_GenName(SKNODE(pnode)->sk));
	SK_NodeAttach(pnode, line);
	return (line);
}

void
SK_LineInit(void *p, Uint32 name)
{
	SK_Line *line = p;

	SK_NodeInit(line, &skLineOps, name, 0);
	line->width = 1.0;
	line->color = SG_ColorRGB(0.0, 0.0, 0.0);
	line->p1 = NULL;
	line->p2 = NULL;
}

int
SK_LineLoad(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Line *line = p;

	line->width = SG_ReadReal(buf);
	line->color = SG_ReadColor(buf);
	line->p1 = SK_ReadRef(buf, sk, "Point");
	line->p2 = SK_ReadRef(buf, sk, "Point");
	if (line->p1 == NULL || line->p2 == NULL) {
		AG_SetError("Missing endpoint (%s)", AG_GetError());
		return (-1);
	}
	dprintf("%s: width=%f, p1=%s, p2=%s\n", SK_NodeName(line),
	    line->width, SK_NodeName(line->p1), SK_NodeName(line->p2));
	return (0);
}

int
SK_LineSave(SK *sk, void *p, AG_Netbuf *buf)
{
	SK_Line *line = p;

	SG_WriteReal(buf, line->width);
	SG_WriteColor(buf, &line->color);
	SK_WriteRef(buf, line->p1);
	SK_WriteRef(buf, line->p2);
	return (0);
}

void
SK_LineDraw(void *p, SK_View *skv)
{
	SK_Line *line = p;
	SG_Vector v1 = SK_NodeCoords(line->p1);
	SG_Vector v2 = SK_NodeCoords(line->p2);

	SG_Begin(SG_LINES);
	if (SKNODE(line)->flags & SK_NODE_MOUSEOVER) {
		SG_Color3f(0.0, 0.0, 1.0);
	} else if (SKNODE_SELECTED(line)) {
		SG_Color3f(0.0, 1.0, 0.0);
	} else {
		SG_Color3v(&line->color);
	}
	SG_Vertex2v(&v1);
	SG_Vertex2v(&v2);
	SG_End();
}

void
SK_LineEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Line *line = p;
//	AG_HSVPal *pal;

	SG_SpinReal(box, _("Width: "), &line->width);
//	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
//	SG_WidgetBindReal(pal, "RGBAv", (void *)&line->color);
}

SG_Real
SK_LineProximity(void *p, const SG_Vector *v, SG_Vector *vC)
{
	SK_Line *line = p;
	SG_Vector p1 = SK_NodeCoords(line->p1);
	SG_Vector p2 = SK_NodeCoords(line->p2);
	SG_Real mag = SG_VectorDistance(p2, p1);
	SG_Real u;

	u = ( ((v->x - p1.x)*(p2.x - p1.x)) +
              ((v->y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);
	if (u < 0.0 || u > 1.0)
		return (HUGE_VAL);

	*vC = SG_VectorAdd(p1, SG_VectorScale(SG_VectorSubp(&p2,&p1), u));
	return (SG_VectorDistancep(v, vC));
}

int
SK_LineDelete(void *p)
{
	SK_Line *line = p;

	SK_NodeDelReference(line, line->p1);
	SK_NodeDelReference(line, line->p2);

	if (SKNODE(line->p1)->nRefs == 0)
		SK_NodeDel(line->p1);
	if (SKNODE(line->p2)->nRefs == 0)
		SK_NodeDel(line->p2);

	return (SK_NodeDel(line));
}

void
SK_LineWidth(SK_Line *line, SG_Real size)
{
	line->width = size;
}

void
SK_LineColor(SK_Line *line, SG_Color c)
{
	line->color = c;
}

SK_NodeOps skLineOps = {
	"Line",
	sizeof(SK_Line),
	0,
	SK_LineInit,
	NULL,		/* destroy */
	SK_LineLoad,
	SK_LineSave,
	NULL,		/* draw_relative */
	SK_LineDraw,
	SK_LineEdit,
	SK_LineProximity,
	SK_LineDelete
};

#ifdef EDITION

struct sk_line_tool {
	SK_Tool tool;
	SK_Line	 *curLine;
	SK_Point *curPoint;
};

static void
init(void *p)
{
	struct sk_line_tool *t = p;

	t->curLine = NULL;
	t->curPoint = NULL;
}

static SK_Point *
OverPoint(SK_View *skv, SG_Vector *pos, SG_Vector *vC, void *ignore)
{
	SK *sk = skv->sk;
	SK_Node *node;
	
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->flags &= ~(SK_NODE_MOUSEOVER);
	}
	if ((node = SK_ProximitySearch(sk, "Point", pos, vC, ignore)) != NULL &&
	    SG_VectorDistance2p(pos, vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		return ((SK_Point *)node);
	}
	return (NULL);
}

static int
mousemotion(void *p, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_line_tool *t = p;
	SG_Vector vC;
	SK_Point *overPoint;

	overPoint = OverPoint(SKTOOL(t)->skv, &pos, &vC, t->curPoint);
	if (t->curLine != NULL) {
		SK_Identity(t->curPoint);
		SK_Translatev(t->curPoint, (overPoint != NULL) ? &vC : &pos);
	}
	return (0);
}

static int
mousebuttondown(void *p, SG_Vector pos, int btn)
{
	struct sk_line_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Point *overPoint;
	SK_Line *line;
	SG_Vector vC;

	if (btn != SDL_BUTTON_LEFT)
		return (0);

	overPoint = OverPoint(skv, &pos, &vC, t->curPoint);

	if ((line = t->curLine) != NULL) {
		if (overPoint != NULL &&
		    overPoint != line->p2) {
		    	SK_NodeDelReference(line, line->p2);
			if (SK_NodeDel(line->p2)) {
				AG_TextMsgFromError();
				return (0);
			}
		    	SK_NodeAddReference(line, overPoint);
			line->p2 = overPoint;
		}
		t->curLine = NULL;
		t->curPoint = NULL;
		return (1);
	}

	line = SK_LineNew(sk->root);
	if (overPoint != NULL) {
		line->p1 = overPoint;
	} else {
		line->p1 = SK_PointNew(sk->root);
		SK_Translatev(line->p1, &pos);
	}
	line->p2 = SK_PointNew(sk->root);
	SK_Translatev(line->p2, &pos);
	
	SK_NodeAddReference(line, line->p1);
	SK_NodeAddReference(line, line->p2);
	t->curLine = line;
	t->curPoint = line->p2;
	return (1);
}

SK_ToolOps skLineToolOps = {
	N_("Line segment"),
	N_("Insert a line segment into the sketch"),
	VGLINES_ICON,
	sizeof(struct sk_line_tool),
	0,
	init,
	NULL,		/* destroy */
	NULL,		/* edit */
	mousemotion,
	mousebuttondown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* EDITION */
#endif /* HAVE_OPENGL */
