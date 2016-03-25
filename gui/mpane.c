/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/mpane.h>
#include <agar/gui/window.h>
#include <agar/gui/cursors.h>
#include <agar/gui/pane.h>

AG_MPane *
AG_MPaneNew(void *parent, enum ag_mpane_layout layout, Uint flags)
{
	AG_MPane *mp;

	mp = Malloc(sizeof(AG_MPane));
	AG_ObjectInit(mp, &agMPaneClass);
	mp->flags |= flags;
	
	if (flags & AG_MPANE_HFILL) { AG_ExpandHoriz(mp); }
	if (flags & AG_MPANE_VFILL) { AG_ExpandVert(mp); }

	AG_MPaneSetLayout(mp, layout);
	AG_ObjectAttach(parent, mp);
	return (mp);
}

static void
Init(void *obj)
{
	AG_MPane *mp = obj;
	int i;

	AG_BoxSetType(&mp->box, AG_BOX_VERT);
	AG_BoxSetPadding(&mp->box, 0);
	AG_BoxSetSpacing(&mp->box, 0);
	for (i = 0; i < 4; i++) {
		mp->panes[i] = AG_BoxNew(NULL, AG_BOX_VERT, AG_BOX_FRAME);
		AG_BoxSetSpacing(mp->panes[i], 0);
		AG_BoxSetPadding(mp->panes[i], 0);
	}
	mp->flags = 0;
	mp->npanes = 0;
}

void
AG_MPaneSetLayout(AG_MPane *mp, enum ag_mpane_layout layout)
{
	AG_Pane *vp = NULL, *hp = NULL, *dp = NULL;

	AG_ObjectLock(mp);

	AG_ObjectFreeChildren(OBJECT(mp));

	switch (layout) {
	case AG_MPANE1:
	default:
		AG_ObjectAttach(mp, mp->panes[0]);
		WIDGET(mp->panes[0])->flags |= AG_WIDGET_EXPAND;
		mp->npanes = 1;
		break;
	case AG_MPANE2H:
		vp = AG_PaneNew(mp, AG_PANE_VERT, AG_PANE_EXPAND);
		AG_PaneAttachBoxes(vp, mp->panes[0], mp->panes[1]);
		mp->npanes = 2;
		break;
	case AG_MPANE2V:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, AG_PANE_EXPAND);
		AG_PaneAttachBoxes(hp, mp->panes[0], mp->panes[1]);
		mp->npanes = 2;
		break;
	case AG_MPANE2L1R:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, AG_PANE_EXPAND);
		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT, AG_PANE_EXPAND);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(vp, 1, mp->panes[1]);
		AG_PaneAttachBox(hp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE1L2R:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, AG_PANE_EXPAND);
		vp = AG_PaneNew(hp->div[1], AG_PANE_VERT, AG_PANE_EXPAND);
		AG_PaneAttachBox(hp, 0, mp->panes[0]);
		AG_PaneAttachBox(vp, 0, mp->panes[1]);
		AG_PaneAttachBox(vp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE2T1B:
		vp = AG_PaneNew(mp, AG_PANE_VERT, AG_PANE_EXPAND);
		hp = AG_PaneNew(vp->div[0], AG_PANE_HORIZ, AG_PANE_EXPAND);
		AG_PaneAttachBox(hp, 0, mp->panes[0]);
		AG_PaneAttachBox(hp, 1, mp->panes[1]);
		AG_PaneAttachBox(vp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE1T2B:
		vp = AG_PaneNew(mp, AG_PANE_VERT, AG_PANE_EXPAND);
		hp = AG_PaneNew(vp->div[1], AG_PANE_HORIZ, AG_PANE_EXPAND);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(hp, 0, mp->panes[1]);
		AG_PaneAttachBox(hp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE3L1R:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, AG_PANE_EXPAND);
		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT, AG_PANE_EXPAND);
		dp = AG_PaneNew(vp->div[1], AG_PANE_VERT, AG_PANE_EXPAND);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(dp, 0, mp->panes[1]);
		AG_PaneAttachBox(dp, 1, mp->panes[2]);
		AG_PaneAttachBox(hp, 1, mp->panes[3]);
		mp->npanes = 4;
		break;
	case AG_MPANE4:
		hp = AG_PaneNew(mp, AG_PANE_VERT, AG_PANE_EXPAND);
		vp = AG_PaneNew(hp->div[0], AG_PANE_HORIZ, AG_PANE_EXPAND);
		dp = AG_PaneNew(hp->div[1], AG_PANE_HORIZ, AG_PANE_EXPAND);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(vp, 1, mp->panes[1]);
		AG_PaneAttachBox(dp, 0, mp->panes[2]);
		AG_PaneAttachBox(dp, 1, mp->panes[3]);
		mp->npanes = 4;
		break;
	}
	if (mp->flags & AG_MPANE_FORCE_DIV) {
		if (hp) {
			AG_PaneMoveDividerPct(hp, 50);
			AG_PaneResizeAction(hp, AG_PANE_DIVIDE_EVEN);
		}
		if (vp) {
			AG_PaneMoveDividerPct(vp, 50);
			AG_PaneResizeAction(vp, AG_PANE_DIVIDE_EVEN);
		}
		if (dp) {
			AG_PaneMoveDividerPct(dp, 50);
			AG_PaneResizeAction(dp, AG_PANE_DIVIDE_EVEN);
		}
	}
	mp->layout = layout;
	
	AG_ObjectUnlock(mp);
}

static void
Draw(void *obj)
{
	AG_MPane *mp = obj;
	int i;

	for (i = 0; i < mp->npanes; i++)
		AG_WidgetDraw(mp->panes[i]);
}

AG_WidgetClass agMPaneClass = {
	{
		"Agar(Widget:Box:MPane)",
		sizeof(AG_MPane),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	NULL,			/* size_request */
	NULL			/* size_allocate */
};
