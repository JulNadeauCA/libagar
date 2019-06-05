#include <string.h>
#include <stdio.h>

/* based on x86info by Dave Jones but sharing no code */

#define CPU_VENDOR_UNKNOWN 0
#define CPU_VENDOR_AMD 1
#define CPU_VENDOR_CENTAUR 2
#define CPU_VENDOR_CYRIX 3
#define CPU_VENDOR_INTEL 4
#define CPU_VENDOR_NATSEMI 5
#define CPU_VENDOR_RISE 6
#define CPU_VENDOR_TRANSMETA 7
#define CPU_VENDOR_SIS 8

#define SYSDEP_CPU_EXT_MMX       0x0001
#define SYSDEP_CPU_EXT_MMX2      0x0002
#define SYSDEP_CPU_EXT_3DNOW     0x0004
#define SYSDEP_CPU_EXT_3DNOWEXT  0x0008
#define SYSDEP_CPU_EXT_SSE       0x0010
#define SYSDEP_CPU_EXT_SSE2      0x0020
#define SYSDEP_CPU_EXT_SSE3      0x0040
#define SYSDEP_CPU_EXT_SSE4      0x0080
#define SYSDEP_CPU_EXT_ALTIVEC   0x0100

struct cpu_info {
  unsigned int number;
  unsigned int vendor;
  unsigned int ext_family;
  unsigned int family;
  unsigned int model;
  unsigned int ext_model;
  unsigned int step;
  unsigned int type;
  unsigned int brand;
  unsigned int cache_l1i;
  unsigned int cache_l1d;
  unsigned int cache_l2;
  unsigned int cache_l3;
  unsigned int cacheline;
  unsigned int flags;
  unsigned int eflags;
  unsigned int bflags;
  unsigned int freq;
  unsigned int maxi1;
  unsigned int maxi2;
  unsigned int maxi3;
};
struct cachedesc {
  unsigned char val;
  unsigned int size;
  unsigned int line;
};
struct vendordesc {
  unsigned long id;
  unsigned int vendor;
  const char *name;
};

static struct cpu_info cpu;
static unsigned long eax;
static unsigned long ebx;
static unsigned long ecx;
static unsigned long edx;

static const struct cachedesc intel_l1i_caches[] = {
  { 0x6, 8, 32 },
  { 0x8, 16, 32 },
  { 0x15, 16, 32 },
  { 0x30, 32, 64 },
  { 0x77, 16, 64 },
  { 0, 0, 0 },
};
static const struct cachedesc intel_l1d_caches[] = {
  { 0xa, 8, 32 },
  { 0xc, 16, 32 },
  { 0x10, 16, 32 },
  { 0x2c, 32, 64 },
  { 0x60, 16, 64 },
  { 0x66, 8, 64 },
  { 0x67, 16, 64 },
  { 0x68, 32, 64 },
  { 0, 0, 0 },
};
static const struct cachedesc intel_l2_caches[] = {
  { 0x39, 128, 64 },
  { 0x3a, 192, 64 },
  { 0x3b, 128, 64 },
  { 0x3c, 256, 64 },
  { 0x3d, 384, 64 },
  { 0x3e, 512, 64 },
  { 0x41, 128, 32 },
  { 0x42, 256, 32 },
  { 0x43, 512, 32 },
  { 0x44, 1024, 32 },
  { 0x45, 2048, 32 },
  { 0x78, 1024, 64 },
  { 0x79, 128, 64 },
  { 0x7a, 256, 64 },
  { 0x7b, 512, 64 },
  { 0x7c, 1024, 64 },
  { 0x82, 256, 32 },
  { 0x83, 512, 32 },
  { 0x84, 1024, 32 },
  { 0x85, 2048, 32 },
  { 0x86, 512, 64 },
  { 0x87, 1024, 64 },
  { 0, 0, 0 },
};
static const struct cachedesc intel_l3_caches[] = {
  { 0x22, 512, 64 },
  { 0x23, 1024, 64 },
  { 0x25, 2048, 64 },
  { 0x29, 4096, 64 },
  { 0, 0, 0 },
};
static const struct vendordesc vendors[] = {
  { 0x756e6547, CPU_VENDOR_INTEL, "intel" },
  { 0x68747541, CPU_VENDOR_AMD, "amd", },
  { 0x69727943, CPU_VENDOR_CYRIX, "cyrix" },
  { 0x746e6543, CPU_VENDOR_CENTAUR, "centaur" },
  { 0x646f6547, CPU_VENDOR_NATSEMI, "national-semiconductor" },
  { 0x52697365, CPU_VENDOR_RISE, "rise" },
  { 0x65736952, CPU_VENDOR_RISE, "rise" },
  { 0x20536953, CPU_VENDOR_SIS, "sis" },
};

