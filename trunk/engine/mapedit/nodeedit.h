/*	$Csoft: nodeedit.h,v 1.8 2004/03/18 03:07:53 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mapview;

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
