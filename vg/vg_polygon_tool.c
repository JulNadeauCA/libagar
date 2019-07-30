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
 * Polygon tool.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

typedef struct vg_polygon_tool {
	VG_Tool _inherit;
	VG_Polygon *vpCur;
	int vtxCur;
	Uint32 _pad;
} VG_PolygonTool;

static void
Init(void *_Nonnull obj)
{
	VG_PolygonTool *t = obj;

	t->vpCur = NULL;
	t->vtxCur = 0;
}

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector vPos, int button)
{
	VG_PolygonTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;
	VG_Point *pt;

	switch (button) {
	case AG_MOUSE_LEFT:
		if (t->vpCur == NULL) {
			if ((pt = VG_NearestPoint(vv, vPos, NULL)) == NULL) {
				pt = VG_PointNew(vg->root, vPos);
			}
			t->vpCur = VG_PolygonNew(vg->root);
			VG_PolygonVertex(t->vpCur, pt);
			pt = VG_PointNew(vg->root, vPos);
			VG_PolygonVertex(t->vpCur, pt);
			t->vtxCur = 1;
		} else {
			if ((pt = VG_NearestPoint(vv, vPos,
			    t->vpCur->pts[t->vtxCur])) == NULL) {
				pt = VG_PointNew(vg->root, vPos);
			}
			VG_PolygonVertex(t->vpCur, pt);
			t->vtxCur++;
		}
		return (1);
	case AG_MOUSE_RIGHT:
		if (t->vpCur != NULL) {
			VG_Point *ptLast = t->vpCur->pts[t->vtxCur];

			VG_PolygonDelVertex(t->vpCur, t->vtxCur);
			(void)VG_Delete(ptLast);	/* No-op if in use */

			t->vpCur = NULL;
			t->vtxCur = 0;
		}
		return (1);
	default:
		return (0);
	}
}

static void
PostDraw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_PolygonTool *t = obj;
	AG_Color c;
	int x, y;

	VG_GetViewCoords(vv, VGTOOL(t)->vCursor, &x,&y);
	c = VG_MapColorRGB(vv->vg->selectionColor);
	AG_DrawCircle(vv, x,y, 3, &c);
}

static int
MouseMotion(void *_Nonnull obj, VG_Vector vPos, VG_Vector vRel, int b)
{
	VG_PolygonTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Polygon *vp;
	VG_Point *pt;
	
	if ((vp = t->vpCur) != NULL) {
		if ((pt = VG_NearestPoint(vv, vPos, vp->pts[t->vtxCur]))) {
			VG_Status(vv, _("Use Point%u"),
			    (Uint)VGNODE(pt)->handle);
		} else {
			VG_Status(vv, _("Create vertex at %.2f,%.2f"),
			    vPos.x, vPos.y);
		}
		VG_SetPosition(vp->pts[t->vtxCur], vPos);
	} else {
		if ((pt = VG_NearestPoint(vv, vPos, NULL))) {
			VG_Status(vv, _("Start Polygon at Point%u"),
			    (Uint)VGNODE(pt)->handle);
		} else {
			VG_Status(vv, _("Start Polygon at %.2f,%.2f"), vPos.x,
			    vPos.y);
		}
	}
	return (0);
}

VG_ToolOps vgPolygonTool = {
	N_("Polygon"),
	N_("Create polygon from sets of points."),
	&vgIconPolygon,
	sizeof(VG_PolygonTool),
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
