/*	$Csoft$	*/

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

/* Specify a single vertex. */
void
vg_vertex(struct vg *vg, double x, double y)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

#ifdef DEBUG
	if (x > vg->w || y > vg->h)
		fatal("vertex out of bounds");
#endif

	if (vge->vertices == NULL) {
		vge->vertices = Malloc(2*sizeof(double), M_VG);
	} else {
		vge->vertices = Realloc(vge->vertices, (vge->nvertices+2) *
		    sizeof(double), M_VG);
	}
	vge->vertices[vge->nvertices++] = x;
	vge->vertices[vge->nvertices++] = y;
}

/* Specify an array of vertices. */
void
vg_vertices(struct vg *vg, double *vertices, int nvertices)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);
	int i;
	
	for (i = 0; i < nvertices-1; i += 2)
		vg_vertex(vg, vertices[i], vertices[i+1]);
}

