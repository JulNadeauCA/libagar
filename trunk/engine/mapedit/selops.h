/*	$Csoft: selops.h,v 1.4 2003/04/25 09:47:07 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
void	selops_copy(struct mapview *);
void	selops_paste(struct mapview *, int, int);
void	selops_cut(struct mapview *);
void	selops_kill(struct mapview *);
__END_DECLS

#include "close_code.h"
