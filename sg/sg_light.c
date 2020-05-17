/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

#include <string.h>

SG_Light *
SG_LightNew(void *parent, const char *name)
{
	SG_Light *lt;

	lt = Malloc(sizeof(SG_Light));
	AG_ObjectInit(lt, &sgLightClass);
	if (name) {
		AG_ObjectSetNameS(lt, name);
	} else {
		OBJECT(lt)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, lt);
	return (lt);
}

static void
Init(void *_Nonnull obj)
{
	SG_Light *lt = obj;
	static int nlight = GL_LIGHT0;			/* XXX */

	lt->flags = 0;
	lt->pri = 0;
	lt->light = nlight++;				/* XXX test */
	lt->tag = 0;
	lt->ambient = M_ColorBlack();
	lt->diffuse = M_ColorWhite();
	lt->specular = M_ColorWhite();
	lt->spot_dir = M_VecZero3();
	lt->spot_exponent = 3.0;
	lt->spot_cutoff = 180.0;
	lt->Kc = 1.0;
	lt->Kl = 0.0;
	lt->Kq = 0.0;
#ifdef HAVE_GLU
	lt->qo = gluNewQuadric();
	gluQuadricDrawStyle(lt->qo, GLU_FILL);
	gluQuadricNormals(lt->qo, GLU_SMOOTH);
#endif
}

static void
Destroy(void *_Nonnull obj)
{
#ifdef HAVE_GLU
	SG_Light *lt = obj;
	gluDeleteQuadric(lt->qo);
#endif
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Light *lt = obj;

	lt->flags = (Uint)AG_ReadUint8(ds);
	lt->pri = (int)AG_ReadSint32(ds);
	lt->light = (int)AG_ReadSint32(ds);
	lt->tag = (int)AG_ReadSint32(ds);
	lt->ambient = M_ReadColor(ds);
	lt->diffuse = M_ReadColor(ds);
	lt->specular = M_ReadColor(ds);
	lt->spot_dir = M_ReadVector3(ds);
	lt->spot_exponent = M_ReadReal(ds);
	lt->spot_cutoff = M_ReadReal(ds);
	lt->Kc = M_ReadReal(ds);
	lt->Kl = M_ReadReal(ds);
	lt->Kq = M_ReadReal(ds);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Light *lt = obj;

	AG_WriteSint32(ds, (Sint32)lt->pri);
	AG_WriteSint32(ds, (Sint32)lt->light);
	AG_WriteSint32(ds, (Sint32)lt->tag);
	M_WriteColor(ds, &lt->ambient);
	M_WriteColor(ds, &lt->diffuse);
	M_WriteColor(ds, &lt->specular);
	M_WriteVector3(ds, &lt->spot_dir);
	M_WriteReal(ds, lt->spot_exponent);
	M_WriteReal(ds, lt->spot_cutoff);
	M_WriteReal(ds, lt->Kc);
	M_WriteReal(ds, lt->Kl);
	M_WriteReal(ds, lt->Kq);
	return (0);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull view)
{
#ifdef HAVE_GLU
	SG_Light *lt = obj;

	if (lt->spot_cutoff == 180.0) {
		gluSphere(lt->qo, 0.125, 4, 4);
	} else {
		gluCylinder(lt->qo, 0.125, 0.125, 1.0, 4, 3); 
	}
#endif /* HAVE_GLU */
}

/*
 * Set up OpenGL light sources for rendering.
 * Must be called from widget draw context.
 */
void
SG_LightSetup(SG_Light *lt)
{
	const int which = lt->light;
	M_Vector3 v;
	GLfloat posf[4];
	GLfloat dirf[4];

	AG_ObjectLock(lt);

	GL_Enable(which);
	v = SG_NodePos(lt);
	posf[0] = (GLfloat)v.x;
	posf[1] = (GLfloat)v.y;
	posf[2] = (GLfloat)v.z;
	posf[3] = 1.0f;
	glLightfv(which, GL_POSITION, posf);

#ifdef DOUBLE_PRECISION
	{
		float ambient[4];
		float diffuse[4];
		float specular[4];

		M_ColorTo4fv(&lt->ambient, ambient);
		glLightfv(which, GL_AMBIENT, ambient);
		M_ColorTo4fv(&lt->diffuse, diffuse);
		glLightfv(which, GL_DIFFUSE, diffuse);
		M_ColorTo4fv(&lt->specular, specular);
		glLightfv(which, GL_SPECULAR, specular);

		if (lt->spot_cutoff != 180.0) {
			v = SG_NodeDir(lt);
			dirf[0] = (GLfloat)v.x;
			dirf[1] = (GLfloat)v.y;
			dirf[2] = (GLfloat)v.z;
			dirf[3] = 1.0f;
			glLightfv(which, GL_SPOT_DIRECTION, dirf);
		}
	}
#else /* !DOUBLE_PRECISION */
	glLightfv(which, GL_AMBIENT,  (GLfloat *)&lt->ambient);
	glLightfv(which, GL_DIFFUSE,  (GLfloat *)&lt->diffuse);
	glLightfv(which, GL_SPECULAR, (GLfloat *)&lt->specular);
	if (lt->spot_cutoff != 180.0) {
		glLightfv(which, GL_SPOT_DIRECTION, dirf);
	}
#endif
	glLightf(which, GL_SPOT_EXPONENT,         (GLfloat)lt->spot_exponent);
	glLightf(which, GL_SPOT_CUTOFF,           (GLfloat)lt->spot_cutoff);
	glLightf(which, GL_CONSTANT_ATTENUATION,  (GLfloat)lt->Kc);
	glLightf(which, GL_LINEAR_ATTENUATION,    (GLfloat)lt->Kl);
	glLightf(which, GL_QUADRATIC_ATTENUATION, (GLfloat)lt->Kq);
	
	AG_ObjectUnlock(lt);
}

