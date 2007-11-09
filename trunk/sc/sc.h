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

#include "begin_code.h"

#ifdef _AGAR_INTERNAL
#include <sc/sc_math.h>
#include <sc/sc_matrix.h>
#include <sc/sc_vector.h>
#include <sc/sc_ivector.h>
#include <sc/sc_gaussj.h>
#include <sc/sc_lu.h>
#else
#include <agar/sc/sc_math.h>
#include <agar/sc/sc_matrix.h>
#include <agar/sc/sc_vector.h>
#include <agar/sc/sc_ivector.h>
#include <agar/sc/sc_gaussj.h>
#include <agar/sc/sc_lu.h>
#endif

__BEGIN_DECLS
void	SC_InitSubsystem(void);
void	SC_DestroySubsystem(void);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SC_H_ */
