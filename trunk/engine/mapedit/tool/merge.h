/*	$Csoft: merge.h,v 1.4 2003/02/12 01:09:32 vedge Exp $	*/
/*	Public domain	*/

enum merge_mode {
	MERGE_REPLACE,
	MERGE_INSERT_HIGHEST,
	MERGE_INSERT_EMPTY,
	MERGE_ERASE_ALL,
	MERGE_ERASE_HIGHEST
};

SLIST_HEAD(brushq, object);

struct merge {
	struct tool	 tool;
	int		 mode;
	int		 flags;
#define MERGE_RANDOMIZE_SHIFT	0x01

	struct tlist	*brushes_tl;
	struct brushq	 brushes;
};

void		 merge_init(void *);
struct window	*merge_window(void *);
void		 merge_effect(void *, struct mapview *, struct node *);
void		 merge_apply(struct merge *, struct mapview *, struct map *);
void		 merge_destroy(void *);
int		 merge_load(void *, int);
int		 merge_save(void *, int);
int		 merge_cursor(void *, struct mapview *, SDL_Rect *);

