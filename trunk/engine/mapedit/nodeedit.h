/*	$Csoft: nodeedit.h,v 1.3 2003/03/28 00:23:22 vedge Exp $	*/
/*	Public domain	*/

#include "begin_code.h"

struct mapview;

struct nodeedit {
	struct window	*win;
	struct button	*trigger;
	struct label	*node_flags_lab;
	struct label	*noderef_type_lab, *noderef_flags_lab;
	struct label	*noderef_center_lab;
	struct tlist	*refs_tl, *transforms_tl;
};

__BEGIN_DECLS
extern DECLSPEC void	nodeedit_init(struct mapview *);
__END_DECLS

#include "close_code.h"
