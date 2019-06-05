#include <stdio.h>

unsigned int ver_major;
unsigned int ver_minor;
unsigned int ver_patch;
unsigned int ver;

static void hex_decode_tri(unsigned int ver)
{
  ver_major = ver / 0x100;
  ver -= ver_major * 0x100;
  ver_minor = ver / 0x010;
  ver -= ver_minor * 0x010;
  ver_patch = ver;
}
static void dec_decode_tri(unsigned int ver)
{
  ver_major = ver / 100;
  ver -= ver_major * 100;
  ver_minor = ver / 10;
  ver -= ver_minor * 10;
  ver_patch = ver;
}

int main(void)
{
  /* XXX: hack to allow compilation on compilers with extreme error checking
          (unused functions cause compilation failure) */

  hex_decode_tri(1);
  dec_decode_tri(1);
  ver_major = 0;
  ver_minor = 0;
  ver_patch = 0;
  ver = 0;

  /* Borland C++ */
#if defined(__BORLANDC__)
  hex_decode_tri(__BORLANDC__);
#endif

  /* Comeau C++ */
#if defined(__COMO_VERSION__)
  dec_decode_tri(__COMO_VERSION__);
#endif

  /* Compaq C */
#if defined(__DECC_VER)
  ver = __DECC_VER;
  ver_major = ver / 10000000;
  ver -= ver_major * 10000000;
  ver_minor = ver / 100000;
  ver -= ver_minor * 100000;
  ver -= (ver / 10000) * 10000;
  ver_patch = ver;
#endif

  /* Compaq C++ */
#if defined(__DECCXX_VER)
  ver = __DECCXX_VER;
  ver_major = ver / 10000000;
  ver -= ver_major * 10000000;
  ver_minor = ver / 100000;
  ver -= ver_minor * 100000;
  ver -= (ver / 10000) * 10000;
  ver_patch = ver;
#endif

  /* Cray C */
#if defined(_CRAY)
  ver_major = _RELEASE;
  ver_minor = _RELEASE_MINOR; 
#endif

  /* Digital Mars */
#if defined(__DMC__)
  hex_decode_tri(__DMC__);
#endif

  /* DJGPP */
#if defined(__DJGPP__)
  ver_major = __DJGPP__;
  ver_minor = __DJGPP_MINOR__;
#endif

  /* EKOPath */
#if defined(__PATHCC__)
  ver_major = __PATHCC__;
  ver_minor = __PATHCC_MINOR__;
  ver_patch = __PATHCC_PATCHLEVEL__;
#endif

  /* EDG C++ Front End */
#if defined(__EDG__)
  dec_decode_tri(__EDG_VERSION__);
#endif

  /* GNU C */
#if defined(__GNUC__)
  ver_major = __GNUC__;
  ver_minor = __GNUC_MINOR__;
  ver_patch = __GNUC_PATCHLEVEL__;
#endif

  /* Green Hill C/C++ */
#if defined(__ghs__)
  dec_decode_tri(__GHS_VERSION_NUMBER__);
#endif

  /* HP aCC */
#if defined(__HP_aCC)
  ver = __HP_aCC;
  ver_major = ver / 10000;
  ver -= ver_major * 10000;
  ver_minor = ver / 100;
  ver -= ver_minor * 100;
  ver_patch = ver;
#endif

  /* IBM XL C/C++ */
#if defined(__xlC__)
  hex_decode_tri(__xlC__);
#endif

  /* IBM XL C/C++ */
#if defined(__IBMC__)
  dec_decode_tri(__IBMC__);
#endif

  /* IBM XL C/C++ */
#if defined(__IBMCPP__)
  dec_decode_tri(__IBMCPP__);
#endif

  /* IAR C/C++ */
#if defined(__IAR_SYSTEMS_ICC__)
  ver = __VER__;
  ver_major = ver / 100;
  ver -= ver_major * 100;
  ver_minor = ver;
  ver_patch = 0;
#endif

  /* Intel C/C++ */
#if defined(__INTEL_COMPILER)
  dec_decode_tri(__INTEL_COMPILER);
#endif

  /* KAI C++ */
#if defined(__KCC)
  ver = __KCC_VERSION;
  ver_major = ver / 0x1000;
  ver -= ver_major * 0x1000;
  ver_minor = ver / 0x100;
  ver -= ver_minor * 0x100;
  ver_patch = ver;
#endif

  /* Keil CARM */
#if defined(__CA__)
  ver = __CA__;
  ver_major = ver / 100;
  ver -= ver_major * 100;
  ver_minor = ver;
  ver_patch = 0;
#endif

  /* Keil C166 */
#if defined(__C166__)
  ver = __C166__;
  ver_major = ver / 100;
  ver -= ver_major * 100;
  ver_minor = ver;
  ver_patch = 0;
#endif

  /* Keil C51 */
#if defined(__C51__)
  ver = __C51__;
  ver_major = ver / 100;
  ver -= ver_major * 100;
  ver_minor = ver;
  ver_patch = 0;
#endif

  /* Metrowerks CodeWarrior */
#if defined(__MWERKS__)
  ver = __MWERKS__;
  ver_major = ver / 0x1000;
  ver -= ver_major * 0x1000;
  ver_minor = ver / 0x0100;
  ver -= ver_minor * 0x0100;
  ver_patch = ver;
#endif

  /* MinGW */
#if defined(__MINGW32__)
  ver_major = __MINGW32_MAJOR_VERSION;
  ver_minor = __MINGW32_MINOR_VERSION;
  ver_patch = 0;
#endif

  /* MIPSpro */
#if (defined(__sgi) || defined(sgi)) && defined(_SGI_COMPILER_VERSION)
  ver = _SGI_COMPILER_VERSION;     
  ver_major = ver / 100;     
  ver -= ver_major * 100;
  ver_minor = ver / 10; 
  ver -= ver_minor * 10;
  ver_patch = ver;     
#endif

  /* MIPSpro again */
#if (defined(__sgi) || defined(sgi)) && defined(_COMPILER_VERSION)
  ver = _COMPILER_VERSION;                   
  ver_major = ver / 100;     
  ver -= ver_major * 100;
  ver_minor = ver / 10; 
  ver -= ver_minor * 10;
  ver_patch = ver;     
#endif

  /* MPW C++ */
#if defined(__MRC__) || defined(MPW_C) || defined(MPW_CPLUS)
  ver = __MRC__;
  ver_major = ver / 0x100;
  ver -= ver_major * 0x100;
  ver_minor = ver / 0x10;
  ver -= ver_minor * 0x10;
  ver_patch = 0;
#endif

  /* Palm C/C++ */
#if defined(_PACC_VER)
  ver = _PACC_VER;
  ver_major = ver / 0x1000000;
  ver -= ver_major * 0x1000000;
  ver_minor = ver / 0x100000;
  ver -= ver_minor * 0x100000;
  ver_patch = ver /  0x1000;
#endif

  /* Pelles C */
#if defined(__POCC__)
  dec_decode_tri(__POCC__);
#endif

  /* PCC (Portable C compiler from 4.3BSD) */
#if defined(__PCC__)
  ver_major = __PCC__;
  ver_minor = __PCC_MINOR__;
  ver_patch = __PCC_MINORMINOR__;
#endif

  /* Small Device C Compiler */
#if defined(SDCC)
  dec_decode_tri(SDCC);
#endif

  /* Sun C */
#if defined(__SUNPRO_C)
  hex_decode_tri(__SUNPRO_C);
#endif

  /* Sun C++ */
#if defined(__SUNPRO_CC)
  hex_decode_tri(__SUNPRO_CC);
#endif

  /* Systems/C and Systems/C++ */
#if defined(__SYSC__)
  ver = __SYSC_VER__;
  ver_major = ver / 10000;
  ver -= ver_major * 10000;
  ver_minor = ver / 1000;
  ver -= ver_minor * 1000;
  ver_patch = ver;
#endif

  /* Ultimate C/C++ */
#if defined(_UCC)
  ver_major = _MAJOR_REV;
  ver_minor = _MINOR_REV;
  ver_patch = 0;
#endif

  /* Watcom C++ */
#if defined(__WATCOMC__)
  ver = __WATCOMC__;
  ver_major = ver / 100;
  ver -= ver_major * 100;
  ver_minor = ver / 10;
  ver -= ver_minor * 10;
  ver_patch = 0;
#endif

  printf("%u.%u.%u\n", ver_major, ver_minor, ver_patch);
  return 0;
}
