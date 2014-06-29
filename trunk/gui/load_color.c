/*
 * Copyright (c) 2004-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Serialization of colors in RGBA format.
 */

#include <agar/core/core.h>
#include <agar/gui/surface.h>
#include <agar/gui/load_color.h>

void
AG_WriteColor(AG_DataSource *ds, AG_Color C)
{
	if (ds->debug) {
		AG_WriteTypeCode(ds, AG_SOURCE_COLOR_RGBA);
	}
	AG_WriteUint8(ds, C.r);
	AG_WriteUint8(ds, C.g);
	AG_WriteUint8(ds, C.b);
	AG_WriteUint8(ds, C.a);
}

AG_Color
AG_ReadColor(AG_DataSource *ds)
{
	AG_Color C;

	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_COLOR_RGBA) == -1) {
		return AG_ColorRGB(255,0,0);
	}
	C.r = AG_ReadUint8(ds);
	C.g = AG_ReadUint8(ds);
	C.b = AG_ReadUint8(ds);
	C.a = AG_ReadUint8(ds);
	return (C);
}
