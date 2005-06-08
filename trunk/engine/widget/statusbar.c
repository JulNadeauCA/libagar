/*	$Csoft: statusbar.c,v 1.2 2005/01/05 04:44:05 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/view.h>

#include "statusbar.h"

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static AG_WidgetOps statusbar_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,
	AG_BoxScale
};

AG_Statusbar *
AG_StatusbarNew(void *parent)
{
	AG_Statusbar *sbar;

	sbar = Malloc(sizeof(AG_Statusbar), M_OBJECT);
	AG_StatusbarInit(sbar);
	AG_ObjectAttach(parent, sbar);
	return (sbar);
}

void
AG_StatusbarInit(AG_Statusbar *sbar)
{
	AG_BoxInit(&sbar->box, AG_BOX_VERT, AG_BOX_WFILL);
	AG_BoxSetPadding(&sbar->box, 2);
	AG_BoxSetSpacing(&sbar->box, 1);
	AG_ObjectSetOps(sbar, &statusbar_ops);
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
		char buf[AG_LABEL_MAX];

		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		AG_LabelInit(lab, AG_LABEL_STATIC, buf);
	} else {
		AG_LabelInit(lab, AG_LABEL_POLLED, fmt);
	}

	if (type == AG_LABEL_POLLED || type == AG_LABEL_POLLED_MT) {
		va_start(ap, fmt);
		lab->poll.lock = (type == AG_LABEL_POLLED_MT) ?
		                 va_arg(ap, pthread_mutex_t *) : NULL;
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

