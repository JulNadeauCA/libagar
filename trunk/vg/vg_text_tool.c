/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Text tool.
 */

#include <core/core.h>
#include <gui/widget.h>
#include <gui/primitive.h>
#include "vg.h"
#include "vg_view.h"
#include "icons.h"

typedef struct vg_text_tool {
	VG_Tool _inherit;
	VG_Text *vtCur;
} VG_TextTool;

static void
Init(void *p)
{
	VG_TextTool *t = p;

	t->vtCur = NULL;
}

static void
SetTextString(AG_Event *event)
{
	VG_Text *vt = AG_PTR(1);
	char *s = AG_STRING(2);

	VG_TextPrintf(vt, "%s", s);
}

static int
MouseButtonDown(void *p, VG_Vector vPos, int button)
{
	VG_TextTool *t = p;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;
	VG_Point *p1, *p2;

	switch (button) {
	case SDL_BUTTON_LEFT:
		if (t->vtCur == NULL) {
			if (!(p1 = VG_NearestPoint(vv, vPos, NULL))) {
				p1 = VG_PointNew(vg->root, vPos);
			}
			p2 = VG_PointNew(vg->root, vPos);
			t->vtCur = VG_TextNew(vg->root, p1, p2);
			VG_TextPrintf(t->vtCur, "(text)");
			AG_TextPromptString(_("Enter text string: "),
			    SetTextString, "%p", t->vtCur);
		} else {
			if ((p2 = VG_NearestPoint(vv, vPos,
			    t->vtCur->p2))) {
				VG_DelRef(t->vtCur, t->vtCur->p2);
				VG_Delete(t->vtCur->p2);
				t->vtCur->p2 = p2;
				VG_AddRef(t->vtCur, p2);
			} else {
				VG_SetPosition(t->vtCur->p2, vPos);
			}
			t->vtCur = NULL;
		}
		return (1);
	case SDL_BUTTON_RIGHT:
		if (t->vtCur != NULL) {
			VG_Delete(t->vtCur);
			t->vtCur = NULL;
		}
		return (1);
	default:
		return (0);
	}
}

static void
PostDraw(void *p, VG_View *vv)
{
	VG_TextTool *t = p;
	int x, y;

	VG_GetViewCoords(vv, VGTOOL(t)->vCursor, &x,&y);
	AG_DrawCircle(vv, x,y, 3, VG_MapColorRGB(vv->vg->selectionColor));
}

static int
MouseMotion(void *p, VG_Vector vPos, VG_Vector vRel, int b)
{
	VG_TextTool *t = p;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Point *pEx;
	VG_Vector pos;
	float theta, rad;
	
	if (t->vtCur != NULL) {
		pEx = t->vtCur->p1;
		pos = VG_Pos(pEx);
		theta = VG_Atan2(vPos.y - pos.y,
		                 vPos.x - pos.x);
		rad = VG_Hypot(vPos.x - pos.x,
		               vPos.y - pos.y);
		if ((pEx = VG_NearestPoint(vv, vPos, t->vtCur->p2))) {
			VG_Status(vv, _("End baseline at Point%u"),
			    VGNODE(pEx)->handle);
		} else {
			VG_Status(vv,
			    _("End baseline at %.2f,%.2f "
			      "(%.2f|%.2f\xc2\xb0)"),
			    vPos.x, vPos.y, rad, VG_Degrees(theta));
		}
		VG_SetPosition(t->vtCur->p2, vPos);
	} else {
		if ((pEx = VG_NearestPoint(vv, vPos, NULL))) {
			VG_Status(vv, _("Start baseline at Point%u"),
			    VGNODE(pEx)->handle);
		} else {
			VG_Status(vv, _("Start baseline at %.2f,%.2f"), vPos.x,
			    vPos.y);
		}
	}
	return (0);
}

VG_ToolOps vgTextTool = {
	N_("Text"),
	N_("Insert text entity."),
	&vgIconText,
	sizeof(VG_TextTool),
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
