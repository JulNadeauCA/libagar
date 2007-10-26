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
 * Derivative of SG_Object that does not allow thin features, but provides
 * mass information for use in physical simulations.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"
#include "sg_gui.h"

#include <string.h>

SG_Solid *
SG_SolidNew(void *pnode, const char *name)
{
	SG_Solid *sld;

	sld = Malloc(sizeof(SG_Solid), M_SG);
	SG_SolidInit(sld, name);
	SG_NodeAttach(pnode, sld);
	return (sld);
}

void
SG_SolidInit(void *p, const char *name)
{
	SG_Solid *sld = p;

	SG_ObjectInit(sld, name);
	SGNODE(sld)->ops = &sgSolidOps;
}

void
SG_SolidDestroy(void *p)
{
	SG_Solid *sld = p;

	SG_ObjectDestroy(sld);
}

int
SG_SolidLoad(void *p, AG_DataSource *buf)
{
	SG_Solid *sld = p;

	if (SG_ObjectLoad(sld, buf) == -1) {
		return (-1);
	}
	sld->vLin = SG_ReadVector(buf);
	sld->vAng = SG_ReadVector(buf);
	sld->rho = SG_ReadReal(buf);
#ifdef DEBUG
	if (sld->rho < 0.0) { fatal("Invalid density"); }
#endif
	return (0);
}

int
SG_SolidSave(void *p, AG_DataSource *buf)
{
	SG_Solid *sld = p;

	if (SG_ObjectSave(sld, buf) == -1) {
		return (-1);
	}
	SG_WriteVector(buf, &sld->vLin);
	SG_WriteVector(buf, &sld->vAng);
	SG_WriteReal(buf, sld->rho);
	return (0);
}

/*
 * Build a parallelepiped of given width, height and depth.
 * Must be called from widget draw context.
 */
void
SG_SolidBox(SG_Solid *sld, SG_Real wd, SG_Real ht, SG_Real dp)
{
	SG_Vector d = VecGet(0.0, ht, 0.0);
	SG_Facet *fct;

	SG_VertexNew2(sld, 0.0, 0.0);
	SG_VertexNew2(sld, wd, 0.0);
	SG_VertexNew2(sld, wd, dp);
	SG_VertexNew2(sld, 0.0, dp);

	fct = SG_FacetFromQuad4(sld, 3,2,1,0);
	if ((SG_FacetExtrude(sld, fct, d, SG_EXTRUDE_REGION)) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
	if (SG_ObjectCheckConnectivity(sld) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
	SG_ObjectNormalize(sld);
}

SG_NodeOps sgSolidOps = {
	"Object:Solid",
	sizeof(SG_Solid),
	0,
	SG_SolidInit,
	SG_SolidDestroy,
	SG_SolidLoad,
	SG_SolidSave,
	NULL,			/* edit */
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	SG_SolidDraw
};

#endif /* HAVE_OPENGL */
