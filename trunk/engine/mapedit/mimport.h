/*	$Csoft: nodeedit.h,v 1.6 2003/06/29 11:33:43 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mimport {
	struct window	*win;
	struct button	*trigger;
};

__BEGIN_DECLS
void	mimport_init(struct mapview *);
__END_DECLS

#include "close_code.h"
