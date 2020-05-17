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

/*
 * Base class for elements of reference geometry.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

/* Cast a generic M_Geom3 structure to the corresponding SG entity. */
SG_Geom *
SG_GeomNew(void *parent, const char *name, M_Geom3 g)
{
	SG_Geom *geo;

	switch (g.type) {
	case M_POINT:
		geo = (SG_Geom *)SG_PointNew(parent, name, &g.g.point);
		break;
	case M_LINE:
		geo = (SG_Geom *)SG_LineNew(parent, name, &g.g.line);
		break;
	case M_CIRCLE:
		geo = (SG_Geom *)SG_CircleNew(parent, name, &g.g.circle);
		break;
	case M_SPHERE:
		geo = (SG_Geom *)SG_SphereNew(parent, name, &g.g.sphere);
		break;
#if 0
	case M_PLANE:
		geo = (SG_Geom *)SG_PlaneNew(parent, name, &g.g.plane);
		break;
#endif
	case M_POLYGON:
		geo = (SG_Geom *)SG_PolygonNew(parent, name, &g.g.polygon);
		break;
	case M_TRIANGLE:
		geo = (SG_Geom *)SG_TriangleNew(parent, name, &g.g.triangle);
		break;
	case M_RECTANGLE:
		geo = (SG_Geom *)SG_RectangleNew(parent, name, &g.g.rectangle);
		break;
	case M_NONE:
	default:
		geo = Malloc(sizeof(SG_Geom));
		AG_ObjectInit(geo, &sgGeomClass);
		if (name) {
			AG_ObjectSetNameS(geo, name);
		} else {
			OBJECT(geo)->flags |= AG_OBJECT_NAME_ONATTACH;
		}
		AG_ObjectAttach(parent, geo);
		break;
	}
	return (geo);
}

static void
Init(void *_Nonnull obj)
{
	SG_Geom *geo = obj;

	geo->flags = 0;
	geo->stFactor = 1;
	geo->wd = 1.0;
	geo->c = M_ColorBlack();
	geo->stPat = 0xffff;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Geom *geo = obj;

	geo->flags &= ~(SG_GEOM_SAVED);
	geo->flags |= (AG_ReadUint32(ds) & SG_GEOM_SAVED);
	geo->wd = M_ReadReal(ds);
	geo->c = M_ReadColor(ds);
	geo->stFactor = (int)AG_ReadSint16(ds);
	geo->stPat = AG_ReadUint16(ds);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Geom *geom = obj;

	AG_WriteUint32(ds, (geom->flags & SG_GEOM_SAVED));
	M_WriteReal(ds, geom->wd);
	M_WriteColor(ds, &geom->c);
	AG_WriteSint16(ds, (Sint16)geom->stFactor);
	AG_WriteUint16(ds, geom->stPat);
	return (0);
}

static void *_Nullable
Edit(void *_Nonnull p, SG_View *_Nullable sgv)
{
	SG_Geom *geo = p;
	AG_Mutex *lock = &OBJECT(geo)->lock;
	AG_Box *box;
	AG_Numerical *num;
	AG_HSVPal *pal;

	box = AG_BoxNew(NULL, AG_BOX_VERT, AG_BOX_HFILL);
	
	num = AG_NumericalNew(box, 0, "px", _("Line Width"));
	M_BindRealMp(num, "value", &geo->wd, lock);
	num = AG_NumericalNew(box, 0, NULL, _("Stipple Factor"));
	AG_BindIntMp(num, "value", &geo->stFactor, lock);
	num = AG_NumericalNew(box, 0, NULL, _("Stipple Pattern"));
	AG_BindUint16Mp(num, "value", &geo->stPat, lock);
	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
	M_BindRealMp(pal, "RGBAv", (void *)&geo->c, lock);

	return (box);
}

void
SG_GeomColor(SG_Geom *geom, M_Color c)
{
	AG_ObjectLock(geom);
	geom->c = c;
	AG_ObjectUnlock(geom);
}

void
SG_GeomLineWidth(SG_Geom *geom, M_Real wd)
{
	AG_ObjectLock(geom);
	geom->wd = wd;
	AG_ObjectUnlock(geom);
}

void
SG_GeomLineStipple(SG_Geom *geom, int fac, Uint16 pat)
{
	AG_ObjectLock(geom);
	geom->stFactor = fac;
	geom->stPat = pat;
	AG_ObjectUnlock(geom);
}

void
SG_GeomDrawBegin(const SG_Geom *geom)
{
	GL_PushAttrib(GL_LIGHTING_BIT|GL_LINE_BIT);
	GL_Disable(GL_LIGHTING);
	GL_LineWidth(geom->wd);
	GL_LineStipple(geom->stFactor, geom->stPat);
	GL_Enable(GL_LINE_STIPPLE);
}

SG_NodeClass sgGeomClass = {
	{
		"SG_Node:SG_Geom",
		sizeof(SG_Geom),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		Load,
		Save,
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	NULL,			/* draw */
	NULL,			/* intersect */
	Edit
};
