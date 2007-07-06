/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Node object representing a plane.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sg.h"

SG_PlaneObj *
SG_PlaneObjNewPts(void *pNode, const char *name, SG_Vector p1, SG_Vector p2,
    SG_Vector p3)
{
	SG_PlaneObj *po;

	po = Malloc(sizeof(SG_PlaneObj), M_SG);
	SG_PlaneObjInit(po, name);
	po->P = SG_PlaneFrom3Pts(p1, p2, p3);
	SG_NodeAttach(pNode, po);
	return (po);
}

SG_PlaneObj *
SG_PlaneObjNew(void *pNode, const char *name, SG_Vector n, SG_Real d)
{
	SG_PlaneObj *po;

	po = Malloc(sizeof(SG_PlaneObj), M_SG);
	SG_PlaneObjInit(po, name);
	po->P = SG_PlaneFromNormal(n, d);
	SG_NodeAttach(pNode, po);
	return (po);
}

void
SG_PlaneObjInit(void *p, const char *name)
{
	SG_PlaneObj *po = p;

	SG_NodeInit(po, name, &sgPlaneObjOps, 0);
	po->P = SG_PlaneFromNormal(SG_J, 0.0);
}

int
SG_PlaneObjLoad(void *p, AG_Netbuf *buf)
{
	SG_PlaneObj *po = p;

	po->P = SG_ReadPlane(buf);
	return (0);
}

int
SG_PlaneObjSave(void *p, AG_Netbuf *buf)
{
	SG_PlaneObj *po = p;

	SG_WritePlane(buf, &po->P);
	return (0);
}

SG_NodeOps sgPlaneObjOps = {
	"Plane",
	sizeof(SG_PlaneObj),
	0,
	SG_PlaneObjInit,
	NULL,			/* destroy */
	SG_PlaneObjLoad,
	SG_PlaneObjSave,
	NULL,			/* edit */
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	NULL,			/* draw */
};

#endif /* HAVE_OPENGL */
