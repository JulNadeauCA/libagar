/*	$Csoft: layedit.h,v 1.3 2003/04/25 09:47:07 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mapview;

struct layedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*layers_tl;
	struct textbox	*rename_tb;
};

__BEGIN_DECLS
void	layedit_init(struct mapview *);
void	layedit_poll(int, union evarg *);
__END_DECLS

#include "close_code.h"
