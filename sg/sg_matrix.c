/*	$Csoft$	*/

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

#include <core/core.h>
#include "sg.h"

void
SG_LoadIdentity3(SG_Matrix3 *A)
{
	A->m[0] = 1.0;
	A->m[1] = 0.0;
	A->m[2] = 0.0;
	A->m[3] = 0.0;
	A->m[4] = 1.0;
	A->m[5] = 0.0;
	A->m[6] = 0.0;
	A->m[7] = 0.0;
	A->m[8] = 1.0;
}

void
SG_LoadIdentity4(SG_Matrix4 *A)
{
	A->m[0] = 1.0;
	A->m[1] = 0.0;
	A->m[2] = 0.0;
	A->m[3] = 0.0;
	A->m[4] = 0.0;
	A->m[5] = 1.0;
	A->m[6] = 0.0;
	A->m[7] = 0.0;
	A->m[8] = 0.0;
	A->m[9] = 0.0;
	A->m[10] = 1.0;
	A->m[11] = 0.0;
	A->m[12] = 0.0;
	A->m[13] = 0.0;
	A->m[14] = 0.0;
	A->m[15] = 1.0;
}
