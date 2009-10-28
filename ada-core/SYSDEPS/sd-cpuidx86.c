#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sd-cpuidx86.h"

/*
 * Based on x86info by Dave Jones but sharing no code.
 */

#define CPU_VENDOR_UNKNOWN      0
#define CPU_VENDOR_AMD          1
#define CPU_VENDOR_CENTAUR      2
#define CPU_VENDOR_CYRIX        3
#define CPU_VENDOR_INTEL        4
#define CPU_VENDOR_NATSEMI      5
#define CPU_VENDOR_RISE         6
#define CPU_VENDOR_TRANSMETA    7
#define CPU_VENDOR_SIS          8

/*
 * Intel specific feature bit masks.
 */

#define CPU_INTEL_MMX_BIT_MASK       0x00800000u
#define CPU_INTEL_MMX2_BIT_MASK      0x02000000u
#define CPU_INTEL_SSE_BIT_MASK       0x02000000u
#define CPU_INTEL_SSE2_BIT_MASK      0x04000000u
#define CPU_INTEL_SSE3_BIT_MASK      0x00000001u
#define CPU_INTEL_SSE4_1_BIT_MASK    0x00080000u
#define CPU_INTEL_SSE4_2_BIT_MASK    0x00100000u

/*
 * Values for EAX register to retrieve information.
 */

#define CPU_ID_GET_MAX_BASIC_PARAM   0x00000000u
#define CPU_ID_VENDOR_ID             CPU_ID_GET_MAX_BASIC_PARAM
#define CPU_ID_PROCESSOR_INFO        0x00000001u
#define CPU_ID_CACHE_TLB_INFO        0x00000002u
#define CPU_ID_SERIAL_NUMBER         0x00000003u
#define CPU_ID_GET_MAX_EXT_ALT_PARAM 0xC0000000u
#define CPU_ID_GET_MAX_EXT_PARAM     0x80000000u
#define CPU_ID_GET_EXT_INFO          0x80000001u
#define CPU_ID_BRAND_1               0x80000002u
#define CPU_ID_BRAND_2               0x80000003u
#define CPU_ID_BRAND_3               0x80000004u
#define CPU_ID_L1_CACHE_TLB          0x80000005u
#define CPU_ID_L2_CACHE              0x80000006u
#define CPU_ID_APM_INFO              0x80000007u
#define CPU_ID_ADDRESS_SIZE          0x80000008u

struct cpu_info {
  uint32_t number;
  uint32_t vendor;
  uint32_t ext_family;
  uint32_t family;
  uint32_t model;
  uint32_t ext_model;
  uint32_t step;
  uint32_t type;
  uint32_t brand;
  uint32_t cache_L1_instruction;
  uint32_t cache_L1_data;
  uint32_t cache_L2;
  uint32_t cache_L3;
  uint32_t cacheline;
  uint32_t flags;
  uint32_t eflags;
  uint32_t bflags;
  uint32_t freq;
  uint32_t max_basic_param;
  uint32_t max_extended_param;
  uint32_t max_alt_extended_param;
};
struct cache_description {
  unsigned char val;
  uint32_t size;
  uint32_t line;
};
struct vendor_description {
  uint32_t id;
  uint32_t vendor;
  const char *name;
};
struct feature {
  const char *name;
  uint32_t bit;
};

static struct cpu_info cpu;
static uint32_t eax;
static uint32_t ebx;
static uint32_t ecx;
static uint32_t edx;

/*
 * Dump current registers.
 */

static void
cpu_registers_local (uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
  (void) fprintf (stderr, "cpu_registers:             %.8lx %.8lx %.8lx %.8lx\n",
    (unsigned long) eax,
    (unsigned long) ebx,
    (unsigned long) ecx,
    (unsigned long) edx);
}

static void
cpu_registers (void)
{
  cpu_registers_local (eax, ebx, ecx, edx);
}

