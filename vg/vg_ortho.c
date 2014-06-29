/*
 * Copyright (c) 2004-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Orthogonal restriction.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/toolbar.h>
#include <agar/gui/button.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>

void
VG_RestrictOrtho(VG_View *vv, VG_Vector *pos)
{
	/* TODO */
}

static void
SelectMode(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_Toolbar *tbar = AG_PTR(1);
	VG_View *vv = AG_PTR(2);
	int ortho_mode = AG_INT(3);

	AG_ToolbarSelectOnly(tbar, bu);
	VG_ViewSetOrthoMode(vv, ortho_mode);
}

AG_Toolbar *
VG_OrthoRestrictToolbar(void *parent, VG_View *vv, int type)
{
	AG_Toolbar *snbar;

	snbar = AG_ToolbarNew(parent, (enum ag_toolbar_type)type, 1,
	    AG_TOOLBAR_HOMOGENOUS|AG_TOOLBAR_STICKY);

	AG_ToolbarButtonIcon(snbar, vgIconSnapFree.s, 1,
	    SelectMode, "%p,%p,%i", snbar, vv, VG_NO_ORTHO);
	AG_ToolbarButtonIcon(snbar, vgIconSnapFree.s, 0,
	    SelectMode, "%p,%p,%i", snbar, vv, VG_HORIZ_ORTHO);
	AG_ToolbarButtonIcon(snbar, vgIconSnapFree.s, 0,
	    SelectMode, "%p,%p,%i", snbar, vv, VG_VERT_ORTHO);

	return (snbar);
}
