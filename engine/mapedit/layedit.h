/*	$Csoft: layedit.h,v 1.2 2003/03/11 03:40:01 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mapview;

struct layedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*layers_tl;
	struct textbox	*rename_tb;
};

void	layedit_init(struct mapview *);

#include "close_code.h"
