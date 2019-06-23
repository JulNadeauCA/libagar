/*	Public domain	*/
/*
 * Serialization of floating-point numbers.
 */

#ifndef	_AGAR_CORE_LOAD_REAL_H_
#define	_AGAR_CORE_LOAD_REAL_H_
#include <agar/core/begin.h>
#ifdef AG_HAVE_FLOAT

__BEGIN_DECLS
/*
 * Inlinables
 */
float       ag_read_float(AG_DataSource *_Nonnull);
void        ag_write_float(AG_DataSource *_Nonnull, float);
void        ag_write_float_at(AG_DataSource *_Nonnull, float, AG_Offset);
double      ag_read_double(AG_DataSource *_Nonnull);
void        ag_write_double(AG_DataSource *_Nonnull, double);
void        ag_write_double_at(AG_DataSource *_Nonnull, double, AG_Offset);

# ifdef AG_INLINE_IO
#  define AG_INLINE_HEADER
#  include <agar/core/inline_load_real.h>
# else
#  define AG_ReadFloat(ds)               ag_read_float(ds)
#  define AG_WriteFloat(ds,x)            ag_write_float((ds),(x))
#  define AG_WriteFloatAt(ds,x,pos)      ag_write_float_at((ds),(x),(pos))
#  define AG_ReadDouble(ds)              ag_read_double(ds)
#  define AG_WriteDouble(ds,x)           ag_write_double((ds),(x))
#  define AG_WriteDoubleAt(ds,x,pos)     ag_write_double_at((ds),(x),(pos))
# endif
__END_DECLS

#endif /* AG_HAVE_FLOAT */
#include <agar/core/close.h>
#endif /* _AGAR_CORE_LOAD_REAL_H_ */
