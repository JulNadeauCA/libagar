/*	$Csoft: integral.c,v 1.2 2003/06/21 06:50:20 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include <stdio.h>

#include <engine/loader/netbuf.h>
#include <engine/loader/integral.h>

#include <SDL_endian.h>

Uint8
read_uint8(struct netbuf *buf)
{
	Uint8 i;

	netbuf_read(&i, sizeof(i), 1, buf);
	return (i);
}

void
write_uint8(struct netbuf *buf, Uint8 i)
{
	netbuf_write(&i, sizeof(i), 1, buf);
}

void
pwrite_uint8(struct netbuf *buf, Uint8 i, off_t offs)
{
	netbuf_pwrite(&i, sizeof(i), 1, offs, buf);
}

Uint16
read_uint16(struct netbuf *buf)
{
	Uint16 i;

	netbuf_read(&i, sizeof(i), 1, buf);
	return ((buf->byte_order == NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE16(i) : SDL_SwapLE16(i));
	
}

void
write_uint16(struct netbuf *buf, Uint16 u16)
{
	Uint16 i = (buf->byte_order == NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE16(u16) : SDL_SwapLE16(u16);

	netbuf_write(&i, sizeof(i), 1, buf);
}

void
pwrite_uint16(struct netbuf *buf, Uint16 u16, off_t offs)
{
	Uint16 i = (buf->byte_order == NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE16(u16) : SDL_SwapLE16(u16);

	netbuf_pwrite(&i, sizeof(i), 1, offs, buf);
}

Uint32
read_uint32(struct netbuf *buf)
{
	Uint32 i;

	netbuf_read(&i, sizeof(i), 1, buf);
	return ((buf->byte_order == NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE32(i) : SDL_SwapLE32(i));
}

void
write_uint32(struct netbuf *buf, Uint32 u32)
{
	Uint32 i = (buf->byte_order == NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE32(u32) : SDL_SwapLE32(u32);

	netbuf_write(&i, sizeof(i), 1, buf);
}

void
pwrite_uint32(struct netbuf *buf, Uint32 u32, off_t offs)
{
	Uint32 i = (buf->byte_order == NETBUF_BIG_ENDIAN) ?
	    SDL_SwapBE32(u32) : SDL_SwapLE32(u32);

	netbuf_pwrite(&i, sizeof(i), 1, offs, buf);
}

