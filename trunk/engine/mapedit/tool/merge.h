/*	$Csoft: merge.h,v 1.13 2003/04/12 01:45:40 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum merge_mode {
	MERGE_REPLACE		/* Replace, including empty source nodes */
};

SLIST_HEAD(brushq, object);

struct merge {
	struct tool	 tool;
	int		 mode;
	struct tlist	*brushes_tl;
	struct brushq	 brushes;
	int		 inherit_flags;		/* Inherit node flags */
	int		 random_shift;		/* Random noderef shifts */
	Uint8		 layer;
};

__BEGIN_DECLS
extern DECLSPEC void		 merge_init(void *);
extern DECLSPEC struct window	*merge_window(void *);
extern DECLSPEC void		 merge_effect(void *, struct mapview *,
				              struct node *);
extern DECLSPEC void		 merge_destroy(void *);
extern DECLSPEC int		 merge_load(void *, struct netbuf *);
extern DECLSPEC int		 merge_save(void *, struct netbuf *);
extern DECLSPEC int		 merge_cursor(void *, struct mapview *,
				              SDL_Rect *);
extern DECLSPEC void		 merge_interpolate(struct merge *, struct map *,
		                                   struct node *, struct node *,
						   int, int, struct mapview *);
__END_DECLS

#include "close_code.h"
