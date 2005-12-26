/*	$Csoft: real.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#include <agar/config/have_long_double.h>
#include "begin_code.h"
__BEGIN_DECLS
__inline__ float	AG_ReadFloat(AG_Netbuf *);
__inline__ void		AG_WriteFloat(AG_Netbuf *, float);
__inline__ void		AG_PwriteFloat(AG_Netbuf *, float, off_t);
__inline__ double	AG_ReadDouble(AG_Netbuf *);
__inline__ void		AG_WriteDouble(AG_Netbuf *, double);
__inline__ void		AG_PwriteDouble(AG_Netbuf *, double, off_t);
#ifdef HAVE_LONG_DOUBLE
__inline__ long double	AG_ReadLongDouble(AG_Netbuf *);
__inline__ void		AG_WriteLongDouble(AG_Netbuf *, long double);
__inline__ void		AG_PwriteLongDouble(AG_Netbuf *, long double, off_t);
#endif
__END_DECLS
#include "close_code.h"
