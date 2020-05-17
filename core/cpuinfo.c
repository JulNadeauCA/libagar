/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Obtain information about architecture extensions.
 */

#include <agar/config/have_altivec.h>
#include <agar/config/_mk_have_signal.h>
#include <agar/config/_mk_have_setjmp.h>

#include <agar/core/core.h>

#if defined(__APPLE__) || defined(__MACOSX__)
# include <AvailabilityMacros.h>
# if defined(__ppc__) && !defined(MAC_OS_X_VERSION_10_4)
#  include <sys/sysctl.h>
# endif
#elif defined(__AMIGAOS4__)
# include <exec/exec.h>
# include <interfaces/exec.h>
# include <proto/exec.h>
#endif

#if !defined(__APPLE__) && !defined(__MACOSX__) && !defined(__ppc__) && \
     defined(HAVE_ALTIVEC) && defined(_MK_HAVE_SIGNAL) && defined(_MK_HAVE_SETJMP)
# include <signal.h>
# include <setjmp.h>
static jmp_buf jmpbuf;
#endif

struct cpuid_regs {
	Uint32 a;
	Uint32 b;
	Uint32 c;
	Uint32 d;
} _Packed_Attribute;

AG_CPUInfo agCPU;

#if defined(__GNUC__) && (defined(__i386__) || defined(i386) || defined(__x86_64__))
static int /* _Pure_Attribute */
X86_HaveCPUID(void)
{
	int rv = 0;

#if defined(__i386__) || defined(i386)
	__asm(
		"pushfl			\n"
		"popl	%%eax		\n"
		"movl	%%eax, %%ecx	\n"
		"xorl	$0x200000,%%eax	\n"		/* ID Bit */
		"pushl	%%eax		\n"
		"popfl			\n"
		"pushfl			\n"
		"popl	%%eax		\n"
		"xorl	%%ecx, %%eax	\n"
		"jz 1f			\n"
		"movl $1,%0		\n"
		"1:			\n"
		: "=m" (rv)
		:
		: "%eax", "%ecx");
#elif defined(__x86_64__)
	__asm(
		"pushfq			\n"
		"popq	%%rax		\n"
		"movq	%%rax, %%rcx	\n"
		"xorl	$0x200000,%%eax	\n"		/* ID Bit */
		"pushq	%%rax		\n"
		"popfq			\n"
		"pushfq			\n"
		"popq	%%rax		\n"
		"xorl	%%ecx, %%eax	\n"
		"jz 1f			\n"
		"movl $1,%0		\n"
		"1:			\n"
		: "=m" (rv)
		:
		: "%rax", "%rcx");
#endif
	return (rv);
}

static struct cpuid_regs /* _Pure_Attribute */
X86_GetCPUID(int fn)
{
	struct cpuid_regs regs;

#if defined(__i386__) || defined(i386)
	__asm(
		"mov %%ebx, %%esi\n"
		".byte 0x0f, 0xa2\n"
		"xchg %%esi, %%ebx\n"
		: "=a" (regs.a), "=S" (regs.b), "=c" (regs.c), "=d" (regs.d)
		: "0" (fn));

#elif defined(__x86_64__)
	__asm(
		"mov %%rbx, %%rsi\n"
		".byte 0x0f, 0xa2\n"
		"xchg %%rsi, %%rbx\n"
		: "=a" (regs.a), "=S" (regs.b), "=c" (regs.c), "=d" (regs.d)
		: "0" (fn));
#endif
	return (regs);
}
#endif /* __GNUC__ && (__i386__ || __x86_64__) */

#if defined(__i386__) || defined(i386) || defined(__x86_64__)
static __inline__ void
Conv32_x86(char *_Nonnull d, unsigned int v)
{
	d[0] =  v        & 0xff;
	d[1] = (v >>  8) & 0xff;
	d[2] = (v >> 16) & 0xff;
	d[3] = (v >> 24) & 0xff;
}
#endif /* x86 */

