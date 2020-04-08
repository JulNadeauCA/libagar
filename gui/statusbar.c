/*
 * Copyright (c) 2004-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Status bar widget. This is just a subclass of AG_Box(3) which embeds
 * one or more AG_Label(3).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/statusbar.h>
#include <agar/gui/window.h>

#include <stdarg.h>

AG_Statusbar *
AG_StatusbarNew(void *parent, Uint flags)
{
	AG_Statusbar *stb;

	stb = Malloc(sizeof(AG_Statusbar));
	AG_ObjectInit(stb, &agStatusbarClass);

	if (flags & AG_STATUSBAR_HFILL) { WIDGET(stb)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_STATUSBAR_VFILL) { WIDGET(stb)->flags |= AG_WIDGET_VFILL; }
	stb->flags |= flags;

	AG_ObjectAttach(parent, stb);
	return (stb);
}

static void
Init(void *_Nonnull obj)
{
	AG_Statusbar *stb = obj;
	AG_Box *box = obj;

	AG_BoxSetType(box, AG_BOX_VERT);

	stb->flags = 0;
	stb->nLabels = 0;
}

/* Add a Label to the container and set its contents. */
AG_Label *
AG_StatusbarAddLabel(AG_Statusbar *stb, const char *fmt, ...)
{
	AG_Label *lab;
	va_list ap;

	AG_OBJECT_ISA(stb, "AG_Widget:AG_Box:AG_Statusbar:*");
	AG_ObjectLock(stb);

#ifdef AG_DEBUG
	if (stb->nLabels+1 >= AG_STATUSBAR_MAX_LABELS)
		AG_FatalError("AG_StatusbarAddLabel: Too many labels");
#endif
	stb->labels[stb->nLabels] = Malloc(sizeof(AG_Label));
	lab = stb->labels[stb->nLabels];

	AG_ObjectInit(lab, &agLabelClass);

	WIDGET(lab)->flags |= AG_WIDGET_HFILL;

	lab->type = AG_LABEL_STATIC;
	va_start(ap, fmt);
	Vasprintf(&lab->text, fmt, ap);
	va_end(ap);

	AG_ObjectAttach(&stb->box, lab);
	lab = stb->labels[stb->nLabels++];

	AG_Redraw(stb);
	AG_ObjectUnlock(stb);

	return (lab);
}

AG_WidgetClass agStatusbarClass = {
	{
		"Agar(Widget:Box:Statusbar)",
		sizeof(AG_Statusbar),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	NULL,			/* size_request */
	NULL			/* size_allocate */
};

#endif /* AG_WIDGETS */
