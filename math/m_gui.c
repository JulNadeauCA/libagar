/*
 * Copyright (c) 2006-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/config/enable_gui.h>
#ifdef ENABLE_GUI

/*
 * Utility GUI routines for types defined by the math library.
 */

#include <core/core.h>
#include <gui/widget.h>
#include <gui/box.h>
#include <gui/numerical.h>

#include "m.h"
#include "m_gui.h"

/* Edit a vector in R^3. */
void *
M_EditVector3(void *parent, const char *label, M_Vector3 *pv)
{
	AG_Box *box;
	AG_Numerical *n[3];
	int i;

	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewS(box, 0, label);

	n[0] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->x);
	n[1] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->y);
	n[2] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->z);

	for (i = 0; i < 3; i++) {
		AG_NumericalSetIncrement(n[i], 0.5);
	}
	return (box);
}

/* Edit a vector in R^3 (mutex-bound). */
void *
M_EditVector3Mp(void *parent, const char *label, M_Vector3 *pv, AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n[3];
	int i;

	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewS(box, 0, label);

	for (i = 0; i < 3; i++)
		n[i] = AG_NumericalNewS(box, 0, NULL, NULL);

	M_BindRealMp(n[0], "value", &pv->x, lock);
	M_BindRealMp(n[1], "value", &pv->y, lock);
	M_BindRealMp(n[2], "value", &pv->z, lock);

	for (i = 0; i < 3; i++)
		AG_NumericalSetIncrement(n[i], 0.5);

	return (box);
}

/* Edit a vector in R^4 */
void *
M_EditVector4(void *parent, const char *label, M_Vector4 *pv)
{
	AG_Box *box;
	AG_Numerical *n[4];
	int i;

	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewS(box, 0, label);

	n[0] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->x);
	n[1] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->y);
	n[2] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->z);
	n[3] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->w);

	for (i = 0; i < 3; i++) {
		AG_NumericalSetIncrement(n[i], 0.5);
	}
	AG_NumericalSetIncrement(n[3], 0.01);

	return (box);
}

/* Edit a vector in R^4 (mutex-bound) */
void *
M_EditVector4Mp(void *parent, const char *label, M_Vector4 *pv, AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n[4];
	int i;

	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewS(box, 0, label);

	for (i = 0; i < 4; i++)
		n[i] = M_NumericalNewReal(box, 0, NULL, NULL, &pv->x);

	M_BindRealMp(n[0], "value", &pv->x, lock);
	M_BindRealMp(n[1], "value", &pv->y, lock);
	M_BindRealMp(n[2], "value", &pv->z, lock);
	M_BindRealMp(n[3], "value", &pv->w, lock);
	
	for (i = 0; i < 3; i++) {
		AG_NumericalSetIncrement(n[i], 0.5);
	}
	AG_NumericalSetIncrement(n[3], 0.01);

	return (box);
}

/* Edit a 4x4 matrix. */
void *
M_EditMatrix44(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *hbox, *vbox;
	AG_Numerical *n;
	int i, j;

	vbox = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_EXPAND);
	if (label != NULL) {
		AG_LabelNewS(vbox, 0, label);
	}
	for (j = 0; j < 4; j++) {
		hbox = AG_BoxNew(vbox, AG_BOX_HORIZ,
		    AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
		for (i = 0; i < 4; i++) {
			n = M_NumericalNewReal(hbox, 0, NULL, NULL, &T->m[i][j]);
			AG_NumericalSetIncrement(n, 0.1);
		}
	}
	return (vbox);
}

/* Edit a 4x4 matrix (mutex-bound). */
void *
M_EditMatrix44Mp(void *parent, const char *label, M_Matrix44 *T, AG_Mutex *lock)
{
	AG_Box *hbox, *vbox;
	AG_Numerical *n;
	int i, j;

	vbox = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_EXPAND);
	if (label != NULL) {
		AG_LabelNewS(vbox, 0, label);
	}
	for (j = 0; j < 4; j++) {
		hbox = AG_BoxNew(vbox, AG_BOX_HORIZ,
		    AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
		for (i = 0; i < 4; i++) {
			n = AG_NumericalNewS(hbox, 0, NULL, NULL);
			M_BindRealMp(n, "value", &T->m[i][j], lock);
			AG_NumericalSetIncrement(n, 0.1);
		}
	}
	return (vbox);
}

/* Edit a 3-vector matrix translation. */
void *
M_EditTranslate3(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		n = M_NumericalNewReal(box, 0, NULL, NULL, &T->m[i][3]);
		AG_NumericalSetIncrement(n, 0.5);
	}
	return (box);
}

/* Edit a 3-vector matrix translation (mutex-bound). */
void *
M_EditTranslate3Mp(void *parent, const char *label, M_Matrix44 *T,
    AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		n = AG_NumericalNewS(box, 0, NULL, NULL);
		M_BindRealMp(n, "value", &T->m[i][3], lock);
		AG_NumericalSetIncrement(n, 0.5);
	}
	return (box);
}

/* Edit a 4-vector matrix translation. */
void *
M_EditTranslate4(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 4; i++) {
		n = M_NumericalNewReal(box, 0, NULL, NULL, &T->m[i][3]);
		AG_NumericalSetIncrement(n, 0.5);
	}
	return (box);
}

/* Edit a 4-vector matrix translation (mutex-bound). */
void *
M_EditTranslate4Mp(void *parent, const char *label, M_Matrix44 *T,
    AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 4; i++) {
		n = AG_NumericalNewS(box, 0, NULL, NULL);
		M_BindRealMp(n, "value", &T->m[i][3], lock);
		AG_NumericalSetIncrement(n, 0.5);
	}
	return (box);
}

/* Edit a 3-vector scaling. */
void *
M_EditScale3(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		n = M_NumericalNewReal(box, 0, NULL, NULL, &T->m[i][i]);
		AG_NumericalSetIncrement(n, 0.05);
	}
	return (box);
}

/* Edit a 3-vector scaling (mutex-bound). */
void *
M_EditScale3Mp(void *parent, const char *label, M_Matrix44 *T, AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 3; i++) {
		n = AG_NumericalNewS(box, 0, NULL, NULL);
		M_BindRealMp(n, "value", &T->m[i][i], lock);
		AG_NumericalSetIncrement(n, 0.05);
	}
	return (box);
}

/* Edit a 4-vector scaling. */
void *
M_EditScale4(void *parent, const char *label, M_Matrix44 *T)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 4; i++) {
		n = M_NumericalNewReal(box, 0, NULL, NULL, &T->m[i][i]);
		AG_NumericalSetIncrement(n, 0.05);
	}
	return (box);
}

/* Edit a 4-vector scaling (mutex-bound). */
void *
M_EditScale4Mp(void *parent, const char *label, M_Matrix44 *T, AG_Mutex *lock)
{
	AG_Box *box;
	AG_Numerical *n;
	int i;

	box = AG_BoxNew(parent, AG_BOX_VERT, AG_BOX_HFILL);
	if (label != NULL) {
		AG_LabelNewS(box, 0, label);
	}
	for (i = 0; i < 4; i++) {
		n = AG_NumericalNewS(box, 0, NULL, NULL);
		M_BindRealMp(n, "value", &T->m[i][i], lock);
		AG_NumericalSetIncrement(n, 0.05);
	}
	return (box);
}

#endif /* ENABLE_GUI */
