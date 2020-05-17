/*
 * Copyright (c) 2011-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

SG_Map *
SG_MapNew(void *parent, const char *name)
{
	SG_Map *m;

	m = Malloc(sizeof(SG_Map));
	AG_ObjectInitNamed(m, &sgMapClass, name);
	AG_ObjectAttach(parent, m);
	return (m);
}

static void
InitBlk(SG_Blk *_Nonnull b)
{
	b->m = 0;
	b->T = 25.0;
	b->n = 0;
	b->c = NULL;
}

#if 0
static void
EditListPoll(AG_Event *_Nonnull event)
{
	SG_Map *m = SG_MAP_SELF();
	AG_Tlist *tl = AG_TLIST_PTR(1);
	const int depth = AG_INT(2);
}
#endif

static void
Init(void *_Nonnull obj)
{
	SG_Map *m = obj;

	m->flags = 0;
	m->root = Malloc(sizeof(SG_Blk));
	InitBlk(m->root);
#if 0
	AG_SetEvent(m, "edit-list-poll", EditListPoll, NULL);
#endif
}

static void
FreeBlk(SG_Blk *_Nonnull blk)
{
	Uint i;

	for (i = 0; i < blk->n; i++) {
		FreeBlk(&blk->c[i]);
	}
	Free(blk->c);
}

static void
Reset(void *_Nonnull obj)
{
	SG_Map *m = obj;

	FreeBlk(m->root);
	InitBlk(m->root);
}

int
SG_MapDivide(SG_Blk *bp, Uint n)
{
	if (bp->n != 0) {
		AG_SetError("Block is already divided by %u", bp->n);
		return (-1);
	}
	bp->c = Malloc(sizeof(SG_Blk)*n*3);
	bp->n = n;
	return (0);
}

static void
DrawBlock(SG_Map *_Nonnull m, SG_Blk *_Nonnull b, SG_View *_Nonnull sgv,
    M_Real bs)
{
	M_Real uc[8][3] = {
		{ 0.0, 0.0, 0.0 },
		{ bs,  0.0, 0.0 },
		{ bs,  0.0, bs },
		{ 0.0, 0.0, bs },
		{ 0.0, bs,  0.0 },
		{ bs,  bs,  0.0 },
		{ bs,  bs,  bs },
		{ 0.0, bs,  bs },
	};
	int x, y, z;
	M_Color C;
	M_Vector3 cv;
	M_Real cbs;
	
	C = M_ColorRGB(0.0, 0.0, 0.0);

	/* Render the block */
	GL_Begin(GL_QUADS);
	GL_MaterialColorv(GL_FRONT, GL_AMBIENT, &C);
	GL_MaterialColorv(GL_FRONT, GL_DIFFUSE, &C);
	GL_MaterialColorv(GL_FRONT, GL_SPECULAR, &C);
	GL_Normal3(0.0, -1.0, 0.0);			/* TOP */
	GL_Vertex3v(&uc[7]);
	GL_Vertex3v(&uc[6]);
	GL_Vertex3v(&uc[5]);
	GL_Vertex3v(&uc[4]);
	GL_Normal3(0.0, +1.0, 0.0);			/* BOTTOM */
	GL_Vertex3v(&uc[1]); 
	GL_Vertex3v(&uc[2]);
	GL_Vertex3v(&uc[3]);
	GL_Vertex3v(&uc[0]);
	GL_Normal3(0.0, 0.0, -1.0);			/* FRONT */
	GL_Vertex3v(&uc[4]);
	GL_Vertex3v(&uc[5]);
	GL_Vertex3v(&uc[1]);
	GL_Vertex3v(&uc[0]);
	GL_Normal3(0.0, 0.0, +1.0);			/* BACK */
	GL_Vertex3v(&uc[3]);
	GL_Vertex3v(&uc[2]);
	GL_Vertex3v(&uc[6]);
	GL_Vertex3v(&uc[7]);
	GL_Normal3(-1.0, 0.0, 0.0);			/* LEFT */
	GL_Vertex3v(&uc[3]);
	GL_Vertex3v(&uc[7]);
	GL_Vertex3v(&uc[4]);
	GL_Vertex3v(&uc[0]);
	GL_Normal3(+1.0, 0.0, 0.0);			/* RIGHT */
	GL_Vertex3v(&uc[5]);
	GL_Vertex3v(&uc[6]);
	GL_Vertex3v(&uc[2]);
	GL_Vertex3v(&uc[1]);
	GL_End();

	/* Render the block cells */
	GL_PushMatrix();
	cbs = bs/b->n;
	for (z = 0, cv.z = 0.0;
	     z < b->n;
	     z++, cv.z += cbs) {
		for (y = 0, cv.y = 0.0;
		     y < b->n;
		     y++, cv.y += cbs) {
			for (x = 0, cv.x = 0.0;
			     x < b->n;
			     x++, cv.z += cbs) {
				SG_Blk *cb = SG_BlkGet(b, x,y,z);

				GL_Translate(cv);
				DrawBlock(m, cb, sgv, cbs);
			}
		}
	}
	GL_PopMatrix();
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull sgv)
{
	SG_Map *m = obj;

	DrawBlock(m, m->root, sgv, 1.0);
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Map *sgm = obj;
	AG_Box *box;

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);
	AG_LabelNew(box, 0, "Map: %s", OBJECT(sgm)->name);

	

	return (box);
}

SG_NodeClass sgMapClass = {
	{
		"SG_Node:SG_Map",
		sizeof(SG_Map),
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
	Edit
};
