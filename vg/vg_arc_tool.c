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
 * Arc tool.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

typedef struct vg_arc_tool {
	VG_Tool _inherit;
	VG_Arc *_Nullable vaCur;		/* Arc being edited */
} VG_ArcTool;

static void
Init(void *_Nonnull obj)
{
	VG_ArcTool *t = obj;

	t->vaCur = NULL;
}

static __inline__ void
AdjustRadius(VG_Arc *_Nonnull va, VG_Vector vPos)
{
	va->r = VG_Distance(vPos, VG_Pos(va->p));
}

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector vPos, int button)
{
	VG_ArcTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;
	VG_Point *pCenter;

	switch (button) {
	case AG_MOUSE_LEFT:
		if (t->vaCur == NULL) {
			if (!(pCenter = VG_NearestPoint(vv, vPos, NULL))) {
				pCenter = VG_PointNew(vg->root, vPos);
			}
			t->vaCur = VG_ArcNew(vg->root, pCenter, 1.0f,
			    0.0f, (float)VG_PI);
			AdjustRadius(t->vaCur, vPos);
		} else {
			AdjustRadius(t->vaCur, vPos);
			t->vaCur = NULL;
		}
		return (1);
	case AG_MOUSE_RIGHT:
		if (t->vaCur != NULL) {
			VG_Delete(t->vaCur);
			t->vaCur = NULL;
		}
		return (1);
	default:
		return (0);
	}
}

static int
MouseMotion(void *_Nonnull obj, VG_Vector vPos, VG_Vector vRel, int buttons)
{
	VG_ArcTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Point *pEx;
	
	if (t->vaCur != NULL) {
		AdjustRadius(t->vaCur, vPos);
		VG_Status(vv, _("Set radius: %.2f"), t->vaCur->r);
	} else {
		if ((pEx = VG_NearestPoint(vv, vPos, NULL))) {
			VG_Status(vv, _("Use Point%u as center"),
			    (Uint)VGNODE(pEx)->handle);
		} else {
			VG_Status(vv, _("Arc center at %.2f,%.2f"),
			    vPos.x, vPos.y);
		}
	}
	return (0);
}

static void
PostDraw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_Tool *t = obj;
	AG_Color c;
	int x, y;

	VG_GetViewCoords(vv, t->vCursor, &x,&y);
	c = VG_MapColorRGB(vv->vg->selectionColor);
	AG_DrawCircle(vv, x,y, 3, &c);
}

VG_ToolOps vgArcTool = {
	N_("Arc"),
	N_("Insert arcs from centerpoint and angles."),
	&vgIconCircle,
	sizeof(VG_ArcTool),
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
