/*	$Csoft: merge.h,v 1.5 2003/01/26 06:15:21 vedge Exp $	*/
/*	Public domain	*/

enum merge_mode {
	MERGE_REPLACE,
	MERGE_INSERT_HIGHEST
};

struct merge {
	struct tool	 tool;
	int		 mode;
	struct map	 brush;
};

void		 merge_init(void *);
struct window	*merge_window(void *);
void		 merge_effect(void *, struct mapview *, struct node *);
void		 merge_destroy(void *);

