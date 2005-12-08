/*	$Csoft: vg_ortho.c,v 1.4 2005/09/27 00:25:21 vedge Exp $	*/

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

#include <core/core.h>

#include <gui/toolbar.h>
#include <gui/button.h>

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

void
VG_RestrictOrtho(VG *vg, float *x, float *y)
{
	switch (vg->ortho_mode) {
	case VG_HORIZ_ORTHO:
	case VG_VERT_ORTHO:
		break;
	case VG_NO_ORTHO:
	default:
		break;
	}
}

void
VG_OrthoRestrictMode(VG *vg, enum vg_ortho_mode mode)
{
	vg->ortho_mode = mode;
}

static void
select_mode(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_Toolbar *tbar = AG_PTR(1);
	VG *vg = AG_PTR(2);
	int ortho_mode = AG_INT(3);

	AG_ToolbarSelectUnique(tbar, bu);
	VG_OrthoRestrictMode(vg, ortho_mode);
}

AG_Toolbar *
VG_OrthoRestrictToolbar(void *parent, VG *vg, enum ag_toolbar_type ttype)
{
	AG_Toolbar *snbar;
	AG_Button *bu;

	snbar = AG_ToolbarNew(parent, ttype, 1, AG_TOOLBAR_HOMOGENOUS);
	bu = AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_FREE_ICON), 1, 0,
	    select_mode, "%p,%p,%i", snbar, vg, VG_NO_ORTHO);
	AG_WidgetSetInt(bu, "state", 1);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_FREE_ICON), 1, 0,
	    select_mode, "%p,%p,%i", snbar, vg, VG_HORIZ_ORTHO);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_FREE_ICON), 1, 0,
	    select_mode, "%p,%p,%i", snbar, vg, VG_VERT_ORTHO);

	return (snbar);
}
