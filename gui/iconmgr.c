/*
 * Copyright (c) 2007-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Icon resource object.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/surface.h>
#include <agar/gui/iconmgr.h>

/* Compile surfaces/textures for the given static icon. */
void
AG_InitStaticIcon(AG_StaticIcon *icon)
{
	const Uint32 *src;
	Uint8 *dst;
	int x, y;

	if (!agGUI)
		return;

	icon->s = AG_SurfaceRGBA(icon->w, icon->h, agSurfaceFmt->BitsPerPixel,
	    0, icon->Rmask, icon->Gmask, icon->Bmask, icon->Amask);
	
	src = &icon->data[0];
	dst = icon->s->pixels;
	for (y = 0; y < icon->h; y++) {
		for (x = 0; x < icon->w; x++) {
			AG_SurfacePut32_At(icon->s, dst, *src);
			dst += icon->s->format.BytesPerPixel;
			src++;
		}
	}
}

void
AG_FreeStaticIcon(AG_StaticIcon *icon)
{
	icon->s->flags &= ~(AG_SURFACE_MAPPED);
	AG_SurfaceFree(icon->s);
	icon->s = NULL;
}
