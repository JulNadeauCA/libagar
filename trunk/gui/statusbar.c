/*
 * Copyright (c) 2004-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/statusbar.h>
#include <agar/gui/window.h>

#include <stdarg.h>

AG_Statusbar *
AG_StatusbarNew(void *parent, Uint flags)
{
	AG_Statusbar *sbar;

	sbar = Malloc(sizeof(AG_Statusbar));
	AG_ObjectInit(sbar, &agStatusbarClass);
	sbar->flags |= flags;
	if (flags & AG_STATUSBAR_HFILL) { AG_ExpandHoriz(sbar); }
	if (flags & AG_STATUSBAR_VFILL) { AG_ExpandVert(sbar); }
	AG_ObjectAttach(parent, sbar);
	return (sbar);
}

static void
Init(void *obj)
{
	AG_Statusbar *sbar = obj;
	AG_Box *box = obj;

	AG_BoxSetType(box, AG_BOX_VERT);
	AG_BoxSetPadding(box, 2);
	AG_BoxSetSpacing(box, 1);
	sbar->flags = 0;
	sbar->nlabels = 0;
}

AG_Label *
AG_StatusbarAddLabel(AG_Statusbar *sbar, const char *fmt, ...)
{
	AG_Label *lab;
	va_list ap;

	AG_ObjectLock(sbar);
#ifdef AG_DEBUG
	if (sbar->nlabels+1 >= AG_STATUSBAR_MAX_LABELS)
		AG_FatalError("AG_StatusbarAddLabel: Too many labels");
#endif
	sbar->labels[sbar->nlabels] = Malloc(sizeof(AG_Label));
	lab = sbar->labels[sbar->nlabels];

	AG_ObjectInit(lab, &agLabelClass);
	lab->type = AG_LABEL_STATIC;
	va_start(ap, fmt);
	Vasprintf(&lab->text, fmt, ap);
	va_end(ap);
	AG_ExpandHoriz(lab);

	AG_ObjectAttach(&sbar->box, lab);
	lab = sbar->labels[sbar->nlabels++];
	AG_ObjectUnlock(sbar);
	AG_Redraw(sbar);
	return (lab);
}

AG_WidgetClass agStatusbarClass = {
	{
		"Agar(Widget:Box:Statusbar)",
		sizeof(AG_Statusbar),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_WidgetInheritDraw,
	AG_WidgetInheritSizeRequest,
	AG_WidgetInheritSizeAllocate
};
