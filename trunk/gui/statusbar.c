/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "statusbar.h"

#include <stdarg.h>

AG_Statusbar *
AG_StatusbarNew(void *parent, Uint flags)
{
	AG_Statusbar *sbar;

	sbar = Malloc(sizeof(AG_Statusbar), M_OBJECT);
	AG_StatusbarInit(sbar, flags);
	AG_ObjectAttach(parent, sbar);
	return (sbar);
}

void
AG_StatusbarInit(AG_Statusbar *sbar, Uint flags)
{
	AG_BoxInit(&sbar->box, AG_BOX_VERT, AG_BOX_HFILL);
	AG_BoxSetPadding(&sbar->box, 2);
	AG_BoxSetSpacing(&sbar->box, 1);
	AG_ObjectSetOps(sbar, &agStatusbarOps);
	sbar->nlabels = 0;
}

AG_Label *
AG_StatusbarAddLabel(AG_Statusbar *sbar, enum ag_label_type type,
    const char *fmt, ...)
{
	AG_Label *lab;
	va_list ap;
	const char *p;

#ifdef DEBUG
	if (sbar->nlabels+1 >= AG_STATUSBAR_MAX_LABELS)
		fatal("too many labels");
#endif
	sbar->labels[sbar->nlabels] = Malloc(sizeof(AG_Label), M_OBJECT);
	lab = sbar->labels[sbar->nlabels];

	if (type == AG_LABEL_STATIC) {
		char *s;

		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
		AG_LabelInit(lab, AG_LABEL_STATIC, AG_LABEL_HFILL, s);
		Free(s, 0);
	} else {
		AG_LabelInit(lab, AG_LABEL_POLLED, AG_LABEL_HFILL, fmt);
	}

	if (type == AG_LABEL_POLLED || type == AG_LABEL_POLLED_MT) {
		va_start(ap, fmt);
		lab->poll.lock = (type == AG_LABEL_POLLED_MT) ?
		                 va_arg(ap, AG_Mutex *) : NULL;
		for (p = fmt; *p != '\0'; p++) {
			if (*p == '%' && *(p+1) != '\0') {
				switch (*(p+1)) {
				case ' ':
				case '(':
				case ')':
				case '%':
					break;
				default:
					if (lab->poll.nptrs+1 <
					    AG_LABEL_MAX_POLLPTRS) {
						lab->poll.ptrs
						    [lab->poll.nptrs++] =
						    va_arg(ap, void *);
					}
					break;
				}
			}
		}
		va_end(ap);
	}
	AG_ObjectAttach(&sbar->box, lab);
	return (sbar->labels[sbar->nlabels++]);
}

const AG_WidgetOps agStatusbarOps = {
	{
		"AG_Widget:AG_Box:AG_Statusbar",
		sizeof(AG_Statusbar),
		{ 0,0 },
		NULL,				/* init */
		NULL,				/* reinit */
		AG_BoxDestroy,
		NULL,				/* load */
		NULL,				/* save */
		NULL				/* edit */
	},
	NULL,					/* draw */
	AG_BoxSizeRequest,
	AG_BoxSizeAllocate
};
