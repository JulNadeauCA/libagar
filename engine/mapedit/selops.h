/*	$Csoft: selops.h,v 1.3 2003/03/26 10:04:15 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
extern DECLSPEC void	selops_copy(struct mapview *);
extern DECLSPEC void	selops_paste(struct mapview *, int, int);
extern DECLSPEC void	selops_cut(struct mapview *);
extern DECLSPEC void	selops_kill(struct mapview *);
__END_DECLS

#include "close_code.h"
