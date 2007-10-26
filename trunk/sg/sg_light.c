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
 * Light source node object.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <gui/notebook.h>
#include <gui/fspinbutton.h>
#include <gui/hsvpal.h>
#include <gui/separator.h>
#include <gui/toolbar.h>

#include "sg.h"
#include "sg_gui.h"

#include <string.h>

SG_Light *
SG_LightNew(void *pnode, const char *name)
{
	SG_Light *lt;

	lt = Malloc(sizeof(SG_Light), M_SG);
	SG_LightInit(lt, name);
	SG_NodeAttach(pnode, lt);
	return (lt);
}

SG_Color
SG_ColorRGB(SG_Real r, SG_Real g, SG_Real b)
{
	SG_Color cr;

	cr.r = r;
	cr.g = g;
	cr.b = b;
	cr.a = 1.0;
	return (cr);
}

SG_Color
SG_ColorRGBA(SG_Real r, SG_Real g, SG_Real b, SG_Real a)
{
	SG_Color cr;

	cr.r = r;
	cr.g = g;
	cr.b = b;
	cr.a = a;
	return (cr);
}

void
SG_ColorTo4fv(const SG_Color *cr, float *v)
{
#ifdef SG_DOUBLE_PRECISION
	/* TODO SIMD */
	v[0] = (float)cr->r;
	v[1] = (float)cr->g;
	v[2] = (float)cr->b;
	v[3] = (float)cr->a;
#else
	memcpy(v, cr, 4*sizeof(float));
#endif
}

void
SG_ColorTo4dv(const SG_Color *cr, double *v)
{
#ifdef SG_DOUBLE_PRECISION
	memcpy(v, cr, 4*sizeof(double));
#else
	/* TODO SIMD */
	v[0] = (double)cr->r;
	v[1] = (double)cr->g;
	v[2] = (double)cr->b;
	v[3] = (double)cr->a;
#endif
}

void
SG_LightInit(void *p, const char *name)
{
	static int nlight = GL_LIGHT0;
	SG_Light *lt = p;

	SG_NodeInit(lt, name, &sgLightOps, 0);
	lt->pri = 0;
	lt->light = nlight++; /* XXX test */
	lt->ambient = SG_ColorRGB(0.0, 0.0, 0.0);
	lt->diffuse = SG_ColorRGB(1.0, 1.0, 1.0);
	lt->specular = SG_ColorRGB(1.0, 1.0, 1.0);
	lt->spot_exponent = 3.0;
	lt->spot_cutoff = 180.0;
	lt->Kc = 1.0;
	lt->Kl = 0.0;
	lt->Kq = 0.0;

	AG_LockGL();				/* Probably not needed */
	lt->cyl = (GLUquadricObj *)gluNewQuadric();
	gluQuadricDrawStyle(lt->cyl, GLU_FILL);
	gluQuadricNormals(lt->cyl, GLU_SMOOTH);
	AG_UnlockGL();
}

void
SG_LightDestroy(void *p)
{
	SG_Light *lt = p;

	AG_LockGL();				/* Probably not needed */
	gluDeleteQuadric(lt->cyl);
	AG_UnlockGL();
}

void
SG_LightAlloc(SG_Light *lt, int name)
{
	lt->light = name;
}

void
SG_WriteColor(AG_DataSource *buf, SG_Color *cr)
{
	AG_WriteUint8(buf, 0);				/* Expn: type */
	SG_WriteReal(buf, cr->r);
	SG_WriteReal(buf, cr->g);
	SG_WriteReal(buf, cr->b);
	SG_WriteReal(buf, cr->a);
}

SG_Color
SG_ReadColor(AG_DataSource *buf)
{
	SG_Color cr;

	AG_ReadUint8(buf);				/* Expn: type */
	cr.r = SG_ReadReal(buf);
	cr.g = SG_ReadReal(buf);
	cr.b = SG_ReadReal(buf);
	cr.a = SG_ReadReal(buf);
	return (cr);
}

int
SG_LightLoad(void *p, AG_DataSource *buf)
{
	SG_Light *lt = p;

	lt->pri = (int)AG_ReadSint32(buf);
	lt->ambient = SG_ReadColor(buf);
	lt->diffuse = SG_ReadColor(buf);
	lt->specular = SG_ReadColor(buf);
	lt->spot_exponent = SG_ReadReal(buf);
	lt->spot_cutoff = SG_ReadReal(buf);
	lt->Kc = SG_ReadReal(buf);
	lt->Kl = SG_ReadReal(buf);
	lt->Kq = SG_ReadReal(buf);
	return (0);
}

int
SG_LightSave(void *p, AG_DataSource *buf)
{
	SG_Light *lt = p;

	AG_WriteSint32(buf, (Sint32)lt->pri);
	SG_WriteColor(buf, &lt->ambient);
	SG_WriteColor(buf, &lt->diffuse);
	SG_WriteColor(buf, &lt->specular);
	SG_WriteReal(buf, lt->spot_exponent);
	SG_WriteReal(buf, lt->spot_cutoff);
	SG_WriteReal(buf, lt->Kc);
	SG_WriteReal(buf, lt->Kl);
	SG_WriteReal(buf, lt->Kq);
	return (0);
}

void
SG_LightDraw(void *p, SG_View *view)
{
	SG_Light *lt = p;

	return;

	if (lt->spot_cutoff == 180.0) {
		gluSphere(lt->cyl, 0.125, 4, 4);
	} else {
		gluCylinder(lt->cyl, 0.125, 0.125, 1.0, 4, 3); 
	}
}

