/*	$Csoft: real.h,v 1.1 2003/06/19 01:53:38 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
__inline__ float	AG_ReadFloat(AG_Netbuf *);
__inline__ void		AG_WriteFloat(AG_Netbuf *, float);
__inline__ void		AG_PwriteFloat(AG_Netbuf *, float, off_t);

__inline__ double	AG_ReadDouble(AG_Netbuf *);
__inline__ void		AG_WriteDouble(AG_Netbuf *, double);
__inline__ void		AG_PwriteDouble(AG_Netbuf *, double, off_t);
__END_DECLS

#include "close_code.h"
