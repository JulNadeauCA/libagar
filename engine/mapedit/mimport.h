/*	$Csoft: mimport.h,v 1.1 2003/07/26 12:35:39 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mimport {
	struct window	*win;
	struct button	*trigger;
};

__BEGIN_DECLS
struct window	*mimport_window(struct mapview *);
__END_DECLS

#include "close_code.h"
