/*	$Csoft: vg_snap.c,v 1.3 2004/04/26 07:03:46 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include <engine/widget/toolbar.h>
#include <engine/widget/button.h>

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

void
vg_ortho_restrict(struct vg *vg, double *x, double *y)
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
vg_ortho_mode(struct vg *vg, enum vg_ortho_mode mode)
{
	vg->ortho_mode = mode;
}

static void
select_mode(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct toolbar *tbar = argv[1].p;
	struct vg *vg = argv[2].p;
	int ortho_mode = argv[3].i;

	toolbar_select_unique(tbar, bu);
	vg_ortho_mode(vg, ortho_mode);
}

struct toolbar *
vg_ortho_toolbar(void *parent, struct vg *vg, enum toolbar_type ttype)
{
	struct toolbar *snbar;
	struct button *bu;

	snbar = toolbar_new(parent, ttype, 1);
	bu = toolbar_add_button(snbar, 0, ICON(SNAP_FREE_ICON), 1, 0,
	    select_mode, "%p,%p,%i", snbar, vg, VG_NO_ORTHO);
	widget_set_int(bu, "state", 1);
	toolbar_add_button(snbar, 0, ICON(SNAP_FREE_ICON), 1, 0,
	    select_mode, "%p,%p,%i", snbar, vg, VG_HORIZ_ORTHO);
	toolbar_add_button(snbar, 0, ICON(SNAP_FREE_ICON), 1, 0,
	    select_mode, "%p,%p,%i", snbar, vg, VG_VERT_ORTHO);

	return (snbar);
}
