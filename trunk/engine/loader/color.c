/*	$Csoft: string.c,v 1.8 2004/01/03 04:25:08 vedge Exp $	*/

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

#include <engine/error/error.h>

#include <sys/types.h>
#include <SDL_types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "netbuf.h"
#include "integral.h"
#include "color.h"

void
write_color(struct netbuf *buf, SDL_PixelFormat *pixfmt, Uint32 color)
{
	Uint8 r, g, b, a;

	SDL_GetRGBA(color, pixfmt, &r, &g, &b, &a);
	write_uint8(buf, r);
	write_uint8(buf, g);
	write_uint8(buf, b);
	write_uint8(buf, a);
}

Uint32
read_color(struct netbuf *buf, SDL_PixelFormat *pixfmt)
{
	Uint8 r, g, b, a;

	r = read_uint8(buf);
	g = read_uint8(buf);
	b = read_uint8(buf);
	a = read_uint8(buf);
	return (SDL_MapRGBA(pixfmt, r, g, b, a));
}
