/*	$Csoft: merge.h,v 1.16 2003/06/18 00:47:01 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum merge_mode {
	MERGE_REPLACE		/* Replace, including empty source nodes */
};

TAILQ_HEAD(brushq, object);

struct merge {
	struct tool	 tool;
	int		 mode;
	struct tlist	*brushes_tl;
	struct brushq	 brushes;
	int		 random_shift;		/* Random noderef shifts */
	Uint8		 layer;
};

__BEGIN_DECLS
void		 merge_init(void *);
struct window	*merge_window(void *);
void		 merge_effect(void *, struct mapview *, struct map *,
		              struct node *);
void		 merge_destroy(void *);
int		 merge_load(void *, struct netbuf *);
int		 merge_save(void *, struct netbuf *);
int		 merge_cursor(void *, struct mapview *, SDL_Rect *);
__END_DECLS

#include "close_code.h"
