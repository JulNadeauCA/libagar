/*	$Csoft: objmgr.h,v 1.1 2005/02/03 05:00:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJMGR_H_
#define _AGAR_OBJMGR_H_
#include "begin_code.h"

__BEGIN_DECLS
struct window	*objmgr_window(void);
void		 objmgr_init(void);
void		 objmgr_destroy(void);
void		 objmgr_reopen(struct object *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_OBJMGR_H_ */
