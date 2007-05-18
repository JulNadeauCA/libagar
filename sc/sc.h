/*	$Csoft: vg.h,v 1.41 2005/09/27 00:25:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SC_H_
#define _AGAR_SC_H_

#if 0
#define SC_PROP_REAL	101
#define SC_PROP_VECTOR	102
#define SC_PROP_MATRIX	103
#define SC_PROP_TIME	104
#define SC_PROP_QTIME	105
#endif

#include <agar/config/_mk_have_sys_types_h.h>
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
typedef unsigned char Uchar;
typedef unsigned long Ulong;
#endif

#include "begin_code.h"

#include <agar/sc/sc_math.h>
#include <agar/sc/sc_matrix.h>
#include <agar/sc/sc_vector.h>
#include <agar/sc/sc_ivector.h>
#include <agar/sc/sc_gaussj.h>
#include <agar/sc/sc_lu.h>

__BEGIN_DECLS
int	SC_Init(Uint);
void	SC_Destroy(void);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SC_H_ */
