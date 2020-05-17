/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

/* Create a new Voxel node. */
SG_Voxel *
SG_VoxelNew(void *parent, const char *name)
{
	SG_Voxel *vol;

	vol = Malloc(sizeof(SG_Voxel));
	AG_ObjectInit(vol, &sgVoxelClass);
	if (name) {
		AG_ObjectSetNameS(vol, name);
	} else {
		OBJECT(vol)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, vol);
	return (vol);
}

static void
Init(void *_Nonnull obj)
{
	SG_Voxel *vol = obj;

	vol->map = NULL;
	vol->w = 0;
	vol->h = 0;
	vol->d = 0;
}

static void
Reset(void *_Nonnull obj)
{
	SG_Voxel *vol = obj;
	int x, y;
	
	for (x = 0; x < vol->w; x++) {
		for (y = 0; y < vol->h; y++) {
			Free(vol->map[y]);
		}
		Free(vol->map[x]);
	}
	Free(vol->map);
	vol->map = NULL;
	vol->w = 0;
	vol->h = 0;
	vol->d = 0;
}

/* Set voxel dimensions in # of cells. */
void
SG_VoxelAlloc3(SG_Voxel *vol, Uint w, Uint h, Uint d)
{
	Uint x, y, z;

	AG_ObjectLock(vol);

	if (vol->map != NULL) {
		Reset(vol);
	}
	vol->w = w;
	vol->h = h;
	vol->d = d;
	vol->map = Malloc(w*sizeof(M_Real *));
	for (x = 0; x < w; x++) {
		vol->map[x] = Malloc(h*sizeof(M_Real *));
		for (y = 0; y < h; y++) {
			vol->map[x][y] = Malloc(d*sizeof(M_Real));
			for (z = 0; z < d; z++)
				vol->map[x][y][z] = 0.0;
		}
	}
	
	AG_ObjectUnlock(vol);
}

/* Set the value of the cell at x,y,z to v. */
int
SG_VoxelSet3(SG_Voxel *vol, int x, int y, int z, M_Real v)
{
	AG_ObjectLock(vol);

	if (x < 0 || y < 0 || z < 0 ||
	    x >= (int)vol->w || y >= (int)vol->h || z >= (int)vol->d) {
		AG_SetError("No such cell: %d,%d,%d", x, y, z);
		goto fail;
	}
	vol->map[x][y][z] = v;

	AG_ObjectUnlock(vol);
	return (0);
fail:
	AG_ObjectUnlock(vol);
	return (-1);
}

/* Set the value of all cells to v. */
void
SG_VoxelReset(SG_Voxel *vol, M_Real v)
{
	int x, y, z;

	AG_ObjectLock(vol);
	for (x = 0; x < vol->w; x++) {
		for (y = 0; y < vol->h; y++) {
			for (z = 0; z < vol->d; z++)
				vol->map[x][y][z] = v;
		}
	}
	AG_ObjectUnlock(vol);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull sgv)
{
	SG_Voxel *vol = obj;
	M_Color white = M_ColorWhite();
	M_Vector3 p[8];
	int x, y, z;
		
	p[0] = M_VECTOR3(0.0, 0.0, 0.0);
	p[1] = M_VECTOR3(1.0, 0.0, 0.0);
	p[2] = M_VECTOR3(1.0, 0.0, 1.0);
	p[3] = M_VECTOR3(0.0, 0.0, 1.0);
	p[4] = M_VECTOR3(0.0, 1.0, 0.0);
	p[5] = M_VECTOR3(1.0, 1.0, 0.0);
	p[6] = M_VECTOR3(1.0, 1.0, 1.0);
	p[7] = M_VECTOR3(0.0, 1.0, 1.0);
	
	GL_PushMatrix();
	GL_PushAttrib(GL_LIGHTING_BIT);
	GL_Disable(GL_LIGHTING);
	GL_Scale3(vol->w, vol->h, vol->d);

	GL_Begin(GL_LINE_STRIP);
	{
		GL_Color3v(&white);
		GL_Vertex3v(&p[0]); GL_Vertex3v(&p[1]);
		GL_Vertex3v(&p[2]); GL_Vertex3v(&p[3]);
		GL_Vertex3v(&p[0]); GL_Vertex3v(&p[4]);
		GL_Vertex3v(&p[5]); GL_Vertex3v(&p[6]);
		GL_Vertex3v(&p[7]); GL_Vertex3v(&p[4]);
	}
	GL_End();
	GL_Begin(GL_LINES);
	{
		GL_Color3v(&white);
		GL_Vertex3v(&p[1]); GL_Vertex3v(&p[5]);
		GL_Vertex3v(&p[2]); GL_Vertex3v(&p[6]);
		GL_Vertex3v(&p[3]); GL_Vertex3v(&p[7]);
	}
	GL_End();

	GL_PopAttrib();
	GL_PopMatrix();

	for (x = 0; x < vol->w; x++) {
		for (y = 0; y < vol->h; y++) {
			for (z = 0; z < vol->d; z++) {
				M_Color C;
				M_Real cell;
			
				cell = vol->map[x][y][z];

				if (cell == 0.0) {
					continue;
				}
				GL_PushMatrix();
				GL_Translate(M_VECTOR3(x,y,z));

				C = M_ColorRGB(0.0, cell, 0.0);
				GL_Begin(GL_QUADS);
				GL_MaterialColorv(GL_FRONT, GL_AMBIENT, &C);
				GL_MaterialColorv(GL_FRONT, GL_DIFFUSE, &C);
				GL_MaterialColorv(GL_FRONT, GL_SPECULAR, &C);
				GL_Normal3(0.0, -1.0, 0.0);	/* TOP */
				GL_Vertex3v(&p[7]);
				GL_Vertex3v(&p[6]);
				GL_Vertex3v(&p[5]);
				GL_Vertex3v(&p[4]);
				GL_Normal3(0.0, -1.0, 0.0);	/* BOTTOM */
				GL_Vertex3v(&p[1]); 
				GL_Vertex3v(&p[2]);
				GL_Vertex3v(&p[3]);
				GL_Vertex3v(&p[0]);
				GL_Normal3(0.0, 0.0, -1.0);	/* FRONT */
				GL_Vertex3v(&p[4]);
				GL_Vertex3v(&p[5]);
				GL_Vertex3v(&p[1]);
				GL_Vertex3v(&p[0]);
				GL_Normal3(0.0, 0.0, -1.0);	/* BACK */
				GL_Vertex3v(&p[3]);
				GL_Vertex3v(&p[2]);
				GL_Vertex3v(&p[6]);
				GL_Vertex3v(&p[7]);
				GL_Normal3(-1.0, 0.0, 0.0);	/* LEFT */
				GL_Vertex3v(&p[3]);
				GL_Vertex3v(&p[7]);
				GL_Vertex3v(&p[4]);
				GL_Vertex3v(&p[0]);
				GL_Normal3(-1.0, 0.0, 0.0);	/* RIGHT */
				GL_Vertex3v(&p[5]);
				GL_Vertex3v(&p[6]);
				GL_Vertex3v(&p[2]);
				GL_Vertex3v(&p[1]);
				GL_End();
				GL_PopMatrix();
			}
		}
	}
}

SG_NodeClass sgVoxelClass = {
	{
		"SG_Node:SG_Voxel",
		sizeof(SG_Voxel),
		{ 0,0 },
		Init,
		Reset,
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	Draw,
	NULL,			/* intersect */
	NULL			/* edit */
};
