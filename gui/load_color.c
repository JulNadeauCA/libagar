/*
 * Copyright (c) 2004-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

void
AG_WriteColor(AG_DataSource *ds, AG_Color C)
{
#ifdef AG_DEBUG
	if (ds->debug)
		AG_WriteTypeCode(ds, AG_SOURCE_COLOR_RGBA);
#endif
#if AG_MODEL == AG_LARGE
	AG_WriteUint8(ds, 16);
	AG_WriteUint16(ds, C.r);
	AG_WriteUint16(ds, C.g);
	AG_WriteUint16(ds, C.b);
	AG_WriteUint16(ds, C.a);
#else
	AG_WriteUint8(ds, 8);
	AG_WriteUint8(ds, C.r);
	AG_WriteUint8(ds, C.g);
	AG_WriteUint8(ds, C.b);
	AG_WriteUint8(ds, C.a);
#endif
}

AG_Color
AG_ReadColor(AG_DataSource *ds)
{
	AG_Color C;
	Uint8 depth;

#ifdef AG_DEBUG
	if (ds->debug && AG_CheckTypeCode(ds, AG_SOURCE_COLOR_RGBA) == -1)
		AG_FatalError("Not COLOR_RGBA");
#endif
	depth = AG_ReadUint8(ds);
	if (depth == 16) {
#if AG_MODEL == AG_LARGE
		C.r = AG_ReadUint16(ds);
		C.g = AG_ReadUint16(ds);
		C.b = AG_ReadUint16(ds);
		C.a = AG_ReadUint16(ds);
#else
		C.r = AG_16to8(AG_ReadUint16(ds));
		C.g = AG_16to8(AG_ReadUint16(ds));
		C.b = AG_16to8(AG_ReadUint16(ds));
		C.a = AG_16to8(AG_ReadUint16(ds));
#endif
	} else if (depth == 8) {
#if AG_MODEL == AG_LARGE
		C.r = AG_8to16(AG_ReadUint8(ds));
		C.g = AG_8to16(AG_ReadUint8(ds));
		C.b = AG_8to16(AG_ReadUint8(ds));
		C.a = AG_8to16(AG_ReadUint8(ds));
#else
		C.r = AG_ReadUint8(ds);
		C.g = AG_ReadUint8(ds);
		C.b = AG_ReadUint8(ds);
		C.a = AG_ReadUint8(ds);
#endif
	} else {
		AG_FatalError("Bad depth");
	}
	return (C);
}