/*
 * Set up OpenGL light sources for rendering.
 * Must be called from widget draw context.
 */
void
SG_LightSetup(SG_Light *lt, SG_View *view)
{
	SG_Vector v;
	GLfloat posf[4];
	GLfloat dirf[4];

	glEnable(lt->light);
	v = SG_NodePos(lt);
	posf[0] = v.x;
	posf[1] = v.y;
	posf[2] = v.z;
	posf[3] = 1.0;
	glLightfv(lt->light, GL_POSITION, posf);

#ifdef SG_DOUBLE_PRECISION
	{
		float ambient[4];
		float diffuse[4];
		float specular[4];

		SG_ColorTo4fv(&lt->ambient, ambient);
		glLightfv(lt->light, GL_AMBIENT, ambient);
		SG_ColorTo4fv(&lt->diffuse, diffuse);
		glLightfv(lt->light, GL_DIFFUSE, diffuse);
		SG_ColorTo4fv(&lt->specular, specular);
		glLightfv(lt->light, GL_SPECULAR, specular);

		if (lt->spot_cutoff != 180.0) {
			v = SG_NodeDir(lt);
			dirf[0] = v.x;
			dirf[1] = v.y;
			dirf[2] = v.z;
			dirf[3] = 1.0;
			glLightfv(lt->light, GL_SPOT_DIRECTION, dirf);
		}
	}
#else
	glLightfv(lt->light, GL_AMBIENT, (GLfloat *)&lt->ambient);
	glLightfv(lt->light, GL_DIFFUSE, (GLfloat *)&lt->diffuse);
	glLightfv(lt->light, GL_SPECULAR, (GLfloat *)&lt->specular);
	if (lt->spot_cutoff != 180.0) {
		glLightfv(lt->light, GL_SPOT_DIRECTION, dirf);
	}
#endif
	glLightf(lt->light, GL_SPOT_EXPONENT, (GLfloat)lt->spot_exponent);
	glLightf(lt->light, GL_SPOT_CUTOFF, (GLfloat)lt->spot_cutoff);
	glLightf(lt->light, GL_CONSTANT_ATTENUATION, (GLfloat)lt->Kc);
	glLightf(lt->light, GL_LINEAR_ATTENUATION, (GLfloat)lt->Kl);
	glLightf(lt->light, GL_QUADRATIC_ATTENUATION, (GLfloat)lt->Kq);
}

static void
SelectColor(AG_Event *event)
{
	AG_HSVPal *pal = AG_PTR(1);
	void *color = AG_PTR(2);

	SG_WidgetBindReal(pal, "RGBAv", color);
}

void
SG_LightEdit(void *p, AG_Widget *box, SG_View *sgv)
{
	SG_Light *lt = p;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_HSVPal *pal;

	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAddTab(nb, _("Src"), AG_BOX_VERT);
	{
		SG_EditTranslate3(ntab, _("Position: "), &SGNODE(lt)->T);
		SG_SpinInt(ntab, _("Priority: "), &lt->pri);
		AG_SeparatorNewHoriz(ntab);
		AG_LabelNewStatic(ntab, 0, _("Using: GL_LIGHT%i"),
		    (int)(lt->light - GL_LIGHT0));
		AG_SeparatorNewHoriz(ntab);
		SG_SpinRealInc(ntab, _("Cutoff angle: "), &lt->spot_cutoff,
		                                          1.0);
		SG_SpinRealInc(ntab, _("Spot exponent: "), &lt->spot_exponent,
		                                           0.1);
	}
	ntab = AG_NotebookAddTab(nb, _("Attenuation"), AG_BOX_VERT);
	{
		SG_SpinRealInc(ntab, _("Constant: "), &lt->Kc, 0.01);
		SG_SpinRealInc(ntab, _("Linear: "), &lt->Kl, 0.001);
		SG_SpinRealInc(ntab, _("Quadratic: "), &lt->Kq, 0.00001);
	}
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_Toolbar *bar;
		
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		SG_WidgetBindReal(pal, "RGBAv", (void *)&lt->ambient);
		bar = AG_ToolbarNew(ntab, AG_TOOLBAR_HORIZ, 1,
		    AG_TOOLBAR_HOMOGENOUS|AG_TOOLBAR_STICKY);
		{
			AG_ToolbarButton(bar, _("Ambient"), 1,
			    SelectColor, "%p,%p", pal, &lt->ambient);
			AG_ToolbarButton(bar, _("Diffuse"), 0,
			    SelectColor, "%p,%p", pal, &lt->diffuse);
			AG_ToolbarButton(bar, _("Specular"), 0,
			    SelectColor, "%p,%p", pal, &lt->specular);
		}
	}
}

void
SG_LightMenu(void *obj, AG_MenuItem *m, SG_View *sgv)
{
#if 0
	SG_Light *lt = obj;

	AG_MenuAction(m, _("Light parameters..."), NULL,
	    EditLightParams, "%p", lt);
#endif
}

SG_NodeOps sgLightOps = {
	"Light",
	sizeof(SG_Light),
	0,
	SG_LightInit,
	SG_LightDestroy,
	SG_LightLoad,
	SG_LightSave,
	SG_LightEdit,
	NULL,		/* menuInstance */
	NULL,		/* menuClass */
	SG_LightDraw
};

#endif /* HAVE_OPENGL */
