/*	$Csoft: real.c,v 1.3 2003/06/21 07:14:19 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

#include <SDL_types.h>
#include <SDL_endian.h>

#include <engine/loader/netbuf.h>
#include <engine/loader/real.h>

float
read_float(struct netbuf *buf)
{
	float f;

	netbuf_read(&f, sizeof(f), 1, buf);
	return (f);
}

void
write_float(struct netbuf *buf, float f)
{
	netbuf_write(&f, sizeof(f), 1, buf);
}

void
pwrite_float(struct netbuf *buf, float f, off_t offs)
{
	netbuf_pwrite(&f, sizeof(f), 1, offs, buf);
}

double
read_double(struct netbuf *buf)
{
	double f;

	netbuf_read(&f, sizeof(f), 1, buf);
	return (f);
}

void
write_double(struct netbuf *buf, double f)
{
	netbuf_write(&f, sizeof(f), 1, buf);
}

void
pwrite_double(struct netbuf *buf, double f, off_t offs)
{
	netbuf_pwrite(&f, sizeof(f), 1, offs, buf);
}

