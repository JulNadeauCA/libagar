/*	$Csoft: objedit.h,v 1.1 2004/03/30 23:44:53 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJEDIT_H_
#define _AGAR_OBJEDIT_H_
#include "begin_code.h"

__BEGIN_DECLS
struct window	*objedit_window(void);
void		 objedit_init(void);
void		 objedit_destroy(void);
void		 objedit_open_data(struct object *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_OBJEDIT_H_ */
