/*	$Csoft: merge.h,v 1.1 2003/02/02 21:14:02 vedge Exp $	*/
/*	Public domain	*/

enum merge_mode {
	MERGE_REPLACE,
	MERGE_INSERT_HIGHEST,
	MERGE_INSERT_EMPTY
};

struct merge {
	struct tool	 tool;
	int		 mode;
	int		 flags;
#define MERGE_RANDOMIZE_SHIFT	0x01
	struct map	 brush;
};

void		 merge_init(void *);
struct window	*merge_window(void *);
void		 merge_effect(void *, struct mapview *, struct node *);
void		 merge_destroy(void *);

