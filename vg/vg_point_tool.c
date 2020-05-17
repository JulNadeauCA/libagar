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
 * Point tool.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

static int
MouseButtonDown(void *_Nonnull obj, VG_Vector v, int b)
{
	VG_Tool *t = obj;
	VG_View *vv = VGTOOL(t)->vgv;
	VG *vg = vv->vg;

	switch (b) {
	case AG_MOUSE_LEFT:
		VG_Status(vv, _("New point at %.2f,%.2f"), v.x, v.y);
		VG_PointNew(vg->root, v);
		return (1);
	default:
		return (0);
	}
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

VG_ToolOps vgPointTool = {
	N_("Point"),
	N_("Insert point in drawing."),
	&vgIconPoints,
	sizeof(VG_Tool),
	0,
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* predraw */
	PostDraw,
	NULL,			/* selected */
	NULL,			/* deselected */
	NULL,			/* mousemotion */
	MouseButtonDown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
