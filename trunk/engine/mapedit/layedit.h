/*	$Csoft: layedit.h,v 1.6 2004/03/18 03:07:53 vedge Exp $	*/
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
void	layedit_init(struct mapview *, struct window *);
void	layedit_poll(int, union evarg *);
void	layedit_destroy(struct mapview *);
__END_DECLS

#include "close_code.h"
