/*	$Csoft: objmgr.h,v 1.2 2004/08/26 07:34:18 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJMGR_H_
#define _AGAR_OBJMGR_H_
#include "begin_code.h"

__BEGIN_DECLS
struct window	*objmgr_window(void);
void		 objmgr_init(void);
void		 objmgr_destroy(void);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_OBJMGR_H_ */
