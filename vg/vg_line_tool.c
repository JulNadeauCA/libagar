/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Line tool.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

typedef struct vg_line_tool {
	VG_Tool _inherit;		/* VG_Tool(3) -> VG_LineTool */
	VG_Line *vlCur;			/* Line being edited */
} VG_LineTool;

static void
Init(void *_Nonnull obj)
{
	VG_LineTool *t = obj;

	t->vlCur = NULL;
}

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector vPos, int button)
{
	VG_LineTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;
	VG_Point *p1, *p2;

	switch (button) {
	case AG_MOUSE_LEFT:
		if (t->vlCur == NULL) {
			if (!(p1 = VG_NearestPoint(vv, vPos, NULL))) {
				p1 = VG_PointNew(vg->root, vPos);
			}
			p2 = VG_PointNew(vg->root, vPos);
			t->vlCur = VG_LineNew(vg->root, p1, p2);
		} else {
			if ((p2 = VG_NearestPoint(vv, vPos,
			    t->vlCur->p2))) {
				VG_DelRef(t->vlCur, t->vlCur->p2);
				VG_Delete(t->vlCur->p2);
				t->vlCur->p2 = p2;
				VG_AddRef(t->vlCur, p2);
			} else {
				VG_SetPosition(t->vlCur->p2, vPos);
			}
			t->vlCur = NULL;
		}
		return (1);
	case AG_MOUSE_RIGHT:
		if (t->vlCur != NULL) {
			VG_Delete(t->vlCur);
			t->vlCur = NULL;
		}
		return (1);
	default:
		return (0);
	}
}

static void
PostDraw(void *_Nonnull obj, VG_View *vv)
{
	VG_LineTool *t = obj;
	AG_Color c;
	int x, y;

	VG_GetViewCoords(vv, VGTOOL(t)->vCursor, &x,&y);
	c = VG_MapColorRGB(vv->vg->selectionColor);
	AG_DrawCircle(vv, x,y, 3, &c);
}

static int
MouseMotion(void *_Nonnull obj, VG_Vector vPos, VG_Vector vRel, int b)
{
	VG_LineTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Point *pEx;
	VG_Vector pos;
	float theta, rad;
	
	if (t->vlCur != NULL) {
		pEx = t->vlCur->p1;
		pos = VG_Pos(pEx);
		theta = VG_Atan2(vPos.y - pos.y,
		                 vPos.x - pos.x);
		rad = VG_Hypot(vPos.x - pos.x,
		               vPos.y - pos.y);
		if ((pEx = VG_NearestPoint(vv, vPos, t->vlCur->p2))) {
			VG_Status(vv, _("Use Point%u"),
			    (Uint)VGNODE(pEx)->handle);
		} else {
			VG_Status(vv,
			    _("Create Point at %.2f,%.2f (%.2f|%.2f\xc2\xb0)"),
			    vPos.x, vPos.y, rad, VG_Degrees(theta));
		}
		VG_SetPosition(t->vlCur->p2, vPos);
	} else {
		if ((pEx = VG_NearestPoint(vv, vPos, NULL))) {
			VG_Status(vv, _("Start Line at Point%u"),
			    (Uint)VGNODE(pEx)->handle);
		} else {
			VG_Status(vv, _("Start Line at %.2f,%.2f"), vPos.x,
			    vPos.y);
		}
	}
	return (0);
}

VG_ToolOps vgLineTool = {
	N_("Line"),
	N_("Insert line from two points."),
	&vgIconLine,
	sizeof(VG_LineTool),
	0,
	Init,
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* predraw */
	PostDraw,
	NULL,			/* selected */
	NULL,			/* deselected */
	MouseMotion,
	MouseButtonDown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
