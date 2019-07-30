/*	Public domain	*/

#ifndef _AGAR_CORE_CPUINFO_H_
#define _AGAR_CORE_CPUINFO_H_
#include <agar/core/begin.h>

typedef struct ag_cpuinfo {
	const char *_Nonnull arch;	/* Architecture name */
	char vendorID[13];		/* CPU Vendor ID string */
	char     _pad1[3];

	Uint32 ext;			/* Architecture extensions
					   (relevant to user-mode) */
#define AG_EXT_CPUID		0x00000001 /* CPUID Instruction */
#define AG_EXT_MMX		0x00000002 /* MMX Technology */
#define AG_EXT_MMX_EXT		0x00000004 /* MMX with AMD Extensions */
#define AG_EXT_3DNOW		0x00000008 /* 3dNow! */
#define AG_EXT_3DNOW_EXT	0x00000010 /* 3dNow! + Extensions */
#define AG_EXT_ALTIVEC		0x00000020 /* AltiVec Extensions */
#define AG_EXT_SSE		0x00000040 /* Streaming SIMD Extensions */
#define AG_EXT_SSE2		0x00000080 /* SSE2 Extensions */
#define AG_EXT_SSE3		0x00000100 /* SSE3 Extensions (PNI) */
#define AG_EXT_LONG_MODE	0x00000200 /* Long Mode */
#define AG_EXT_RDTSCP		0x00000400 /* RDTSCP instruction */
#define AG_EXT_FXSR		0x00000800 /* Fast FXSAVE/FXSTOR */
#define AG_EXT_PAGE_NX		0x00001000 /* W^X Page Protection */
#define AG_EXT_SSE5A		0x00002000 /* SSE5A Extensions */
#define AG_EXT_3DNOW_PREFETCH	0x00004000 /* PREFETCH/PREFETCHW for 3DNow! */
#define AG_EXT_SSE_MISALIGNED	0x00008000 /* Misaligned SSE mode */
#define AG_EXT_SSE4A		0x00010000 /* SSE4A Extensions */
#define AG_EXT_ONCHIP_FPU	0x00020000 /* On-chip FPU */
#define AG_EXT_TSC		0x00040000 /* Time Stamp Counter */
#define AG_EXT_CMOV		0x00080000 /* Conditional Move instruction */
#define AG_EXT_CLFLUSH		0x00100000 /* Cache-Line Flush instruction */
#define AG_EXT_HTT		0x00200000 /* Hyper-Threading technology */
#define AG_EXT_MON		0x00400000 /* MONITOR/MWAIT instructions */
#define AG_EXT_VMX		0x00800000 /* Virtual Machine extensions */
#define AG_EXT_SSSE3		0x01000000 /* SSSE3 Extensions */
#define AG_EXT_SSE41		0x02000000 /* SSE4.1 extensions */
#define AG_EXT_SSE42		0x04000000 /* SSE4.1 extensions */

	Uint32 _pad2;
} AG_CPUInfo;

__BEGIN_DECLS
extern AG_CPUInfo agCPU;
void AG_GetCPUInfo(AG_CPUInfo *_Nonnull);
__END_DECLS

#include <agar/core/close.h>
#endif	/* _AGAR_CORE_CPUINFO_H_ */
