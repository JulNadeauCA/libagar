/*	$Csoft: objmgr.h,v 1.2 2005/02/03 09:19:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJMGR_H_
#define _AGAR_OBJMGR_H_
#include "begin_code.h"

__BEGIN_DECLS
struct window	*objmgr_window(void);
void		 objmgr_init(void);
void		 objmgr_destroy(void);
void		 objmgr_reopen(struct object *);
void		 objmgr_open_data(struct object *);
void		 objmgr_open_generic(struct object *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_OBJMGR_H_ */
