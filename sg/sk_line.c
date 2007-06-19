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
	line->color = SG_ColorRGB(0.0, 1.0, 0.0);
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
	SG_Color3v(&line->color);
	SG_Vertex2v(&v1);
	SG_Vertex2v(&v2);
	SG_End();
}

void
SK_LineEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Line *line = p;
	AG_HSVPal *pal;

	SG_SpinReal(box, _("Width: "), &line->width);
	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
	SG_WidgetBindReal(pal, "RGBAv", (void *)&line->color);
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
	SK_LineEdit
};

#ifdef EDITION

struct sk_line_tool {
	SK_Tool tool;
	SK_Line	 *cur_line;
	SK_Point *cur_point;
};

static void
init(void *p)
{
	struct sk_line_tool *t = p;

	t->cur_line = NULL;
	t->cur_point = NULL;
}

static int
mousemotion(void *p, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_line_tool *t = p;
	
	if (t->cur_line != NULL) {
		SK_Identity(t->cur_point);
		SK_Translatev(t->cur_point, &pos);
	}
	return (0);
}

static int
mousebuttondown(void *p, SG_Vector pos, int btn)
{
	struct sk_line_tool *t = p;
	SK *sk = SKTOOL(t)->skv->sk;
	SK_Line *line;

	if (btn != SDL_BUTTON_LEFT)
		return (0);

	if (t->cur_line != NULL) {
		t->cur_line = NULL;
		t->cur_point = NULL;
		return (1);
	}
	line = SK_LineNew(sk->root);
	line->p1 = SK_PointNew(line);
	line->p2 = SK_PointNew(line);
	SK_NodeAddReference(line, line->p1);
	SK_NodeAddReference(line, line->p2);
	SK_Translatev(line->p1, &pos);
	SK_Translatev(line->p2, &pos);
	t->cur_line = line;
	t->cur_point = line->p2;
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