#if !defined(__APPLE__) && !defined(__MACOSX__) && !defined(__ppc__) && \
     defined(HAVE_ALTIVEC) && defined(_MK_HAVE_SIGNAL) && defined(_MK_HAVE_SETJMP)
static void
IllegalInsn_Altivec(int sig)
{
    longjmp(jmpbuf, 1);
}
#endif /* Altivec */

/* Initialize the CPUInfo structure. */
void
AG_GetCPUInfo(AG_CPUInfo *_Nonnull cpu)
{
#if defined(__i386__) || defined(i386) || defined(__x86_64__)
	struct cpuid_regs r, rExt;
	Uint maxFns, maxExt;
#endif

	cpu->vendorID[0] = '\0';
	cpu->ext = 0;

#if defined(__CC65__)
	cpu->arch = "6502";		/* Use getcpu() in <6502.h> */
#elif defined(__alpha__)
	cpu->arch = "alpha";
#elif defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
	cpu->arch = "amd64";
#elif defined(__arm__) || defined(__arm32__)
	cpu->arch = "arm";
#elif defined(__c64__)
	cpu->arch = "c64";
#elif defined(__c128__)
	cpu->arch = "c128";
#elif defined(__hppa64__)
	cpu->arch = "hppa64";
#elif defined(__hppa__)
	cpu->arch = "hppa";
#elif defined(__i386__) || defined(i386) || defined(_M_IX86)
	cpu->arch = "i386";
#elif defined(__ia64__) || defined(ia64)
	cpu->arch = "itanium";
#elif defined(__m68010__)
	cpu->arch = "m68010";
#elif defined(__m68k__)
	cpu->arch = "m68k";
#elif defined(__mips64__)
	cpu->arch = "mips64";
#elif defined(__mips__)
	cpu->arch = "mips";
#elif defined(__nes__)
	cpu->arch = "nes";
#elif defined(__ns32k__)
	cpu->arch = "ns32k";
#elif defined(__ppc__) || defined(__macppc__) || defined(__powerpc__)
	cpu->arch = "powerpc";
#elif defined(__sh3__)
	cpu->arch = "sh3";
#elif defined(__sparc64__)
	cpu->arch = "sparc64";
#elif defined(__sparc__)
	cpu->arch = "sparc";
#elif defined(__vax__)
	cpu->arch = "vax";
#else
	cpu->arch = "unknown";
#endif

#if defined(__i386__) || defined(i386) || defined(__x86_64__)
	if (X86_HaveCPUID() == 0) {
		return;
	}
	cpu->ext |= AG_EXT_CPUID;

	/* Standard Level 0 */
	r = X86_GetCPUID(0x00000000);
	maxFns = (Uint)r.a;		/* Maximum supported standard level */
	Conv32_x86(&cpu->vendorID[0], r.b);
	Conv32_x86(&cpu->vendorID[4], r.d);
	Conv32_x86(&cpu->vendorID[8], r.c);
	cpu->vendorID[12] = '\0';

	/* Extended Level */
	rExt = X86_GetCPUID(0x80000000);
	maxExt = rExt.a;
	if (maxExt >= 0x80000001) {
		rExt = X86_GetCPUID(0x80000001);
		if (rExt.d & 0x80000000) cpu->ext |= AG_EXT_3DNOW;
		if (rExt.d & 0x40000000) cpu->ext |= AG_EXT_3DNOW_EXT;
		if (rExt.d & 0x20000000) cpu->ext |= AG_EXT_LONG_MODE;
		if (rExt.d & 0x08000000) cpu->ext |= AG_EXT_RDTSCP;
		if (rExt.d & 0x02000000) cpu->ext |= AG_EXT_FXSR;
		if (rExt.d & 0x00400000) cpu->ext |= AG_EXT_MMX_EXT;
		if (rExt.d & 0x00100000) cpu->ext |= AG_EXT_PAGE_NX;

		if (rExt.c & 0x00000800) cpu->ext |= AG_EXT_SSE5A;
		if (rExt.c & 0x00000100) cpu->ext |= AG_EXT_3DNOW_PREFETCH;
		if (rExt.c & 0x00000080) cpu->ext |= AG_EXT_SSE_MISALIGNED;
		if (rExt.c & 0x00000040) cpu->ext |= AG_EXT_SSE4A;
	}
	if (maxFns >= 1) {
		rExt = X86_GetCPUID(1);
		if (rExt.d & 0x00000001) cpu->ext |= AG_EXT_ONCHIP_FPU;
		if (rExt.d & 0x00000010) cpu->ext |= AG_EXT_TSC;
		if (rExt.d & 0x00008000) cpu->ext |= AG_EXT_CMOV;
		if (rExt.d & 0x00080000) cpu->ext |= AG_EXT_CLFLUSH;
		if (rExt.d & 0x00800000) cpu->ext |= AG_EXT_MMX;
		if (rExt.d & 0x01000000) cpu->ext |= AG_EXT_FXSR;
		if (rExt.d & 0x02000000) cpu->ext |= AG_EXT_SSE;
		if (rExt.d & 0x04000000) cpu->ext |= AG_EXT_SSE2;
		if (rExt.d & 0x10000000) cpu->ext |= AG_EXT_HTT;

		if (rExt.c & 0x00000001) cpu->ext |= AG_EXT_SSE3;
		if (rExt.c & 0x00000008) cpu->ext |= AG_EXT_MON;
		if (rExt.c & 0x00000020) cpu->ext |= AG_EXT_VMX;
		if (rExt.c & 0x00000200) cpu->ext |= AG_EXT_SSSE3;
		if (rExt.c & 0x00080000) cpu->ext |= AG_EXT_SSE41;
		if (rExt.c & 0x00100000) cpu->ext |= AG_EXT_SSE42;
	}
#endif /* i386 or x86_64 */

#if (defined(__APPLE__) || defined(__MACOSX__)) && defined(__ppc__) && \
    !defined(MAC_OS_X_VERSION_10_4)
	{
		int selectors[2] = { CTL_HW, HW_VECTORUNIT };
		int flag = 0;
		size_t length = sizeof(flag);
	
		if (sysctl(selectors, 2, &flag, &length, NULL, 0) == 0) {
			if (flag != 0)
				cpu->ext |= AG_EXT_ALTIVEC;
		}
	}
	
#elif (defined(__APPLE__) || defined(__MACOSX__)) && defined(__ppc__) && \
       defined(MAC_OS_X_VERSION_10_4)
	{
		/* XXX sysctl.h issues */
		cpu->ext |= AG_EXT_ALTIVEC;
	}
    
#elif defined(__AMIGAOS4__)
	{
    		extern struct ExecIFace *IExec;
    		Ulong rv = 0;

    		IExec->GetCPUInfoTags(GCIT_VectorUnit, &rv, TAG_DONE);
    		if (rv == VECTORTYPE_ALTIVEC)
			cpu->ext |= AG_EXT_ALTIVEC;
	}
#elif defined(HAVE_ALTIVEC) && defined(_MK_HAVE_SIGNAL) && defined(_MK_HAVE_SETJMP)
	{
		volatile int hasAltiVec = 0;
		void (*fn)(int);

		fn = signal(SIGILL, IllegalInsn_Altivec);
		if (setjmp(jmpbuf) == 0) {
			__asm volatile (
				"mtspr 256, %0	\n"
				"vand %%v0, %%v0, %%v0\n"
				:
				: "r" (-1));
			hasAltiVec = 1;
		}
		signal(SIGILL, fn);
		if (hasAltiVec)
			cpu->ext |= AG_EXT_ALTIVEC;
	}
#endif /* HAVE_ALTIVEC and _MK_HAVE_SIGNAL and _MK_HAVE_SETJMP */
}
