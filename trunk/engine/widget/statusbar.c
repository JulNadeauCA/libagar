/*	$Csoft$	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

static struct widget_ops statusbar_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		box_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,
	box_scale
};

struct statusbar *
statusbar_new(void *parent)
{
	struct statusbar *sbar;

	sbar = Malloc(sizeof(struct statusbar), M_OBJECT);
	statusbar_init(sbar);
	object_attach(parent, sbar);
	return (sbar);
}

void
statusbar_init(struct statusbar *sbar)
{
	box_init(&sbar->box, BOX_VERT, BOX_WFILL);
	box_set_padding(&sbar->box, 2);
	box_set_spacing(&sbar->box, 1);
	object_set_ops(sbar, &statusbar_ops);
	sbar->nlabels = 0;
}

struct label *
statusbar_add_label(struct statusbar *sbar, enum label_type type,
    const char *fmt, ...)
{
	struct label *lab;
	va_list ap;
	const char *p;

#ifdef DEBUG
	if (sbar->nlabels+1 >= STATUSBAR_MAX_LABELS)
		fatal("too many labels");
#endif
	sbar->labels[sbar->nlabels] = Malloc(sizeof(struct label), M_OBJECT);
	lab = sbar->labels[sbar->nlabels];

	if (type == LABEL_STATIC) {
		char buf[LABEL_MAX];

		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		label_init(lab, LABEL_STATIC, buf);
	} else {
		label_init(lab, LABEL_POLLED, fmt);
	}

	if (type == LABEL_POLLED || type == LABEL_POLLED_MT) {
		va_start(ap, fmt);
		lab->poll.lock = (type == LABEL_POLLED_MT) ?
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
					    LABEL_MAX_POLLPTRS) {
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
	object_attach(&sbar->box, lab);
	return (sbar->labels[sbar->nlabels++]);
}

