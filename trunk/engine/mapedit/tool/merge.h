/*	$Csoft: merge.h,v 1.9 2003/03/07 03:24:49 vedge Exp $	*/
/*	Public domain	*/

enum merge_mode {
	MERGE_FILL,		/* Replace and skip empty source nodes */
	MERGE_REPLACE,		/* Replace, including empty source nodes */
	MERGE_INSERT_HIGHEST,	/* Insert new noderefs */
	MERGE_INSERT_EMPTY,	/* Insert new noderefs on empty dst nodes */
	MERGE_ERASE		/* Erase mask  */
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
int		 merge_load(void *, int);
int		 merge_save(void *, int);
int		 merge_cursor(void *, struct mapview *, SDL_Rect *);
void		 merge_interpolate(struct merge *, struct map *,
		     struct node *, struct node *, int, int, struct mapview *);

