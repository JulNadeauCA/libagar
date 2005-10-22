/*	$Csoft: xcf.h,v 1.2 2003/06/21 06:50:20 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct ag_gfx;

__BEGIN_DECLS
int AG_XCFLoad(AG_Netbuf *, off_t, struct ag_gfx *);
__END_DECLS

#include "close_code.h"
