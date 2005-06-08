/*	$Csoft: color.c,v 1.3 2005/01/05 04:44:04 vedge Exp $	*/

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

#include "netbuf.h"
#include "integral.h"
#include "color.h"

void
AG_WriteColor(AG_Netbuf *buf, SDL_PixelFormat *pixfmt, Uint32 color)
{
	Uint8 r, g, b, a;

	SDL_GetRGBA(color, pixfmt, &r, &g, &b, &a);
	AG_WriteUint8(buf, r);
	AG_WriteUint8(buf, g);
	AG_WriteUint8(buf, b);
	AG_WriteUint8(buf, a);
}

Uint32
AG_ReadColor(AG_Netbuf *buf, SDL_PixelFormat *pixfmt)
{
	Uint8 r, g, b, a;

	r = AG_ReadUint8(buf);
	g = AG_ReadUint8(buf);
	b = AG_ReadUint8(buf);
	a = AG_ReadUint8(buf);
	return (SDL_MapRGBA(pixfmt, r, g, b, a));
}
