#include <stdio.h>

const char *var = "UNKNOWN";

int main()
{
#if defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
  var = "ALPHA";
#endif
#if defined(__arm__) || defined(__thumb__) || defined(__TARGET_ARCH___TARGET_ARCH_THUMB)
  var = "ARM";
#endif
#if defined(_CRAY)
  var = "CRAY";
#endif
#if defined(_CRAY2)
  var = "CRAY_2";
#endif
#if defined(_CRAY1)
  var = "CRAY_X_MP";
#endif
#if defined(__ia64) || defined(__ia64__) || defined(___IA64__) || defined(_M_IA64)
  var = "IA64";
#endif
#if defined(__m68k__) || defined(M68000)
  var = "M68K";
#endif
#if defined(__mips__) || defined(_MIPS_ISA) || defined(_R3000) || defined(_R4000) || defined(__mips) || defined(__MIPS__)
  var = "MIPS";
#endif
#if defined(__hppa__) || defined(__hppa)
  var = "PARISC";
#endif
#if defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) || defined(_M__ARCH_PPC)
  var = "PPC";
#endif
#if defined(__powerpc64) || defined(__powerpc64__) || defined(__POWERPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
  var = "PPC64";
#endif
#if defined(__THW__IBMR2) || defined(_POWER) || defined(_ARCH_PWR) || defined(_ARCH_PWR2)
  var = "RS6000";
#endif
#if defined(__sparc__) || defined(__sparc) || defined(__sparcv8) || defined(__sparcv9)
  var = "SPARC";
#endif
#if defined(__sh__) || defined(__sh1__) || defined(__sh2__) || defined(__sh3__) || defined(__SH3__) || defined(__SH4__) || defined(__SH5__)
  var = "SUPERH";
#endif
#if defined(__370__) || defined(__THW_370__)
  var = "SYSTEM_370";
#endif
#if defined(__s390__) || defined(__s390x__)
  var = "SYSTEM_390";
#endif
#if defined(__i386__) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__i386) || defined(_M_I_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__)
  var = "X86";
#endif
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
  var = "X86_64";
#endif
#if defined(__SYSC_ZARCH__)
  var = "Z_ARCHITECTURE";
#endif

  printf("SYSDEP_ARCH_%s\n", var);
  return 0;
}
