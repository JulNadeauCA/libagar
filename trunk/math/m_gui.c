/*
 * Copyright (c) 2006-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Utility GUI routines for types defined by the math library.
 */

#include <core/core.h>
#include <gui/widget.h>
#include <gui/box.h>
#include <gui/fspinbutton.h>
#include <gui/spinbutton.h>

#include "m.h"
#include "m_gui.h"

AG_FSpinbutton *
M_SpinReal(void *parent, const char *label, M_Real *pv)
{
	AG_FSpinbutton *fsb;

	fsb = AG_FSpinbuttonNew(parent, 0, NULL, label);
	AG_WidgetBind(fsb, "value", M_WIDGET_REAL, pv);
	AG_FSpinbuttonSetIncrement(fsb, 0.05);
	return (fsb);
}

AG_FSpinbutton *
M_SpinRealInc(void *parent, const char *label, M_Real *pv, M_Real inc)
{
	AG_FSpinbutton *fsb;

	fsb = M_SpinReal(parent, label, pv);
	AG_FSpinbuttonSetIncrement(fsb, inc);
	return (fsb);
}

AG_FSpinbutton *
M_SpinFloat(void *parent, const char *label, float *pv)
{
	AG_FSpinbutton *fsb;

	fsb = AG_FSpinbuttonNew(parent, 0, NULL, label);
	AG_WidgetBind(fsb, "value", AG_WIDGET_FLOAT, pv);
	return (fsb);
}

AG_FSpinbutton *
M_SpinDouble(void *parent, const char *label, double *pv)
{
	AG_FSpinbutton *fsb;

	fsb = AG_FSpinbuttonNew(parent, 0, NULL, label);
	AG_WidgetBind(fsb, "value", AG_WIDGET_DOUBLE, pv);
	return (fsb);
}

AG_Spinbutton *
M_SpinInt(void *parent, const char *label, int *pv)
{
	AG_Spinbutton *sb;

	sb = AG_SpinbuttonNew(parent, 0, label);
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, pv);
	return (sb);
}

void *
M_EditVector3(void *parent, const char *label, M_Vector3 *pv)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;

	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewStatic(box, 0, label);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.5);
	M_WidgetBindReal(fsb, "value", &pv->x);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.5);
	M_WidgetBindReal(fsb, "value", &pv->y);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.5);
	M_WidgetBindReal(fsb, "value", &pv->z);

	return (box);
}

void *
M_EditVector4(void *parent, const char *label, M_Vector4 *pv)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;

	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewStatic(box, 0, label);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.5);
	M_WidgetBindReal(fsb, "value", &pv->x);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.5);
	M_WidgetBindReal(fsb, "value", &pv->y);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.5);
	M_WidgetBindReal(fsb, "value", &pv->z);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, 0.01);
	M_WidgetBindReal(fsb, "value", &pv->w);
	return (box);
}

void *
M_EditMatrix44(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *hbox, *vbox;
	AG_FSpinbutton *fsb;
	int i, j;

	vbox = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_EXPAND);
	if (label != NULL) {
		AG_LabelNewStatic(vbox, 0, label);
	}
	for (j = 0; j < 4; j++) {
		hbox = AG_BoxNew(vbox, AG_BOX_HORIZ,
		    AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
		for (i = 0; i < 4; i++) {
			fsb = AG_FSpinbuttonNew(hbox, 0, NULL, NULL);
			AG_FSpinbuttonSetIncrement(fsb, 0.1);
			M_WidgetBindReal(fsb, "value", &T->m[i][j]);
		}
	}
	return (vbox);
}

void *
M_EditTranslate3(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewStatic(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
		AG_FSpinbuttonSetIncrement(fsb, 0.5);
		M_WidgetBindReal(fsb, "value", &T->m[i][3]);
	}
	return (box);
}

void *
M_EditTranslate4(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewStatic(box, 0, label);
	}
	for (i = 0; i < 4; i++) {
		fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
		AG_FSpinbuttonSetIncrement(fsb, 0.5);
		M_WidgetBindReal(fsb, "value", &T->m[i][3]);
	}
	return (box);
}

void *
M_EditScale3(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewStatic(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
		AG_FSpinbuttonSetIncrement(fsb, 0.05);
		M_WidgetBindReal(fsb, "value", &T->m[i][i]);
	}
	return (box);
}

void *
M_EditScale4(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewStatic(box, 0, label);
	}
	for (i = 0; i < 4; i++) {
		fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
		AG_FSpinbuttonSetIncrement(fsb, 0.05);
		M_WidgetBindReal(fsb, "value", &T->m[i][i]);
	}
	return (box);
}
