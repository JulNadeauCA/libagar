/*	$Csoft: nodeedit.h,v 1.5 2003/06/18 00:47:00 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mapview;

struct nodeedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*refs_tl;
};

__BEGIN_DECLS
void	nodeedit_init(struct mapview *);
__END_DECLS

#include "close_code.h"