static const struct cache_description intel_L1i_caches[] = {
  { 0x0006u,  8, 32 },
  { 0x0008u, 16, 32 },
  { 0x0009u, 32, 64 },
  { 0x0015u, 16, 32 },
  { 0x0030u, 32, 64 },
  { 0x0077u, 16, 64 },
  {       0,  0,  0 },
};
static const struct cache_description intel_L1d_caches[] = {
  { 0x000au,  8, 32 },
  { 0x000cu, 16, 32 },
  { 0x000Du, 16, 64 },
  { 0x0010u, 16, 32 },
  { 0x002cu, 32, 64 },
  { 0x0060u, 16, 64 },
  { 0x0066u,  8, 64 },
  { 0x0067u, 16, 64 },
  { 0x0068u, 32, 64 },
  {       0,  0,  0 },
};
static const struct cache_description intel_L2_caches[] = {
  { 0x0021u,  256, 64 },
  { 0x0039u,  128, 64 },
  { 0x003au,  192, 64 },
  { 0x003bu,  128, 64 },
  { 0x003cu,  256, 64 },
  { 0x003du,  384, 64 },
  { 0x003eu,  512, 64 },
  { 0x0041u,  128, 32 },
  { 0x0042u,  256, 32 },
  { 0x0043u,  512, 32 },
  { 0x0044u, 1024, 32 },
  { 0x0045u, 2048, 32 },
  { 0x0048u, 3072, 64 },
  { 0x0049u, 4096, 64 },
  { 0x004Eu, 6144, 64 },
  { 0x0078u, 1024, 64 },
  { 0x0079u,  128, 64 },
  { 0x007au,  256, 64 },
  { 0x007bu,  512, 64 },
  { 0x007cu, 1024, 64 },
  { 0x007Du, 2048, 64 },
  { 0x007Fu,  512, 64 },
  { 0x0082u,  256, 32 },
  { 0x0083u,  512, 32 },
  { 0x0084u, 1024, 32 },
  { 0x0085u, 2048, 32 },
  { 0x0086u,  512, 64 },
  { 0x0087u, 1024, 64 },
  {       0,    0,  0 },
};
static const struct cache_description intel_L3_caches[] = {
  { 0x0022u,      512, 64 },
  { 0x0023u,     1024, 64 },
  { 0x0025u,     2048, 64 },
  { 0x0029u,     4096, 64 },
  { 0x00D0u,   524288, 64 },
  { 0x00D1u,  1048576, 64 },
  { 0x00D2u,  2097152, 64 },
  { 0x00D6u,  1048576, 64 },
  { 0x00D7u,  2097152, 64 },
  { 0x00D8u,  8388608, 64 },
  { 0x00DCu,  1572864, 64 },
  { 0x00DDu,  3145728, 64 },
  { 0x00DEu,  6291456, 64 },
  { 0x00E2u,  2097152, 64 },
  { 0x00E3u,  4194304, 64 },
  { 0x00E4u,  8388608, 64 },
  { 0x00EAu, 12582912, 64 },
  { 0x00EBu, 18874368, 64 },
  { 0x00ECu, 25165824, 64 },
  {       0,        0,  0 },
};
static const struct vendor_description vendors[] = {
  { 0x756e6547u, CPU_VENDOR_INTEL,   "intel" },
  { 0x68747541u, CPU_VENDOR_AMD,     "amd", },
  { 0x69727943u, CPU_VENDOR_CYRIX,   "cyrix" },
  { 0x746e6543u, CPU_VENDOR_CENTAUR, "centaur" },
  { 0x646f6547u, CPU_VENDOR_NATSEMI, "national-semiconductor" },
  { 0x52697365u, CPU_VENDOR_RISE,    "rise" },
  { 0x65736952u, CPU_VENDOR_RISE,    "rise" },
  { 0x20536953u, CPU_VENDOR_SIS,     "sis" },
};

/*
 * CPU features by name.
 */

static const struct feature features[] = {
  { "mmx",      SD_SYSINFO_CPU_EXT_MMX,     },
  { "mmx2",     SD_SYSINFO_CPU_EXT_MMX2,    },
  { "3dnow",    SD_SYSINFO_CPU_EXT_3DNOW    },
  { "3dnowext", SD_SYSINFO_CPU_EXT_3DNOWEXT },
  { "sse",      SD_SYSINFO_CPU_EXT_SSE,     },
  { "sse2",     SD_SYSINFO_CPU_EXT_SSE2,    },
  { "sse3",     SD_SYSINFO_CPU_EXT_SSE3,    },
  { "sse4.1",   SD_SYSINFO_CPU_EXT_SSE4_1,  },
  { "sse4.2",   SD_SYSINFO_CPU_EXT_SSE4_2,  },
};