static int cpuid_sup()
{
  unsigned long eax = 0;
  unsigned long ecx = 0;

#ifdef __SUNPRO_C
  asm("pushf");
  asm("pop %eax");
  asm("movl %eax, -12(%ebp)");
  asm("xor $0x200000, %eax");
  asm("push %eax");
  asm("popf");
  asm("pushf");
  asm("pop %eax");
  asm("movl %eax, -8(%ebp)");
#endif

#ifdef __GNUC__
  __asm__ __volatile__(
    "pushf\n\t"
    "pop %0\n\t"
    "mov %0, %1\n\t"
    "xor $0x200000, %0\n\t"
    "push %0\n\t"
    "popf\n\t"
    "pushf\n\t"
    "pop %0\n\t"
    : "=a" (eax), "=c" (ecx)
    :
    : "cc" 
  );
#endif
  return ((eax ^ ecx) & 0x200000);
}
static void cpuid(unsigned long val, unsigned long *eax, unsigned long *ebx,
                                     unsigned long *ecx, unsigned long *edx)
{
#ifdef __SUNPRO_C
  asm("movl 8(%ebp), %eax");
  asm("cpuid");
  asm("movl 12(%ebp), %esp");
  asm("movl %eax, 0(%esp)");
  asm("movl 16(%ebp), %esp");
  asm("movl %ebx, 0(%esp)");
  asm("movl 20(%ebp), %esp");
  asm("movl %ecx, 0(%esp)");
  asm("movl 24(%ebp), %esp");
  asm("movl %edx, 0(%esp)");
#endif

#ifdef __GNUC__
  __asm __volatile__(
    "mov %%ebx, %%esi\n\t"
    "cpuid\n\t"
    "xchg %%ebx, %%esi"
    : "=a" (*eax), "=S" (*ebx), 
      "=c" (*ecx), "=d" (*edx)
    : "0" (val)
  );
#endif
}
static void cachesize(const struct cachedesc *ctab, unsigned long cpunum,
                      unsigned int *sz, unsigned int *ls)
{
  unsigned long regs[4];
  const struct cachedesc *cp;
  char *ptr;
  unsigned int ind;
  unsigned int max;
  unsigned int jnd;
  unsigned char ch;

  if (ctab) {
    cpuid(cpunum, &regs[0], &regs[1], &regs[2], &regs[3]);
    max = regs[0] & 0xff;
    ptr = (char *) regs;
    for (ind = 0; ind < max; ++ind) {
      cpuid(cpunum, &regs[0], &regs[1], &regs[2], &regs[3]);
      for (jnd = 0; jnd < 3; ++jnd)
        regs[jnd] = (regs[jnd] & 0x80000000) ? 0 : regs[jnd];  
      for (jnd = 1; jnd < 16; ++jnd) {
        ch = ptr[jnd];
        if (ch) {
          cp = ctab;
          while (cp->val) {
            if (cp->val == ch) {
              *sz = cp->size;
              *ls = cp->line;
            }
            ++cp;
          }
        }
      }
    }
  }
}
static void vendor_intel()
{
  cpuid(0x00000001, &eax, &ebx, &ecx, &edx);

  cpu.step = eax & 0xf;
  cpu.model = (eax >> 4) & 0xf;
  cpu.family = (eax >> 8) & 0xf;
  cpu.type = (eax >> 12) & 0x3;
  cpu.brand = ebx & 0xf;

  if (edx & 0x00800000) cpu.flags |= SYSDEP_CPU_EXT_MMX;
  if (edx & 0x02000000) cpu.flags |= SYSDEP_CPU_EXT_MMX2;
  if (edx & 0x02000000) cpu.flags |= SYSDEP_CPU_EXT_SSE;
  if (edx & 0x04000000) cpu.flags |= SYSDEP_CPU_EXT_SSE2;
  if (ecx & 0x00000001) cpu.flags |= SYSDEP_CPU_EXT_SSE3;

  cachesize(intel_l1i_caches, 0x00000002, &cpu.cache_l1i, &cpu.cacheline);
  cachesize(intel_l1d_caches, 0x00000002, &cpu.cache_l1d, &cpu.cacheline);
  cachesize(intel_l2_caches, 0x00000002, &cpu.cache_l2, &cpu.cacheline);
  cachesize(intel_l3_caches, 0x00000002, &cpu.cache_l3, &cpu.cacheline);
}
static void vendor_amd()
{
  cpuid(0x00000001, &eax, &ebx, &ecx, &edx);

  cpu.step = eax & 0xf;
  cpu.model = (eax >> 4) & 0xf;
  cpu.family = (eax >> 8) & 0xf;
  cpu.ext_model = (eax >> 16) & 0xff;
  cpu.ext_family = (eax >> 20) & 0xf;

  if (cpu.maxi2 >= 0x80000005) {
    cpuid(0x80000005, &eax, &ebx, &ecx, &edx);
    cpu.cache_l1d = ecx >> 24;
    cpu.cacheline = ecx & 0xff;
    cpu.cache_l1i = edx >> 24;
  }
  if (cpu.maxi2 >= 0x80000006) {
    cpuid(0x80000006, &eax, &ebx, &ecx, &edx);
    cpu.cache_l2 = ecx >> 24;
  }
}

