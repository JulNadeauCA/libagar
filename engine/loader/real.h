/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
__inline__ float	read_float(struct netbuf *);
__inline__ void		write_float(struct netbuf *, float);
__inline__ void		pwrite_float(struct netbuf *, float, off_t);

__inline__ double	read_double(struct netbuf *);
__inline__ void		write_double(struct netbuf *, double);
__inline__ void		pwrite_double(struct netbuf *, double, off_t);
__END_DECLS

#include "close_code.h"