static void
SelectColor(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_PTR(1);
	void *color = AG_PTR(2);
	AG_Mutex *lock = AG_PTR(3);

	M_BindRealMp(pal, "RGBAv", color, lock);
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Light *lt = obj;
	AG_Mutex *lock = &OBJECT(lt)->lock;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_HSVPal *pal;
	AG_Numerical *num;

	nb = AG_NotebookNew(NULL, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAdd(nb, _("Src"), AG_BOX_VERT);
	{
		M_EditTranslate3Mp(ntab, _("Position: "),
		    &SGNODE(lt)->T, lock);

		num = AG_NumericalNew(ntab, 0, NULL, _("Priority: "));
		AG_BindIntMp(num, "value", &lt->pri, lock);

		AG_SeparatorNewHoriz(ntab);
		
		AG_LabelNew(ntab, 0, _("Using: GL_LIGHT%i"),
		    (int)(lt->light - GL_LIGHT0));

		AG_SeparatorNewHoriz(ntab);

		num = AG_NumericalNew(ntab, 0, "deg", _("Cutoff angle: "));
		M_BindRealMp(num, "value", &lt->spot_cutoff, lock);

		num = AG_NumericalNew(ntab, 0, NULL, _("Spot exponent: "));
		M_BindRealMp(num, "value", &lt->spot_exponent, lock);
		M_SetReal(num, "inc", 0.1);
	}
	ntab = AG_NotebookAdd(nb, _("Attenuation"), AG_BOX_VERT);
	{
		num = AG_NumericalNew(ntab, 0, NULL, "Kc: ");
		M_BindRealMp(num, "value", &lt->Kc, lock);
		M_SetReal(num, "inc", 0.01);
	
		num = AG_NumericalNew(ntab, 0, NULL, "Kl: ");
		M_BindRealMp(num, "value", &lt->Kl, lock);
		M_SetReal(num, "inc", 0.001);

		num = AG_NumericalNew(ntab, 0, NULL, "Kq: ");
		M_BindRealMp(num, "value", &lt->Kq, lock);
		M_SetReal(num, "inc", 0.00001);
	}
	ntab = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
	{
		AG_Toolbar *bar;
		
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		M_BindRealMp(pal, "RGBAv", (void *)&lt->ambient, lock);

		bar = AG_ToolbarNew(ntab, AG_TOOLBAR_HORIZ, 1,
		    AG_TOOLBAR_HOMOGENOUS|AG_TOOLBAR_STICKY);
		{
			AG_ToolbarButton(bar, _("Ambient"), 1,
			    SelectColor, "%p,%p", pal, &lt->ambient, lock);
			AG_ToolbarButton(bar, _("Diffuse"), 0,
			    SelectColor, "%p,%p", pal, &lt->diffuse, lock);
			AG_ToolbarButton(bar, _("Specular"), 0,
			    SelectColor, "%p,%p", pal, &lt->specular, lock);
		}
	}
	return (nb);
}

#if 0
static void
MenuInstance(void *_Nonnull obj, AG_MenuItem *_Nonnull m, SG_View *_Nonnull sgv)
{
#if 0
	SG_Light *lt = obj;

	AG_MenuAction(m, _("Light parameters..."), NULL,
	    EditLightParams, "%p", lt);
#endif
}
#endif

SG_NodeClass sgLightClass = {
	{
		"SG_Node:SG_Light",
		sizeof(SG_Light),
		{ 0,0 },
		Init,
		NULL,	/* reset */
		Destroy,
		Load,
		Save,
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	Draw,
	NULL,			/* intersect */
	Edit
};
