/*	$Csoft: layedit.h,v 1.1 2003/03/05 02:16:32 vedge Exp $	*/
/*	Public domain	*/

struct mapview;

struct layedit {
	struct window	*win;
	struct button	*trigger;
	struct tlist	*layers_tl;
	struct textbox	*rename_tb;
};

void	layedit_init(struct mapview *);

