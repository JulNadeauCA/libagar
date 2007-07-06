/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Three-dimensional voxel object.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sg.h"
#include "sg_gui.h"

SG_Voxel *
SG_VoxelNew(void *pNode, const char *name)
{
	SG_Voxel *vol;

	vol = AG_Malloc(sizeof(SG_Voxel), M_SG);
	SG_VoxelInit(vol, name);
	SG_NodeAttach(pNode, vol);
	return (vol);
}

void
SG_VoxelInit(void *p, const char *name)
{
	SG_Voxel *vol = p;

	SG_NodeInit(vol, name, &sgVoxelOps, 0);
	vol->map = NULL;
	vol->w = 0;
	vol->h = 0;
	vol->d = 0;
}

void
SG_VoxelReinit(void *p)
{
	SG_Voxel *vol = p;
	int x, y, z;
	
	for (x = 0; x < vol->w; x++) {
		for (y = 0; y < vol->h; y++) {
			Free(vol->map[y], M_SG);
		}
		Free(vol->map[x], M_SG);
	}
	Free(vol->map, M_SG);
	vol->map = NULL;
	vol->w = 0;
	vol->h = 0;
	vol->d = 0;
}

void
SG_VoxelDestroy(void *p)
{
	SG_VoxelReinit(p);
}

void
SG_VoxelAlloc3(SG_Voxel *vol, Uint w, Uint h, Uint d)
{
	Uint x, y, z;

	if (vol->map != NULL) {
		SG_VoxelReinit(vol);
	}
	vol->w = w;
	vol->h = h;
	vol->d = d;
	vol->map = Malloc(w*sizeof(SG_Real *), M_SG);
	for (x = 0; x < w; x++) {
		vol->map[x] = Malloc(h*sizeof(SG_Real *), M_SG);
		for (y = 0; y < h; y++) {
			vol->map[x][y] = Malloc(d*sizeof(SG_Real), M_SG);
			for (z = 0; z < d; z++)
				vol->map[x][y][z] = 0.0;
		}
	}
	return;
fail_mem:
	fatal("Out of memory for %ux%ux%u voxel", w, h, d);
}

void
SG_VoxelSet3(SG_Voxel *vol, int x, int y, int z, SG_Real v)
{
	if (x < 0 || y < 0 || z < 0 ||
	    x >= (int)vol->w || y >= (int)vol->h || z >= (int)vol->d) {
		fprintf(stderr, "VoxelSet3: Illegal cell %d,%d,%d\n",
		    x, y, z);
	} else {
		vol->map[x][y][z] = v;
	}
}

void
SG_VoxelReset(SG_Voxel *vol, SG_Real v)
{
	int x, y, z;

	for (x = 0; x < vol->w; x++) {
		for (y = 0; y < vol->h; y++) {
			for (z = 0; z < vol->d; z++)
				vol->map[x][y][z] = v;
		}
	}
}

void
SG_VoxelDraw(void *pNode, SG_View *sgv)
{
	static const SG_Vector p[] = {
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 1.0, 0.0, 1.0 },
		{ 0.0, 0.0, 1.0 },
		{ 0.0, 1.0, 0.0 },
		{ 1.0, 1.0, 0.0 },
		{ 1.0, 1.0, 1.0 },
		{ 0.0, 1.0, 1.0 },
	};
	SG_Voxel *vol = pNode;
	Uint x, y, z;
	
	glPushMatrix();
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glScalef(vol->w, vol->h, vol->d);
	SG_Begin(SG_LINE_STRIP);
	SG_Color3f(1.0, 1.0, 1.0);
	SG_Vertex3v(&p[0]); SG_Vertex3v(&p[1]);
	SG_Vertex3v(&p[2]); SG_Vertex3v(&p[3]);
	SG_Vertex3v(&p[0]); SG_Vertex3v(&p[4]);
	SG_Vertex3v(&p[5]); SG_Vertex3v(&p[6]);
	SG_Vertex3v(&p[7]); SG_Vertex3v(&p[4]);
	SG_End();
	SG_Begin(SG_LINES);
	SG_Color3f(1.0, 1.0, 1.0);
	SG_Vertex3v(&p[1]); SG_Vertex3v(&p[5]);
	SG_Vertex3v(&p[2]); SG_Vertex3v(&p[6]);
	SG_Vertex3v(&p[3]); SG_Vertex3v(&p[7]);
	SG_End();
	glPopAttrib();
	glPopMatrix();

	for (x = 0; x < vol->w; x++) {
		for (y = 0; y < vol->h; y++) {
			for (z = 0; z < vol->d; z++) {
				SG_Color color;
				SG_Real c;
			
				c = vol->map[x][y][z];

				if (c == 0.0) {
					continue;
				}
				glPushMatrix();
				glTranslatef(x, y, z);

				color = SG_ColorRGB(0.0,c,0.0);
				SG_Begin(SG_QUADS);
				SG_MaterialColor(GL_FRONT, GL_AMBIENT, &color);
				SG_MaterialColor(GL_FRONT, GL_DIFFUSE, &color);
				SG_MaterialColor(GL_FRONT, GL_SPECULAR, &color);
				SG_Normal3(0.0, -1.0, 0.0);	/* TOP */
				SG_Vertex3v(&p[7]);
				SG_Vertex3v(&p[6]);
				SG_Vertex3v(&p[5]);
				SG_Vertex3v(&p[4]);
				SG_Normal3(0.0, -1.0, 0.0);	/* BOTTOM */
				SG_Vertex3v(&p[1]); 
				SG_Vertex3v(&p[2]);
				SG_Vertex3v(&p[3]);
				SG_Vertex3v(&p[0]);
				SG_Normal3(0.0, 0.0, -1.0);	/* FRONT */
				SG_Vertex3v(&p[4]);
				SG_Vertex3v(&p[5]);
				SG_Vertex3v(&p[1]);
				SG_Vertex3v(&p[0]);
				SG_Normal3(0.0, 0.0, -1.0);	/* BACK */
				SG_Vertex3v(&p[3]);
				SG_Vertex3v(&p[2]);
				SG_Vertex3v(&p[6]);
				SG_Vertex3v(&p[7]);
				SG_Normal3(-1.0, 0.0, 0.0);	/* LEFT */
				SG_Vertex3v(&p[3]);
				SG_Vertex3v(&p[7]);
				SG_Vertex3v(&p[4]);
				SG_Vertex3v(&p[0]);
				SG_Normal3(-1.0, 0.0, 0.0);	/* RIGHT */
				SG_Vertex3v(&p[5]);
				SG_Vertex3v(&p[6]);
				SG_Vertex3v(&p[2]);
				SG_Vertex3v(&p[1]);
				SG_End();
				glPopMatrix();
			}
		}
	}
}

SG_NodeOps sgVoxelOps = {
	"Voxel",
	sizeof(SG_Voxel),
	0,
	SG_VoxelInit,
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
	NULL,		/* menuInstance */
	NULL,		/* menuClass */
	SG_VoxelDraw
};

#endif /* HAVE_OPENGL */
