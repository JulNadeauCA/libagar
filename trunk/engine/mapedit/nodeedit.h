/*	$Csoft: nodeedit.h,v 1.6 2003/06/29 11:33:43 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct nodeedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*refs;
};

__BEGIN_DECLS
void	nodeedit_init(struct mapview *);
void	nodeedit_destroy(struct mapview *);
__END_DECLS

#include "close_code.h"