static int
cpu_check_features (const char *name)
{
  unsigned int index;

  for (index = 0; index < sizeof (features) / sizeof (features[0]); ++index) {
    if (strcmp (features [index].name, name) == 0) {
      if ((cpu.flags & features [index].bit) != 0) {
        (void) printf ("1\n");
        return 1;
      } else {
        (void) printf ("0\n");
        return 1;
      }
    }
  }

  /* Feature name not known. */
  return 0;
}

/*
 * Check CPUID support.
 */

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86_64
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
static int
cpuid_check_support_x86_64_gcc (void)
{
  /* Assume all 64 bit CPUs support CPUID. */
  return 1;
}
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#endif /* SD_SYSINFO_ARCH_X86_64 */

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
static int
cpuid_check_support_x86_gcc (void)
{
  uint32_t eax = 0;
  uint32_t ecx = 0;

  __asm__ __volatile__(
    /* Place copies of EFLAGS into eax, ecx. */
    "pushf\n\t"
    "popl %0\n\t"
    "mov %0, %1\n\t"

    /* Toggle ID bit in eax and store to EFLAGS register. */
    "xor $0x200000, %0\n\t"
    "push %0\n\t"
    "popf\n\t"

    /* Get EFLAGS */
    "pushf\n\t"
    "pop %0\n\t"
    : "=a" (eax), "=c" (ecx)
    :
    : "cc" 
  );

  return eax != ecx;
}
#endif /* SD_SYSINFO_CC_TYPE_GCC */

#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_SUN_C
static int
cpuid_check_support_x86_sun (void)
{
  uint32_t eax = 0;
  uint32_t ecx = 0;

  asm("pushf");
  asm("pop %eax");
  asm("movl %eax, -12(%ebp)");
  asm("xor $0x200000, %eax");
  asm("push %eax");
  asm("popf");
  asm("pushf");
  asm("pop %eax");
  asm("movl %eax, -8(%ebp)");

  return eax != ecx;
}
#endif /* SD_SYSINFO_CC_TYPE_SUN_C */
#endif /* SD_SYSINFO_ARCH_X86 */

static int
cpuid_check_support (void)
{
  int supported = 0;

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86_64
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
  supported = cpuid_check_support_x86_64_gcc ();
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#endif /* SD_SYSINFO_ARCH_X86 */

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
  supported = cpuid_check_support_x86_gcc ();
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_SUN_C
  supported = cpuid_check_support_x86_sun ();
#endif /* SD_SYSINFO_CC_TYPE_SUN_C */
#endif /* SD_SYSINFO_ARCH_X86 */

  if (supported == 0) {
    (void) fprintf (stderr, "cpuid_check:      cpuid not supported\n");
    return 0;
  } else {
    (void) fprintf (stderr, "cpuid_check:      cpuid supported\n");
    return 1;
  }
}

/*
 * Execute CPUID instruction.
 */

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86_64
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
static void
cpuid_exec_gcc_x86_64 (uint32_t val,  uint32_t *eax, uint32_t *ebx,
                       uint32_t *ecx, uint32_t *edx)
{
  __asm __volatile__(
    "mov %%rbx, %%rsi\n\t"
    "cpuid\n\t"
    "xchg %%rbx, %%rsi"
    : "=a" (*eax), "=S" (*ebx), 
      "=c" (*ecx), "=d" (*edx)
    : "0" (val)
  );
}
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#endif /* SD_SYSINFO_ARCH_X86_64 */

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
static void
cpuid_exec_gcc_x86 (uint32_t val,  uint32_t *eax, uint32_t *ebx,
                    uint32_t *ecx, uint32_t *edx)
{
  __asm __volatile__(
    "mov %%ebx, %%esi\n\t"
    "cpuid\n\t"
    "xchg %%ebx, %%esi"
    : "=a" (*eax), "=S" (*ebx), 
      "=c" (*ecx), "=d" (*edx)
    : "0" (val)
  );
}
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_SUN_C
static void
cpuid_exec_x86_sun (uint32_t val,  uint32_t *eax, uint32_t *ebx,
                    uint32_t *ecx, uint32_t *edx)
{
  asm ("movl 8(%ebp), %eax");
  asm ("cpuid");
  asm ("movl 12(%ebp), %esp");
  asm ("movl %eax, 0(%esp)");
  asm ("movl 16(%ebp), %esp");
  asm ("movl %ebx, 0(%esp)");
  asm ("movl 20(%ebp), %esp");
  asm ("movl %ecx, 0(%esp)");
  asm ("movl 24(%ebp), %esp");
  asm ("movl %edx, 0(%esp)");
}
#endif /* SD_SYSINFO_CC_TYPE_SUN_C */
#endif /* SD_SYSINFO_ARCH_X86 */

