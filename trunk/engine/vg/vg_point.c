/*	$Csoft: vg_points.c,v 1.1 2004/03/17 06:04:59 vedge Exp $	*/

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

#include "vg.h"
#include "vg_primitive.h"

void
vg_point_radius(struct vg *vg, double radius)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_point.radius = radius;
}

void
vg_point_diameter(struct vg *vg, double diameter)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_point.radius = diameter/2;
}

void
vg_draw_points(struct vg *vg, struct vg_element *vge)
{
	int radpx = VG_PX(vg, vge->vg_point.radius);
	int i;

	for (i = 0; i < vge->nvertices-1; i += 2) {
		int x = VG_X(vg, vge->vertices[i]);
		int y = VG_Y(vg, vge->vertices[i+1]);
		int j;

		vg_putpixel(vg, x, y, vge->color);
	}
}

