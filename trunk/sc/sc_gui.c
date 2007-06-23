/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
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

#include <core/core.h>

#include "sc.h"
#include "sc_gui.h"

#include <gui/label.h>
#include <gui/box.h>

AG_FSpinbutton *
SC_SpinReal(void *parent, const char *label, SC_Real *pv, SC_Real incr)
{
	AG_FSpinbutton *fsb;

	fsb = AG_FSpinbuttonNew(parent, 0, NULL, label);
	AG_WidgetBind(fsb, "value", SC_WIDGET_REAL, pv);
	AG_FSpinbuttonSetIncrement(fsb, incr);
	return (fsb);
}

void *
SC_EditVector3(void *parent, const char *label, SC_Vector *pv, SC_Real incr)
{
	AG_Box *box;
	AG_FSpinbutton *fsb;

#ifdef DEBUG
	if (pv->m < 3) { fatal("Vector has <3 components"); }
#endif
	box = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	AG_LabelNewStatic(box, 0, label);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, incr);
	SC_WidgetBindReal(fsb, "value", &pv->mat[1][1]);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, incr);
	SC_WidgetBindReal(fsb, "value", &pv->mat[2][1]);

	fsb = AG_FSpinbuttonNew(box, 0, NULL, NULL);
	AG_FSpinbuttonSetIncrement(fsb, incr);
	SC_WidgetBindReal(fsb, "value", &pv->mat[3][1]);
	return (box);
}

