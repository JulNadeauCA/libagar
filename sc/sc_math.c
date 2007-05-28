/*	$Csoft: vg_math.c,v 1.7 2005/01/05 04:44:05 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <agar/core/core.h>

#include "sc_pvt.h"

SC_Real
SC_Rad2Deg(SC_Real theta)
{
	return (theta/(2.0*SC_PI)*360.0);
}

SC_Real
SC_Deg2Rad(SC_Real theta)
{
	return ((theta/360.0)*(2.0*SC_PI));
}

SC_Real
SC_ReadReal(AG_Netbuf *buf)
{
	double rv;

	AG_NetbufRead(&rv, sizeof(double), 1, buf);
	return ((SC_Real)rv);
}

void
SC_CopyReal(AG_Netbuf *buf, SC_Real *pv)
{
	double v;

	AG_NetbufRead(&v, sizeof(double), 1, buf);
	*pv = (SC_Real)v;
}

void
SC_WriteReal(AG_Netbuf *buf, SC_Real v)
{
	double rv = (double)v;

	AG_NetbufWrite(&rv, sizeof(double), 1, buf);
}

SC_Range
SC_ReadRange(AG_Netbuf *buf)
{
	SC_Range r;

	r.min = SC_ReadReal(buf);
	r.typ = SC_ReadReal(buf);
	r.max = SC_ReadReal(buf);
	return (r);
}

void
SC_CopyRange(AG_Netbuf *buf, SC_Range *r)
{
	r->min = SC_ReadReal(buf);
	r->typ = SC_ReadReal(buf);
	r->max = SC_ReadReal(buf);
}

void
SC_WriteRange(AG_Netbuf *buf, SC_Range r)
{
	SC_WriteReal(buf, r.min);
	SC_WriteReal(buf, r.typ);
	SC_WriteReal(buf, r.max);
}

SC_QTimeRange
SC_ReadQTimeRange(AG_Netbuf *buf)
{
	SC_QTimeRange r;

	r.min = SC_ReadQTime(buf);
	r.typ = SC_ReadQTime(buf);
	r.max = SC_ReadQTime(buf);
	return (r);
}

void
SC_CopyQTimeRange(AG_Netbuf *buf, SC_QTimeRange *r)
{
	r->min = SC_ReadQTime(buf);
	r->typ = SC_ReadQTime(buf);
	r->max = SC_ReadQTime(buf);
}

void
SC_WriteQTimeRange(AG_Netbuf *buf, SC_QTimeRange r)
{
	SC_WriteQTime(buf, r.min);
	SC_WriteQTime(buf, r.typ);
	SC_WriteQTime(buf, r.max);
}
