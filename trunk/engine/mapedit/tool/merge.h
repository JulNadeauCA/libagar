/*	$Csoft: merge.h,v 1.12 2003/03/25 13:48:05 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

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

void		 merge_init(void *);
struct window	*merge_window(void *);
void		 merge_effect(void *, struct mapview *, struct node *);
void		 merge_destroy(void *);
int		 merge_load(void *, struct netbuf *);
int		 merge_save(void *, struct netbuf *);
int		 merge_cursor(void *, struct mapview *, SDL_Rect *);
void		 merge_interpolate(struct merge *, struct map *,
		     struct node *, struct node *, int, int, struct mapview *);

