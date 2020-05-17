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
 * Display and tweak CPU information.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/window.h>
#include <agar/gui/label.h>
#include <agar/gui/separator.h>
#include <agar/gui/tlist.h>
	
static AG_FlagDescr archExtns[] = {
	{ AG_EXT_CPUID,		"CPUID Instruction",			1 },
	{ AG_EXT_MMX,		"MMX",					1 },
	{ AG_EXT_MMX_EXT,	"MMX + AMD Extensions",			1 },
	{ AG_EXT_3DNOW,		"3dNow!",				1 },
	{ AG_EXT_3DNOW_EXT,	"3dNow! + Extensions",			1 },
	{ AG_EXT_3DNOW_PREFETCH,"3dNow! PREFETCH/PREFETCHW",		1 },
	{ AG_EXT_ALTIVEC,	"AltiVec",				1 },
	{ AG_EXT_SSE,		"SSE",					1 },
	{ AG_EXT_SSE2,		"SSE2",					1 },
	{ AG_EXT_SSE3,		"SSE3",					1 },
	{ AG_EXT_SSSE3,		"SSSE3",				1 },
	{ AG_EXT_SSE4A,		"SSE4a Extensions",			1 },
	{ AG_EXT_SSE41,		"SSE41",				1 },
	{ AG_EXT_SSE42,		"SSE42",				1 },
	{ AG_EXT_SSE5A,		"SSE5a Extensions",			1 },
	{ AG_EXT_SSE_MISALIGNED,"Misaligned SSE Mode",			1 },
	{ AG_EXT_LONG_MODE,	"Long Mode",				1 },
	{ AG_EXT_RDTSCP,	"RDTSCP Instruction",			1 },
	{ AG_EXT_FXSR,		"Fast FXSAVE/FXSTOR",			1 },
	{ AG_EXT_PAGE_NX,	"W^X Page Protection",			1 },
	{ AG_EXT_ONCHIP_FPU,	"On-chip FPU",				1 },
	{ AG_EXT_TSC,		"Time Stamp Counter (TSC)",		1 },
	{ AG_EXT_CMOV,		"Conditional Move (CMOV)",		1 },
	{ AG_EXT_CLFLUSH,	"Cache-Line Flush (CLFLUSH)",		1 },
	{ AG_EXT_HTT,		"Hyper-Threading Technology",		1 },
	{ AG_EXT_MON,		"MONITOR/MWAIT Instructions",		1 },
	{ AG_EXT_VMX,		"Virtual Machine Extensions",		1 },
	{ 0,			NULL,					0 }
};

AG_Window *
AG_DEV_CPUInfo(void)
{
	AG_Window *win;
	AG_FlagDescr *fd;
	AG_Tlist *tl;

	if ((win = AG_WindowNewNamedS(0, "DEV_CPUInfo")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("CPU Information"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	AG_LabelNew(win, 0, _("Architecture: %s"),
	    agCPU.arch[0] != '\0' ? agCPU.arch : "unknown");

	if (agCPU.vendorID[0] != '\0')
		AG_LabelNew(win, 0, _("Vendor ID: %s"), agCPU.vendorID);

	AG_SeparatorNewHoriz(win);

	AG_LabelNew(win, 0, _("Architecture Extensions:"));
	tl = AG_TlistNew(win, AG_TLIST_EXPAND);
	AG_TlistSizeHint(tl, "MONITOR/MWAIT Instructions  ", 10);
	for (fd = &archExtns[0]; fd->bitmask != 0; fd++) {
		if (agCPU.ext & fd->bitmask)
			AG_TlistAddS(tl, NULL, fd->descr);
	}
	return (win);
}

#endif /* AG_WIDGETS */
