/*	$Csoft: nodeedit.h,v 1.7 2004/03/17 12:42:06 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct nodeedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*refs;
};

__BEGIN_DECLS
void	nodeedit_init(struct mapview *, struct window *win);
void	nodeedit_destroy(struct mapview *);
__END_DECLS

#include "close_code.h"
