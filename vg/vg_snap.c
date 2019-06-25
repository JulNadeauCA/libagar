/*
 * Copyright (c) 2004-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Grid and snapping methods.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/toolbar.h>
#include <agar/gui/button.h>
#include <agar/gui/menu.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

static void
SnapTo(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Toolbar *tbar = AG_TOOLBAR_PTR(1);
	VG_View *vv = VG_VIEW_PTR(2);
	int snap_mode = AG_INT(3);

	AG_ToolbarSelectOnly(tbar, bu);
	VG_ViewSetSnapMode(vv, snap_mode);
}

static void
SnapToMenu(AG_Event *_Nonnull event)
{
	VG_View *vv = VG_VIEW_PTR(1);
	int snap_mode = AG_INT(2);

	VG_ViewSetSnapMode(vv, snap_mode);
}

AG_Toolbar *
VG_SnapToolbar(void *parent, VG_View *vv, int type)
{
	AG_Toolbar *snbar;

	snbar = AG_ToolbarNew(parent, (enum ag_toolbar_type)type, 1,
	    AG_TOOLBAR_HOMOGENOUS|AG_TOOLBAR_STICKY);
	AG_ToolbarButtonIcon(snbar, vgIconSnapFree.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_FREE_POSITIONING);
	AG_ToolbarButtonIcon(snbar, vgIconSnapGrid.s, 1,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_GRID);
#if 0
	AG_ToolbarButtonIcon(snbar, vgIconSnapEndpt.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_ENDPOINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapEndptDist.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_ENDPOINT_DISTANCE);
	AG_ToolbarButtonIcon(snbar, vgIconSnapClosest.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_CLOSEST_POINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapCenterPt.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_CENTER_POINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapMiddlePt.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_MIDDLE_POINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapIntsectAuto.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_INTERSECTIONS_AUTO);
	AG_ToolbarButtonIcon(snbar, vgIconSnapIntsectManual.s, 0,
	    SnapTo, "%p,%p,%i", snbar, vv, VG_INTERSECTIONS_MANUAL);
#endif
	return (snbar);
}

void
VG_SnapMenu(AG_MenuItem *mi, VG_View *vv)
{
	AG_MenuAction(mi, _("Free positioning"), vgIconSnapFree.s,
	    SnapToMenu, "%p,%i", vv, VG_FREE_POSITIONING);
	AG_MenuAction(mi, _("Grid"), vgIconSnapGrid.s,
	    SnapToMenu, "%p,%i", vv, VG_GRID);
#if 0
	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Endpoint"), vgIconSnapEndpt.s,
	    SnapToMenu, "%p,%i", vv, VG_ENDPOINT);
	AG_MenuAction(mi, _("Distance from endpoint"), vgIconSnapEndptDist.s,
	    SnapToMenu, "%p,%i", vv, VG_ENDPOINT_DISTANCE);
	AG_MenuAction(mi, _("Closest point"), vgIconSnapClosest.s,
	    SnapToMenu, "%p,%i", vv, VG_CLOSEST_POINT);
	AG_MenuAction(mi, _("Center point"), vgIconSnapCenterPt.s,
	    SnapToMenu, "%p,%i", vv, VG_CENTER_POINT);
	AG_MenuAction(mi, _("Middle point"), vgIconSnapMiddlePt.s,
	    SnapToMenu, "%p,%i", vv, VG_MIDDLE_POINT);

	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Intersections (auto)"), vgIconSnapIntsectAuto.s,
	    SnapToMenu, "%p,%i", vv, VG_INTERSECTIONS_AUTO);
	AG_MenuAction(mi, _("Intersections (manual)"),vgIconSnapIntsectManual.s,
	    SnapToMenu, "%p,%i", vv, VG_INTERSECTIONS_MANUAL);
#endif
}
