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
 * Text tool.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/textbox.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

typedef struct vg_text_tool {
	VG_Tool _inherit;			/* VG_Tool(3) -> VG_TextTool */
	VG_Text *_Nullable vtIns;		/* Text being edited */
	char text[VG_TEXT_MAX];			/* Initial input text */
	Uint32 _pad;
} VG_TextTool;

static void
Init(void *_Nonnull obj)
{
	VG_TextTool *t = obj;

	t->vtIns = NULL;
	Strlcpy(t->text, "<text>", sizeof(t->text));
}

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector vPos, int button)
{
	VG_TextTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;
	VG_Point *p1, *p2;

	switch (button) {
	case AG_MOUSE_LEFT:
		if (t->vtIns == NULL) {
			if (!(p1 = VG_NearestPoint(vv, vPos, NULL))) {
				p1 = VG_PointNew(vg->root, vPos);
			}
			p2 = VG_PointNew(vg->root, vPos);
			t->vtIns = VG_TextNew(vg->root, p1, p2);
			VG_TextString(t->vtIns, t->text);
			if (VGTOOL(t)->p != NULL)
				VG_TextSubstObject(t->vtIns, VGTOOL(t)->p);
		} else {
			if ((p2 = VG_NearestPoint(vv, vPos,
			    t->vtIns->p2))) {
				VG_DelRef(t->vtIns, t->vtIns->p2);
				VG_Delete(t->vtIns->p2);
				t->vtIns->p2 = p2;
				VG_AddRef(t->vtIns, p2);
			} else {
				VG_SetPosition(t->vtIns->p2, vPos);
			}
			t->vtIns = NULL;
		}
		return (1);
	case AG_MOUSE_RIGHT:
		if (t->vtIns != NULL) {
			VG_Delete(t->vtIns);
			t->vtIns = NULL;
		}
		return (1);
	default:
		return (0);
	}
}

static void
PostDraw(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_TextTool *t = obj;
	AG_Color c;
	int x, y;

	VG_GetViewCoords(vv, VGTOOL(t)->vCursor, &x,&y);
	c = VG_MapColorRGB(vv->vg->selectionColor);
	AG_DrawCircle(vv, x,y, 3, &c);
}

static int
MouseMotion(void *_Nonnull obj, VG_Vector vPos, VG_Vector vRel, int b)
{
	VG_TextTool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG_Point *pEx;
	VG_Vector pos;
	float theta, rad;
	
	if (t->vtIns != NULL) {
		pEx = t->vtIns->p1;
		pos = VG_Pos(pEx);
		theta = VG_Atan2(vPos.y - pos.y,
		                 vPos.x - pos.x);
		rad = VG_Hypot(vPos.x - pos.x,
		               vPos.y - pos.y);
		if ((pEx = VG_NearestPoint(vv, vPos, t->vtIns->p2))) {
			VG_Status(vv, _("End baseline at Point%u"),
			    (Uint)VGNODE(pEx)->handle);
		} else {
			VG_Status(vv,
			    _("End baseline at %.2f,%.2f "
			      "(%.2f|%.2f\xc2\xb0)"),
			    vPos.x, vPos.y, rad, VG_Degrees(theta));
		}
		VG_SetPosition(t->vtIns->p2, vPos);
	} else {
		if ((pEx = VG_NearestPoint(vv, vPos, NULL))) {
			VG_Status(vv, _("Start baseline at Point%u"),
			    (Uint)VGNODE(pEx)->handle);
		} else {
			VG_Status(vv, _("Start baseline at %.2f,%.2f"), vPos.x,
			    vPos.y);
		}
	}
	return (0);
}

static void *_Nonnull
Edit(void *_Nonnull obj, VG_View *_Nonnull vv)
{
	VG_TextTool *t = obj;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	AG_Textbox *tb;

	AG_LabelNew(box, 0, _("Text: "));
	tb = AG_TextboxNewS(box, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_HFILL, NULL);
#ifdef AG_UNICODE
	AG_TextboxBindUTF8(tb, t->text, sizeof(t->text));
#else
	AG_TextboxBindASCII(tb, t->text, sizeof(t->text));
#endif
	return (box);
}

VG_ToolOps vgTextTool = {
	N_("Text"),
	N_("Insert text entity."),
	&vgIconText,
	sizeof(VG_TextTool),
	0,
	Init,
	NULL,			/* destroy */
	Edit,
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
