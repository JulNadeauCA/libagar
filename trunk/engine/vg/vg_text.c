/*	$Csoft: vg_text.c,v 1.3 2004/04/22 12:36:09 vedge Exp $	*/

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

#include "vg.h"
#include "vg_text.h"

#include <stdarg.h>

void
vg_text_init(struct vg *vg, struct vg_element *vge)
{
	vge->vg_text.text[0] = '\0';
	vge->vg_text.angle = 0;
	vge->vg_text.align = VG_ALIGN_MC;
}

/* Specify text alignment and angle relative to the central vertex. */
void
vg_text_align(struct vg *vg, enum vg_alignment align, double angle)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_text.align = align;
	vge->vg_text.angle = angle;
}

/* Specify the font to use. */
int
vg_text_font(struct vg *vg, const char *face, int size, int style)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	if (size < VG_FONT_SIZE_MIN || size > VG_FONT_SIZE_MAX) {
		error_set(_("Illegal font size."));
		return (-1);
	}
	strlcpy(vge->vg_text.face, face, sizeof(vge->vg_text.face));
	vge->vg_text.size = size;
	vge->vg_text.style = style;
	return (0);
}

/* Specify the text to display. */
void
vg_text_printf(struct vg *vg, const char *fmt, ...)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);
	va_list args;

	va_start(args, fmt);
	vsnprintf(vge->vg_text.text, sizeof(vge->vg_text.text), fmt, args);
	va_end(args);
}