static void
cpuid_exec (uint32_t val,  uint32_t *eax, uint32_t *ebx,
            uint32_t *ecx, uint32_t *edx)
{
#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86_64
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
  cpuid_exec_gcc_x86_64 (val, eax, ebx, ecx, edx);
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#endif /* SD_SYSINFO_ARCH_X86_64 */

#if SD_CPUIDX86_ARCH == SD_SYSINFO_ARCH_X86
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_GCC || SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_MINGW32
  cpuid_exec_gcc_x86 (val, eax, ebx, ecx, edx);
#endif /* SD_SYSINFO_CC_TYPE_GCC */
#if SD_CPUIDX86_CC_TYPE == SD_SYSINFO_CC_TYPE_SUN_C
  cpuid_exec_x86_sun (val, eax, ebx, ecx, edx);
#endif /* SD_SYSINFO_CC_TYPE_SUN_C */
#endif /* SD_SYSINFO_ARCH_X86 */
}

static void
cpuid (uint32_t val,  uint32_t *eax, uint32_t *ebx,
       uint32_t *ecx, uint32_t *edx)
{
  (void) fprintf (stderr, "cpuid_pre:        %.8lx %.8lx %.8lx %.8lx %.8lx\n",
    (unsigned long) val,
    (unsigned long) *eax,
    (unsigned long) *ebx,
    (unsigned long) *ecx,
    (unsigned long) *edx);

  cpuid_exec (val, eax, ebx, ecx, edx);

  (void) fprintf (stderr, "cpuid_post:       %.8lx %.8lx %.8lx %.8lx %.8lx\n",
    (unsigned long) val,
    (unsigned long) *eax,
    (unsigned long) *ebx,
    (unsigned long) *ecx,
    (unsigned long) *edx);
}

/*
 * Dump four byte segment of CPU vendor ID.
 */

static void
cpu_dump_id_segment (unsigned char reg[4])
{
  (void) fprintf (stderr, "%c%c%c%c",
    reg [0],
    reg [1],
    reg [2],
    reg [3]);
}

/*
 * Show vendor ID.
 */

static void
cpu_dump_vendor (void)
{
  (void) fprintf (stderr, "cpu_dump_vendor: ");
  cpu_dump_id_segment ((unsigned char *) &ebx);
  cpu_dump_id_segment ((unsigned char *) &edx);
  cpu_dump_id_segment ((unsigned char *) &ecx);
  (void) fprintf (stderr, "\n");
}

/*
 * Detect CPU vendor
 */

static void
cpu_detect_vendor (void)
{
  unsigned int index;

  (void) fprintf (stderr, "cpu_detect_vendor: checking vendor ID\n");

  cpuid (CPU_ID_VENDOR_ID, &eax, &ebx, &ecx, &edx);
  cpu_dump_vendor ();

  /* Select vendor. */
  for (index = 0; index < sizeof (vendors) / sizeof (struct vendor_description); ++index) {
    if (ebx == vendors [index].id) {
      cpu.vendor = vendors [index].vendor;
    }
  }
}

/*
 * Check cache list for description descriptor_id.
 */

static int
cpu_check_cache_list (const struct cache_description *cache_list,
  uint8_t descriptor_id, uint32_t *cache_size, uint32_t *cache_line)
{
  const struct cache_description *cdesc;

  /* If descriptor_id is nonzero, search cache list for matching value. */
  if (descriptor_id) {
    cdesc = cache_list;
    while (cdesc->val) {
      if (cdesc->val == descriptor_id) {
        *cache_size = cdesc->size;
        *cache_line = cdesc->line;
        return 1;
      }
      ++cdesc;
    }
  }
  return 0;
}

/*
 * Unpack register and check each byte for a matching cache description.
 */

static int
cpu_check_cache_list_by_register (const struct cache_description *cache_list,
  uint32_t reg_value, uint32_t *cache_size, uint32_t *cache_line)
{
  if (cpu_check_cache_list (cache_list, (reg_value >> 24) & 0xffu, cache_size, cache_line)) return 1;
  if (cpu_check_cache_list (cache_list, (reg_value >> 16) & 0xffu, cache_size, cache_line)) return 1;
  if (cpu_check_cache_list (cache_list, (reg_value >> 8)  & 0xffu, cache_size, cache_line)) return 1;
  if (cpu_check_cache_list (cache_list,  reg_value        & 0xffu, cache_size, cache_line)) return 1;

  return 0;
}

