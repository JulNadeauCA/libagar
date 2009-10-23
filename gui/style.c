/*
 * Copyright (c) 2007-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Generic functions related to style attributes.
 */

#include <core/core.h>

#include "gui.h"

static void
ApplyStyle(AG_Widget *w, AG_Style *style)
{
	AG_Widget *wChld;

	AG_ObjectLock(w);
	AG_SetStyle(w, style);
	OBJECT_FOREACH_CHILD(wChld, w, ag_widget) {
		ApplyStyle(wChld, style);
	}
	AG_ObjectUnlock(w);
}

void
AG_SetStyle(void *obj, AG_Style *style)
{
	AG_ObjectLock(obj);
	if (AG_OfClass(obj, "AG_Driver:AG_DriverSw:*")) {
		AG_DriverSw *dsw = obj;
		AG_Window *win;

		dsw->style = style;
		AG_FOREACH_WINDOW(win, dsw) {
			ApplyStyle(WIDGET(win), style);
		}
	} else if (AG_OfClass(obj, "AG_Driver:AG_DriverMw:*")) {
		AG_DriverMw *dmw = obj;

		ApplyStyle(WIDGET(dmw->win), style);
	} else if (AG_OfClass(obj, "AG_Widget:*")) {
		WIDGET(obj)->style = style;
	} else {
		AG_FatalError("AG_SetStyle: Invalid argument");
	}
	if (style->init != NULL) {
		style->init(style);
	}
	AG_ObjectUnlock(obj);
}