int main(int argc, char *argv[])
{
  char *arg;
  unsigned int ind;

  if (argc < 2) return 111;
  if (!cpuid_sup()) return 112;

  arg = argv[1];

  cpuid(0x00000000, &eax, &ebx, &ecx, &edx);
  cpu.maxi1 = eax & 0xffff;

  cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
  cpu.maxi2 = eax;

  cpuid(0xC0000000, &eax, &ebx, &ecx, &edx);
  cpu.maxi3 = eax;

  cpuid(0x00000000, &eax, &ebx, &ecx, &edx);

  for (ind = 0; ind < sizeof(vendors) / sizeof(struct vendordesc); ++ind)
    if (ebx == vendors[ind].id) cpu.vendor = vendors[ind].vendor;

  switch (cpu.vendor) {
    case CPU_VENDOR_AMD: vendor_amd(); break;
    case CPU_VENDOR_CENTAUR: break;
    case CPU_VENDOR_CYRIX: break;
    case CPU_VENDOR_INTEL: vendor_intel(); break;
    case CPU_VENDOR_NATSEMI: break;
    case CPU_VENDOR_RISE: break;
    case CPU_VENDOR_TRANSMETA: break;
    case CPU_VENDOR_SIS: break;
    default: break;
  }

  cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
  if (eax >= 0x80000001) {
    cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    if (edx & 0x00800000) cpu.flags |= SYSDEP_CPU_EXT_MMX;
    if (edx & 0x00400000) cpu.flags |= SYSDEP_CPU_EXT_MMX2;
    if (edx & 0x80000000) cpu.flags |= SYSDEP_CPU_EXT_3DNOW;
    if (edx & 0x40000000) cpu.flags |= SYSDEP_CPU_EXT_3DNOWEXT;
  }

  if (strcmp(arg, "mmx") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_MMX));
  if (strcmp(arg, "mmx2") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_MMX2));
  if (strcmp(arg, "3dnow") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_3DNOW));
  if (strcmp(arg, "3dnowext") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_3DNOWEXT));
  if (strcmp(arg, "sse") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_SSE));
  if (strcmp(arg, "sse2") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_SSE2));
  if (strcmp(arg, "sse3") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_SSE3));
  if (strcmp(arg, "sse4") == 0)
    return !!printf("%u\n", !!(cpu.flags & SYSDEP_CPU_EXT_SSE4));
 
  if (strcmp(arg, "l1icachesize") == 0)
    return !!printf("%u\n", cpu.cache_l1i << 10);
  if (strcmp(arg, "l1dcachesize") == 0)
    return !!printf("%u\n", cpu.cache_l1d << 10);
  if (strcmp(arg, "l2cachesize") == 0)
    return !!printf("%u\n", cpu.cache_l2 << 10);
  if (strcmp(arg, "l3cachesize") == 0)
    return !!printf("%u\n", cpu.cache_l3 << 10);
  if (strcmp(arg, "cacheline") == 0)
    return !!printf("%u\n", cpu.cacheline);
 
  printf("0\n");
  return 0;
}