/*
 * Determine cache sizes.
 */

static void
cpu_cache_size (const struct cache_description *ctab,
  uint32_t *cache_size, uint32_t *cache_line)
{
  uint32_t local_eax;
  uint32_t local_ebx;
  uint32_t local_ecx;
  uint32_t local_edx;
  unsigned int index;
  unsigned int max;

  (void) fprintf (stderr, "cpu_cache_size: %p\n", (void *) ctab);

  if (ctab) {
    cpuid (CPU_ID_CACHE_TLB_INFO, &local_eax, &local_ebx, &local_ecx, &local_edx);

    max = local_eax & 0xff;

    for (index = 0; index < max; ++index) {
      cpuid (CPU_ID_CACHE_TLB_INFO, &local_eax, &local_ebx, &local_ecx, &local_edx);

      local_eax = (local_eax & CPU_ID_GET_MAX_EXT_PARAM) ? 0 : local_eax;
      local_ebx = (local_ebx & CPU_ID_GET_MAX_EXT_PARAM) ? 0 : local_ebx;
      local_ecx = (local_ecx & CPU_ID_GET_MAX_EXT_PARAM) ? 0 : local_ecx;
      local_edx = (local_edx & CPU_ID_GET_MAX_EXT_PARAM) ? 0 : local_edx;

      if (cpu_check_cache_list_by_register (ctab, local_eax, cache_size, cache_line)) return;
      if (cpu_check_cache_list_by_register (ctab, local_ebx, cache_size, cache_line)) return;
      if (cpu_check_cache_list_by_register (ctab, local_ecx, cache_size, cache_line)) return;
      if (cpu_check_cache_list_by_register (ctab, local_edx, cache_size, cache_line)) return;
    }
  }
}

/*
 * Intel-specific CPU.
 */

static void
cpu_vendor_intel (void)
{
  (void) fprintf (stderr, "cpu_vendor_intel: Intel cpu\n");

  cpuid (CPU_ID_PROCESSOR_INFO, &eax, &ebx, &ecx, &edx);

  cpu.step   =  eax        & 0x000fu;
  cpu.model  = (eax >> 4)  & 0x000fu;
  cpu.family = (eax >> 8)  & 0x000fu;
  cpu.type   = (eax >> 12) & 0x0003u;
  cpu.brand  =  ebx        & 0x000fu;

  if (edx & CPU_INTEL_MMX_BIT_MASK)    cpu.flags |= SD_SYSINFO_CPU_EXT_MMX;
  if (edx & CPU_INTEL_MMX2_BIT_MASK)   cpu.flags |= SD_SYSINFO_CPU_EXT_MMX2;
  if (edx & CPU_INTEL_SSE_BIT_MASK)    cpu.flags |= SD_SYSINFO_CPU_EXT_SSE;
  if (edx & CPU_INTEL_SSE2_BIT_MASK)   cpu.flags |= SD_SYSINFO_CPU_EXT_SSE2;
  if (ecx & CPU_INTEL_SSE3_BIT_MASK)   cpu.flags |= SD_SYSINFO_CPU_EXT_SSE3;
  if (ecx & CPU_INTEL_SSE4_1_BIT_MASK) cpu.flags |= SD_SYSINFO_CPU_EXT_SSE4_1;
  if (ecx & CPU_INTEL_SSE4_2_BIT_MASK) cpu.flags |= SD_SYSINFO_CPU_EXT_SSE4_2;

  cpu_cache_size (intel_L1i_caches, &cpu.cache_L1_instruction, &cpu.cacheline);
  cpu_cache_size (intel_L1d_caches, &cpu.cache_L1_data, &cpu.cacheline);
  cpu_cache_size (intel_L2_caches,  &cpu.cache_L2, &cpu.cacheline);
  cpu_cache_size (intel_L3_caches,  &cpu.cache_L3, &cpu.cacheline);
}

/*
 * AMD-specific CPU.
 */

