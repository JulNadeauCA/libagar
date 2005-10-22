/*	$Csoft$	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include "sg.h"
#include "sg_view.h"

const AG_WidgetOps sgViewOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		SG_ViewDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_GLViewDraw,
	AG_GLViewScale
};

SG_View	*
SG_ViewNew(void *parent, SG *sg, Uint flags)
{
	SG_View *sv;

	sv = Malloc(sizeof(SG_View), M_OBJECT);
	SG_ViewInit(sv, sg, flags);
	AG_ObjectAttach(parent, sv);
	return (sv);
}

void
SG_ViewInit(SG_View *sv, SG *sg, Uint flags)
{
	Uint glvflags = 0;

	if (flags & SG_VIEW_WFILL) { glvflags |= AG_GLVIEW_WFILL; }
	if (flags & SG_VIEW_HFILL) { glvflags |= AG_GLVIEW_HFILL; }

	AG_GLViewInit(AGGLVIEW(sv), glvflags);
	AG_ObjectSetOps(sv, &sgViewOps);

	sv->flags = flags;
	sv->sg = sg;
}

void
SG_ViewDestroy(void *p)
{
	AG_GLViewDestroy(p);
}

void
SG_ViewKeydownFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewKeyupFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewButtondownFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewButtonupFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}

void
SG_ViewMotionFn(SG_View *sv, AG_EventFn fn, const char *fmt, ...)
{
}
