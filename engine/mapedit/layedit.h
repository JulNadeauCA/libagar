/*	$Csoft: nodeedit.h,v 1.2 2003/03/02 07:29:53 vedge Exp $	*/
/*	Public domain	*/

struct mapview;

struct layedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*layers_tl;
};

void	layedit_init(struct mapview *);