static void
cpu_vendor_amd (void)
{
  (void) fprintf (stderr, "cpu_vendor_amd: AMD cpu\n");

  cpuid (CPU_ID_PROCESSOR_INFO, &eax, &ebx, &ecx, &edx);

  cpu.step       =  eax        & 0x000fu;
  cpu.model      = (eax >> 4)  & 0x000fu;
  cpu.family     = (eax >> 8)  & 0x000fu;
  cpu.ext_model  = (eax >> 16) & 0x00ffu;
  cpu.ext_family = (eax >> 20) & 0x000fu;

  /* Check for L1 cache information, if supported. */
  if (cpu.max_extended_param >= CPU_ID_L1_CACHE_TLB) {
    cpuid (CPU_ID_L1_CACHE_TLB, &eax, &ebx, &ecx, &edx);
    cpu.cache_L1_data = ecx >> 24;
    cpu.cacheline = ecx & 0xff;
    cpu.cache_L1_instruction = edx >> 24;
  }

  /* Check for L2 cache information, if supported. */
  if (cpu.max_extended_param >= CPU_ID_L2_CACHE) {
    cpuid (CPU_ID_L2_CACHE, &eax, &ebx, &ecx, &edx);
    cpu.cache_L2 = ecx >> 24;
  }
}

int
main (int argc, char *argv[])
{
  if (argc < 2) exit (EXIT_FAILURE);

  if (!cpuid_check_support()) {
    (void) printf ("0\n");
    exit (EXIT_SUCCESS);
  }
  cpu_registers ();

  /* Check highest basic calling parameter. */
  cpuid (CPU_ID_GET_MAX_BASIC_PARAM, &eax, &ebx, &ecx, &edx);
  cpu_registers ();
  cpu.max_basic_param = eax & 0xffff;

  /* Determine highest extended calling parameter. */
  cpuid (CPU_ID_GET_MAX_EXT_PARAM, &eax, &ebx, &ecx, &edx);
  cpu_registers ();
  cpu.max_extended_param = eax;

  /* Check VIA/Centaur extended calling parameter. */
  cpuid (CPU_ID_GET_MAX_EXT_ALT_PARAM, &eax, &ebx, &ecx, &edx);
  cpu_registers ();
  cpu.max_alt_extended_param = eax;

  /* Detect CPU vendor */
  cpu_detect_vendor ();

  /* Vendor-specific handling. */
  switch (cpu.vendor) {
    case CPU_VENDOR_AMD:       cpu_vendor_amd(); break;
    case CPU_VENDOR_CENTAUR:   break;
    case CPU_VENDOR_CYRIX:     break;
    case CPU_VENDOR_INTEL:     cpu_vendor_intel(); break;
    case CPU_VENDOR_NATSEMI:   break;
    case CPU_VENDOR_RISE:      break;
    case CPU_VENDOR_TRANSMETA: break;
    case CPU_VENDOR_SIS:       break;
    default:                   break;
  }

  /* Common features. */
  cpuid (CPU_ID_GET_MAX_EXT_PARAM, &eax, &ebx, &ecx, &edx);
  if (eax >= 0x80000001u) {
    cpuid (0x80000001, &eax, &ebx, &ecx, &edx);
    if (edx & 0x00800000u) cpu.flags |= SD_SYSINFO_CPU_EXT_MMX;
    if (edx & 0x00400000u) cpu.flags |= SD_SYSINFO_CPU_EXT_MMX2;
    if (edx & 0x80000000u) cpu.flags |= SD_SYSINFO_CPU_EXT_3DNOW;
    if (edx & 0x40000000u) cpu.flags |= SD_SYSINFO_CPU_EXT_3DNOWEXT;
  }

  /* Exit if name was consumed. */
  if (cpu_check_features (argv[1])) exit (EXIT_SUCCESS);

  /* Various cache sizes. */
  if (strcmp (argv[1], "l1icachesize") == 0)
    return !!printf("%lu\n", (unsigned long) cpu.cache_L1_instruction << 10);
  if (strcmp (argv[1], "l1dcachesize") == 0)
    return !!printf("%lu\n", (unsigned long) cpu.cache_L1_data << 10);
  if (strcmp (argv[1], "l2cachesize") == 0)
    return !!printf("%lu\n", (unsigned long) cpu.cache_L2 << 10);
  if (strcmp (argv[1], "l3cachesize") == 0)
    return !!printf("%lu\n", (unsigned long) cpu.cache_L3 << 10);
  if (strcmp (argv[1], "cacheline") == 0)
    return !!printf("%lu\n", (unsigned long) cpu.cacheline);
 
  printf("0\n");
  return 0;
}
