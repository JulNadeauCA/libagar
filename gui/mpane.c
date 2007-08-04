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

#include <core/core.h>
#include <core/view.h>

#include "mpane.h"

#include "window.h"
#include "cursors.h"
#include "pane.h"

AG_MPane *
AG_MPaneNew(void *parent, enum ag_mpane_layout lay, Uint flags)
{
	AG_MPane *mp;

	mp = Malloc(sizeof(AG_MPane), M_OBJECT);
	AG_MPaneInit(mp, lay, flags);
	AG_ObjectAttach(parent, mp);
	return (mp);
}

void
AG_MPaneInit(AG_MPane *mp, enum ag_mpane_layout lay, Uint flags)
{
	Uint boxflags = 0;
	Uint paneflags = flags & AG_MPANE_FRAMES ? AG_BOX_FRAME : 0;
	int i;

	if (flags & AG_MPANE_HFILL) boxflags |= AG_BOX_HFILL;
	if (flags & AG_MPANE_VFILL) boxflags |= AG_BOX_VFILL;

	AG_BoxInit(&mp->box, AG_BOX_VERT, boxflags);
	AG_BoxSetPadding(&mp->box, 0);
	AG_BoxSetSpacing(&mp->box, 0);
	for (i = 0; i < 4; i++) {
		mp->panes[i] = AG_BoxNew(NULL, AG_BOX_VERT, paneflags);
		AG_BoxSetSpacing(mp->panes[i], 0);
		AG_BoxSetPadding(mp->panes[i], 0);
	}
	mp->flags = flags;
	mp->npanes = 0;
	AG_MPaneSetLayout(mp, lay);
}

void
AG_MPaneSetLayout(AG_MPane *mp, enum ag_mpane_layout layout)
{
	AG_Pane *vp, *hp, *dp;
	Uint pflags = AG_PANE_EXPAND|AG_PANE_DIV;

	if (mp->flags & AG_MPANE_FORCE_DIV)
		pflags |= AG_PANE_FORCE_DIV;

	AG_ObjectFreeChildren(AGOBJECT(mp));

	switch (layout) {
	case AG_MPANE1:
		AG_ObjectAttach(mp, mp->panes[0]);
		AGWIDGET(mp->panes[0])->flags |= AG_WIDGET_EXPAND;
		mp->npanes = 1;
		break;
	case AG_MPANE2H:
		vp = AG_PaneNew(mp, AG_PANE_VERT, pflags);
		AG_PaneAttachBoxes(vp, mp->panes[0], mp->panes[1]);
		mp->npanes = 2;
		break;
	case AG_MPANE2V:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, pflags);
		AG_PaneAttachBoxes(hp, mp->panes[0], mp->panes[1]);
		mp->npanes = 2;
		break;
	case AG_MPANE2L1R:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, pflags);
		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT, pflags);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(vp, 1, mp->panes[1]);
		AG_PaneAttachBox(hp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE1L2R:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, pflags);
		vp = AG_PaneNew(hp->div[1], AG_PANE_VERT, pflags);
		AG_PaneAttachBox(hp, 0, mp->panes[0]);
		AG_PaneAttachBox(vp, 0, mp->panes[1]);
		AG_PaneAttachBox(vp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE2T1B:
		vp = AG_PaneNew(mp, AG_PANE_VERT, pflags);
		hp = AG_PaneNew(vp->div[0], AG_PANE_HORIZ, pflags);
		AG_PaneAttachBox(hp, 0, mp->panes[0]);
		AG_PaneAttachBox(hp, 1, mp->panes[1]);
		AG_PaneAttachBox(vp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE1T2B:
		vp = AG_PaneNew(mp, AG_PANE_VERT, pflags);
		hp = AG_PaneNew(vp->div[1], AG_PANE_HORIZ, pflags);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(hp, 0, mp->panes[1]);
		AG_PaneAttachBox(hp, 1, mp->panes[2]);
		mp->npanes = 3;
		break;
	case AG_MPANE3L1R:
		hp = AG_PaneNew(mp, AG_PANE_HORIZ, pflags);
		vp = AG_PaneNew(hp->div[0], AG_PANE_VERT, pflags);
		dp = AG_PaneNew(vp->div[1], AG_PANE_VERT, pflags);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(dp, 0, mp->panes[1]);
		AG_PaneAttachBox(dp, 1, mp->panes[2]);
		AG_PaneAttachBox(hp, 1, mp->panes[3]);
		mp->npanes = 4;
		break;
	case AG_MPANE4:
		hp = AG_PaneNew(mp, AG_PANE_VERT, pflags);
		vp = AG_PaneNew(hp->div[0], AG_PANE_HORIZ, pflags);
		dp = AG_PaneNew(hp->div[1], AG_PANE_HORIZ, pflags);
		AG_PaneAttachBox(vp, 0, mp->panes[0]);
		AG_PaneAttachBox(vp, 1, mp->panes[1]);
		AG_PaneAttachBox(dp, 0, mp->panes[2]);
		AG_PaneAttachBox(dp, 1, mp->panes[3]);
		mp->npanes = 4;
		break;
	default:
		break;
	}
	mp->layout = layout;
}
