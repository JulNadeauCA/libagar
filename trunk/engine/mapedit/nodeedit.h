/*	$Csoft: nodeedit.h,v 1.1 2003/03/02 04:08:54 vedge Exp $	*/
/*	Public domain	*/

struct mapview;

struct nodeedit {
	struct window	*win;
	struct button	*trigger;
	struct label	*node_flags_lab, *node_size_lab;
	struct label	*noderef_type_lab, *noderef_flags_lab;
	struct label	*noderef_center_lab;
	struct tlist	*refs_tl, *transforms_tl;
};

void	nodeedit_init(struct mapview *);

