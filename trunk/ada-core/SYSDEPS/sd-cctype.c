/*
 * Auto generated - do not edit.
 */

#include <stdio.h>

const char *var;

int main (void)
{
  var = "UNKNOWN";
#if defined(__ACK__)
  var = "AMSTERDAM_KIT";
#endif
#if defined(__BORLANDC__)
  var = "BORLAND_CXX";
#endif
#if defined(__TURBOC__)
  var = "BORLAND_TURBO_C";
#endif
#if defined(__MWERKS__)
  var = "CODEWARRIOR";
#endif
#if defined(__COMO__)
  var = "COMEAU_CXX";
#endif
#if defined(__DECC) || defined(__VAXC) || defined(VAXC)
  var = "COMPAQ_C";
#endif
#if defined(__DECCXX) || defined(__VAXC) || defined(VAXC)
  var = "COMPAQ_CXX";
#endif
#if defined(_CRAYC)
  var = "CRAY_C";
#endif
#if defined(__DCC__)
  var = "DIAB";
#endif
#if defined(__DMC__) || defined(__SC__) || defined(__ZTC__)
  var = "DIGITAL_MARS";
#endif
#if defined(__DJGPP__) || defined(__GO32__)
  var = "DJGPP";
#endif
#if defined(__PATHCC__)
  var = "EKOPATH";
#endif
#if defined(__GNUC__)
  var = "GCC";
#endif
#if defined(__ghs__)
  var = "GREENHILL";
#endif
#if defined(__HP_aCC)
  var = "HP_ACC";
#endif
#if defined(__HP_cc)
  var = "HP_C";
#endif
#if defined(__IAR_SYSTEMS_ICC__)
  var = "IAR";
#endif
#if defined(__xlC__) || defined(__IBMC__) || defined(__IBMCPP__)
  var = "IBM_XL";
#endif
#if defined(__INTEL_COMPILER) || defined(__ICC) || defined(__ECC) || defined(__ICL)
  var = "INTEL";
#endif
#if defined(__KCC)
  var = "KAI";
#endif
#if defined(__C166__)
  var = "KEIL_C166";
#endif
#if defined(__C51__) || defined(__CX51__)
  var = "KEIL_C51";
#endif
#if defined(__CA__) || defined(__KEIL__)
  var = "KEIL_CARM";
#endif
#if defined(__LCC__)
  var = "LCC";
#endif
#if defined(__llvm__)
  var = "LLVM";
#endif
#if defined(__HIGHC__)
  var = "METAWARE";
#endif
#if defined(_MRI)
  var = "MICROTEC";
#endif
#if defined(__MINGW32__)
  var = "MINGW32";
#endif
#if defined(__sgi) || defined(sgi)
  var = "MIPSPRO";
#endif
#if defined(__MRC__) || defined(MPW_C) || defined(MPW_CPLUS)
  var = "MPW";
#endif
#if defined(__CC_NORCROFT)
  var = "NORCROFT";
#endif
#if defined(__PACIFIC__)
  var = "PACIFIC";
#endif
#if defined(_PACC_VER)
  var = "PALM";
#endif
#if defined(__PCC__)
  var = "PCC";
#endif
#if defined(__POCC__)
  var = "PELLES";
#endif
#if defined(__PGI)
  var = "PORTLAND";
#endif
#if defined(__CC_ARM)
  var = "REALVIEW";
#endif
#if defined(SASC) || defined(__SASC) || defined(__SASC__)
  var = "SAS";
#endif
#if defined(_SCO_DS)
  var = "SCO";
#endif
#if defined(SDCC)
  var = "SMALLDEV";
#endif
#if defined(__SUNPRO_C)
  var = "SUN_C";
#endif
#if defined(__SUNPRO_CC)
  var = "SUN_CXX";
#endif
#if defined(__SYSC__)
  var = "SYSTEMS";
#endif
#if defined(__TenDRA__)
  var = "TENDRA";
#endif
#if defined(__TINYC__)
  var = "TINYC";
#endif
#if defined(_UCC)
  var = "ULTIMATE";
#endif
#if defined(__USLC__)
  var = "USL_C";
#endif
#if defined(_MSC_VER)
  var = "VISUAL_CXX";
#endif
#if defined(__WATCOMC__)
  var = "WATCOM";
#endif
  printf("SD_SYSINFO_CC_TYPE_%s\n", var);
  return 0;
}
