/*	$Csoft: objmgr.h,v 1.7 2005/09/05 05:02:16 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJMGR_H_
#define _AGAR_OBJMGR_H_
#include "begin_code.h"

__BEGIN_DECLS
struct window	*objmgr_window(void);
void		 objmgr_init(void);
void		 objmgr_destroy(void);
void		 objmgr_reopen(struct object *);
void		 objmgr_open_data(void *, int);
void		 objmgr_close_data(void *);
void		 objmgr_open_generic(struct object *);
void		 objmgr_quit_dlg(void *);
void		 objmgr_save_to(void *);
void		 objmgr_generic_menu(void *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_OBJMGR_H_ */
