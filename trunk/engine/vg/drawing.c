/*	$Csoft: drawing.c,v 1.13 2005/09/27 00:25:20 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/map/map.h>
#include <engine/map/mapedit.h>
#include <engine/map/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mfspinbutton.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/combo.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "drawing.h"

const AG_Version agDrawingVer = {
	"agar drawing",
	0, 0
};

const AG_ObjectOps drawing_ops = {
	AG_DrawingInit,
	AG_DrawingReinit,
	AG_DrawingDestroy,
	AG_DrawingLoad,
	AG_DrawingSave,
	AG_DrawingEdit
};

void
AG_DrawingInit(void *p, const char *name)
{
	AG_Drawing *dwg = p;

	AG_ObjectInit(dwg, "drawing", name, &drawing_ops);
	dwg->vg = VG_New(dwg, VG_RLEACCEL|VG_VISORIGIN|VG_VISGRID);
	dwg->vg->fill_color = SDL_MapRGB(dwg->vg->fmt, 0, 0, 0);
	VG_Scale(dwg->vg, 8.5, 11, 1);
}

void
AG_DrawingReinit(void *p)
{
	AG_Drawing *dwg = p;

	VG_Reinit(dwg->vg);
}

void
AG_DrawingDestroy(void *p)
{
	AG_Drawing *dwg= p;

	VG_Destroy(dwg->vg);
}

int
AG_DrawingLoad(void *p, AG_Netbuf *buf)
{
	AG_Drawing *dwg = p;

	if (AG_ReadVersion(buf, &agDrawingVer, NULL) != 0)
		return (-1);

	return (VG_Load(dwg->vg, buf));
}

int
AG_DrawingSave(void *p, AG_Netbuf *buf)
{
	AG_Drawing *dwg = p;

	AG_WriteVersion(buf, &agDrawingVer);
	VG_Save(dwg->vg, buf);
	return (0);
}

static void
drawing_settings(int argc, union evarg *argv)
{
	extern void VG_ChangedEv(int, union evarg *);
	extern void VG_GeoChangedEv(int, union evarg *);
	AG_Window *pwin = argv[1].p;
	AG_Drawing *dwg = argv[2].p;
	VG *vg = dwg->vg;
	AG_Window *win;
	AG_MFSpinbutton *mfsu;
	AG_FSpinbutton *fsu;
	
	if ((win = AG_WindowNew(AG_WINDOW_HIDE|AG_WINDOW_NO_VRESIZE, "%s-settings",
	    AGOBJECT(dwg)->name)) == NULL)
		return;

	AG_WindowSetCaption(win, _("Parameters for \"%s\""),
	    AGOBJECT(dwg)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	mfsu = AG_MFSpinbuttonNew(win, NULL, "x", _("Geometry: "));
	AG_WidgetBind(mfsu, "xvalue", AG_WIDGET_DOUBLE, &vg->w);
	AG_WidgetBind(mfsu, "yvalue", AG_WIDGET_DOUBLE, &vg->h);
	AG_MFSpinbuttonSetMin(mfsu, 1.0);
	AG_MFSpinbuttonSetIncrement(mfsu, 0.1);
	AG_SetEvent(mfsu, "mfspinbutton-changed", VG_GeoChangedEv, "%p", vg);

	fsu = AG_FSpinbuttonNew(win, NULL, _("Grid interval: "));
	AG_WidgetBind(fsu, "value", AG_WIDGET_DOUBLE, &vg->grid_gap);
	AG_FSpinbuttonSetMin(fsu, 0.0625);
	AG_FSpinbuttonSetIncrement(fsu, 0.0625);
	AG_SetEvent(fsu, "fspinbutton-changed", VG_ChangedEv, "%p", vg);
	
	fsu = AG_FSpinbuttonNew(win, NULL, _("Scaling factor: "));
	AG_WidgetBind(fsu, "value", AG_WIDGET_DOUBLE, &vg->scale);
	AG_FSpinbuttonSetMin(fsu, 0.1);
	AG_FSpinbuttonSetIncrement(fsu, 0.1);
	AG_SetEvent(fsu, "fspinbutton-changed", VG_GeoChangedEv, "%p", vg);

#if 0
	AG_LabelNew(win, AG_LABEL_STATIC, _("Background color: "));
	pal = palette_new(win, AG_PALETTE_RGB);
	AG_WidgetBind(pal, "pixel", AG_WIDGET_UINT32, &vg->fill_color);
	AG_WidgetBind(pal, "pixel-format", AG_WIDGET_POINTER, &vg->fmt);
	AG_SetEvent(pal, "palette-changed", VG_ChangedEv, "%p", vg);
	
	AG_LabelNew(win, AG_LABEL_STATIC, _("Grid color: "));
	pal = palette_new(win, AG_PALETTE_RGB);
	AG_WidgetBind(pal, "pixel", AG_WIDGET_UINT32, &vg->grid_color);
	AG_WidgetBind(pal, "pixel-format", AG_WIDGET_POINTER, &vg->fmt);
	AG_SetEvent(pal, "palette-changed", VG_ChangedEv, "%p", vg);
#endif

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
rasterize_drawing(AG_Mapview *mv, void *p)
{
	VG *vg = p;

	if (vg->redraw) {
		vg->redraw = 0;
	}
	vg->redraw++;
	VG_Rasterize(vg);
}

void *
AG_DrawingEdit(void *obj)
{
	extern const AG_MaptoolOps vgScaleTool;
	extern const AG_MaptoolOps vgOriginTool;
	extern const AG_MaptoolOps vg_point_tool;
	extern const AG_MaptoolOps vg_line_tool;
	extern const AG_MaptoolOps vg_circle_tool;
	extern const AG_MaptoolOps vg_ellipse_tool;
	extern const AG_MaptoolOps vgTextTool;
	AG_Drawing *dwg = obj;
	VG *vg = dwg->vg;
	AG_Window *win;
	AG_Box *bo;
	AG_Toolbar *tbar;
	AG_Statusbar *statbar;

	win = AG_WindowNew(AG_WINDOW_DETACH, NULL);
	AG_WindowSetCaption(win, _("Drawing: %s"), AGOBJECT(dwg)->name);

	tbar = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 1, 0);
	AG_ToolbarAddButton(tbar, 0, AGICON(SETTINGS_ICON), 0, 0,
	    drawing_settings, "%p, %p", win, dwg);

	statbar = Malloc(sizeof(AG_Statusbar), M_OBJECT);
	AG_StatusbarInit(statbar);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HFILL);
	AG_BoxSetSpacing(bo, 0);
	AG_BoxSetPadding(bo, 0);
	{
		AG_Toolbar *snbar;
		AG_Button *bu;

		snbar = VG_SnapToolbar(bo, vg, AG_TOOLBAR_VERT);

		bo = AG_BoxNew(bo, AG_BOX_VERT, AG_BOX_WFILL|AG_BOX_HFILL);
		AG_BoxSetSpacing(bo, 0);
		AG_BoxSetPadding(bo, 0);
		{
			AG_Combo *laysel;
			AG_Mapview *mv;
		
			laysel = VG_NewLayerSelector(bo, vg);
			mv = AG_MapviewNew(bo, vg->map,
			    AG_MAPVIEW_EDIT|AG_MAPVIEW_NO_BMPSCALE|
			    AG_MAPVIEW_NO_BG,
			    tbar, statbar);
			AG_MapviewPrescale(mv, 10, 8);
			AG_MapviewRegDrawCb(mv, rasterize_drawing, vg);

			AG_MapviewRegTool(mv, &vgScaleTool, vg);
			AG_MapviewRegTool(mv, &vgOriginTool, vg);
			AG_MapviewRegTool(mv, &vg_point_tool, vg);
			AG_MapviewRegTool(mv, &vg_line_tool, vg);
			AG_MapviewRegTool(mv, &vg_circle_tool, vg);
			AG_MapviewRegTool(mv, &vg_ellipse_tool, vg);
			AG_MapviewRegTool(mv, &vgTextTool, vg);
		}
	}

	AG_ObjectAttach(win, statbar);
	return (win);
}
